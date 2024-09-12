import os
import subprocess


def get_dlls(MSYSTEM):
    """Return the dlls used by progress-tracker.exe"""
    dlls = []
    if MSYSTEM == "UCRT64":
        ldd_command = "ldd src/progress-tracker.exe".split(" ")
        ldd_output = subprocess.run(ldd_command, capture_output=True).stdout.replace(
            b"\t", b""
        )
        dlls_infos = map(
            lambda line: line.strip(),
            filter(lambda line: "ucrt64" in line, ldd_output.decode().split("\n")),
        )
        for dll_info in dlls_infos:
            dlls.append(dll_info.split("=>")[0].strip())

    elif MSYSTEM == "MINGW32":
        ntldd_command = "ntldd -R src/progress-tracker.exe".split(" ")
        ntldd_output = subprocess.run(
            ntldd_command, capture_output=True
        ).stdout.replace(b"\t", b"")
        dlls_infos = map(
            lambda line: line.strip(),
            filter(lambda line: "mingw32" in line, ntldd_output.decode().split("\n")),
        )
        for dll_info in dlls_infos:
            dlls.append(dll_info.split("=>")[0].strip())
    else:
        return []

    missing_dlls = [
        "librsvg-2-2.dll",
        "libxml2-2.dll",
        "libiconv-2.dll",
        "libcharset-1.dll",
        "zlib1.dll",
    ]
    return dlls + missing_dlls


def main():
    """Script for setting up the gtk environment for the application
    to run.The script assumes the current directory is
    PROJECT_BINARY_DIR.
    """
    SUPPORTED_ENVIRONMENTS = ["UCRT64", "MINGW32"]
    MSYSTEM = os.getenv("MSYSTEM")

    if MSYSTEM is None and MSYSTEM not in SUPPORTED_ENVIRONMENTS:
        print("Script is not in a proper MSYS2 environment. " "Returning...")
        return 1

    ROOT = f"/{MSYSTEM.lower()}"

    try:
        os.mkdir("share")
        os.mkdir("lib")
        os.mkdir("DLLS")
    except FileExistsError as err:
        print("Directories still exists. Files will still be replaced")

    subprocess.run(["cp", "-r", f"{ROOT}/share/glib-2.0/", "share"])
    subprocess.run(["cp", "-r", f"{ROOT}/share/icons/", "share"])
    subprocess.run(["cp", "-r", f"{ROOT}/lib/gdk-pixbuf-2.0/", "lib"])

    dlls = get_dlls(MSYSTEM)
    for dll in dlls:
        subprocess.run(["cp", f"{ROOT}/bin/{dll}", "DLLS"])

    if MSYSTEM == "UCRT64":
        # Necessary for older 64 bit operating systems
        subprocess.run("cp /c/Windows/System32/ucrtbase.dll DLLS".split())

    # Helper executable
    subprocess.run(f"cp {ROOT}/bin/gdbus.exe .".split())

    print("Setting up Progress schemas")
    progress_schema = "../data/io.github.smolblackcat.Progress.gschema.xml"
    subprocess.run(f"cp {progress_schema} share/glib-2.0/schemas/".split())
    subprocess.run("glib-compile-schemas share/glib-2.0/schemas/".split())


if __name__ == "__main__":
    main()
