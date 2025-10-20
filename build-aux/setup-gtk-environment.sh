#!/usr/bin/env bash
# Setup script for GTK environment under MSYS2
# Equivalent to the original Python version

set -euo pipefail

get_dlls() {
    local MSYSTEM="$1"
    local dlls=()

    if [[ "$MSYSTEM" == "UCRT64" ]]; then
        # Get DLLs used by progress-tracker.exe
        while IFS= read -r line; do
            # Include only DLLs from ucrt64 and extract names
            if [[ "$line" == *"ucrt64"* ]]; then
                dll_name="$(echo "$line" | sed 's/\t//g' | awk -F '=>' '{print $1}' | xargs)"
                dlls+=("$dll_name")
            fi
        done < <(ldd src/progress-tracker.exe)

        # Add missing DLLs
        dlls+=(
            "librsvg-2-2.dll"
            "libxml2-2.dll"
            "libiconv-2.dll"
            "libcharset-1.dll"
            "zlib1.dll"
            "vulkan-1.dll"
            "libcrypto-3-x64.dll"
            "libssl-3-x64.dll"
        )
    fi

    echo "${dlls[@]}"
}

main() {
    local SUPPORTED_ENVIRONMENTS=("UCRT64")
    local MSYSTEM="${MSYSTEM:-}"

    if [[ ! " ${SUPPORTED_ENVIRONMENTS[*]} " =~ " ${MSYSTEM} " ]]; then
        echo "Script is not under a valid MSYS2 environment"
        exit 1
    fi

    local ROOT="/${MSYSTEM,,}"  # lowercase

    # Create directories
    mkdir -p share lib DLLS || echo "Directories already exist. Files will be replaced."

    # Copy GTK resources
    cp -r "${ROOT}/share/glib-2.0/" share/
    cp -r "${ROOT}/share/icons/" share/
    cp -r "${ROOT}/lib/gdk-pixbuf-2.0/" lib/

    # Get DLL list and copy them
    IFS=' ' read -r -a dlls <<< "$(get_dlls "$MSYSTEM")"
    for dll in "${dlls[@]}"; do
        cp "${ROOT}/bin/${dll}" DLLS/ 2>/dev/null || echo "Warning: Missing DLL $dll"
    done

    if [[ "$MSYSTEM" == "UCRT64" ]]; then
        # Necessary for older 64-bit systems
        cp "/c/Windows/System32/ucrtbase.dll" DLLS/
    fi

    # Copy gdbus
    cp "${ROOT}/bin/gdbus.exe" .

    echo "Setting up Progress schemas..."
    local progress_schema="../data/io.github.smolblackcat.Progress.gschema.xml"
    cp "$progress_schema" "share/glib-2.0/schemas/"
    glib-compile-schemas "share/glib-2.0/schemas/"
}

main "$@"
