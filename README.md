# ![progress-logo] Progress

Simple Kanban board manager

[![GitHub Release][github-release-badge]][github-release]
[![Flathub Downloads][flatpak-release-badge]][progress-flathub]
[![GitHub License][github-licence]](LICENSE)

![App Windows](pictures/progress-app-presentation.png)

Progress is a Kanban board management app designed to be simple yet flexible for
most workflows. Whether you're managing software projects or organsing
daily tasks, Progress keeps things straightforward. It is built in
C++ with GTK4 and LibAdwaita.

## Dependencies

* gtkmm-4 (>= 4.15.0)
* libadwaita1 (>= 1.5)
* tinyxml2 (>= 11.0.0)
* spdlog (>= 1.15)
* crossguid (built-in)

## Installation

### Flatpak

[![flathub-badge]][progress-flathub]

### Debian-based

A Debian package can be found and installed in the Progress's releases page. It
is built on a *Debian Testing* box, so some dependencies may not be available to
your Debian-based or Ubuntu-based distribution.

### Arch Linux (AUR)

Progress is available on the AUR (Arch User Repository). You can install it
using the `yay` command-line application:

```sh
yay -S progress-tracker
```

### Windows

For Windows systems, Progress is available as a standard Windows installer
executable or as a compressed file containing the application's binary and its
dependencies. These builds target x64 systems, so make sure the host operating
system is also x64.

### Building from Source

Alternative for those who may want to contribute to the project somehow or want
a custom build. Ensure all dependencies are installed beforehand and verify
whether the system's C++ compiler is C++20 compliant.

#### Linux

1. Install the dependencies

   ```sh
   # Use your package manager
   sudo apt install cmake libgtkmm-4.0-dev libadwaita-1-dev libtinyxml2-dev \
    libspdlog-dev gettext catch2
   ```

2. Clone the project's repository

   ```sh
   git clone https://github.com/smolBlackCat/progress-tracker.git --recursive
   ```

3. Configure and compile project.

   ```sh
   cd progress-tracker/

   cmake -S . -B build/ -DCMAKE_INSTALL_PREFIX=/usr -DDEVELOPMENT=OFF
   cmake --build build/
   ```

4. Install the Project

   ```sh
   sudo ninja install -C build/
   sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
   ```

#### Windows

[MSYS2][msys-download] is a build platform utilised by the project for porting
Progress to Windows operating systems. Ensure that the platform is installed
before proceeding.

1. Install the dependencies

   ```sh
   # This snippet assumes you're running on the UCRT64 environment
   pacman -Syu mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-gtkmm4 mingw-w64-ucrt-x86_64-libadwaita mingw-w64-ucrt-x86_64-tinyxml2 mingw-w64-ucrt-x86_64-gettext mingw-w64-ucrt-x86_64-catch mingw-w64-ucrt-x86_64-spdlog mingw-w64-ucrt-x86_64-nsis git
   ```

2. Clone the project's repository

   ```sh
   git clone https://github.com/smolBlackCat/progress-tracker.git --recursive
   ```

3. Configure and compile project

   ```sh
   cd progress-tracker/
   cmake -S . -B build/ -DDEVELOPMENT=OFF
   cmake --build build/
   ```

4. Build the Progress installer and ZIP file

   ```sh
   cd build/

   # Builds a Windows installer executable
   cpack
   ```

[progress-logo]: data/io.github.smolblackcat.Progress.svg
[progress-flathub]: https://flathub.org/apps/io.github.smolblackcat.Progress
[github-release]: https://github.com/smolBlackCat/progress-tracker/releases
[flathub-badge]: https://flathub.org/assets/badges/flathub-badge-i-en.png
[github-release-badge]: https://img.shields.io/github/v/release/smolBlackCat/progress-tracker?logo=github
[flatpak-release-badge]: https://img.shields.io/flathub/downloads/io.github.smolblackcat.Progress?logo=flathub
[github-licence]: https://img.shields.io/github/license/smolBlackCat/progress-tracker
[build-status]: https://img.shields.io/github/actions/workflow/status/smolBlackCat/progress-tracker/progress-ci.yml
[msys-download]: https://www.msys2.org/