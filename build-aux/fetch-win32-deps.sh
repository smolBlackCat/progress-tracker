#! /usr/bin/sh

# This script assumes it's being run on the project's root directory. Run this
# script only after building using make. It is also supposed to be run on a
# UCRT64 environment

MSYS_ROOT="/ucrt64"

cd "build"

mkdir "share"
mkdir "lib"

cp -r "${MSYS_ROOT}/share/glib-2.0/" "share"
cp -r "${MSYS_ROOT}/share/icons/" "share"
cp -r "${MSYS_ROOT}/lib/gdk-pixbuf-2.0/" "lib"

ldd src/progress-tracker.exe | grep '\/ucrt64.*\.dll' -o | xargs -I{} cp "{}" .

# Some dependencies are not returned by ldd. Fetch them manually
cp ${MSYS_ROOT}/bin/librsvg-2-2.dll .
cp ${MSYS_ROOT}/bin/libxml2-2.dll .
cp ${MSYS_ROOT}/bin/libiconv-2.dll .
cp ${MSYS_ROOT}/bin/libcharset-1.dll .
cp ${MSYS_ROOT}/bin/zlib1.dll .

# Compatibility for older Windows x86_64 systems
cp "/c/Windows/System32/ucrtbase.dll" .