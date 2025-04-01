# ![progress-logo] Progress

Simple kanban-style task organiser

[![GitHub Release][github-release-badge]][github-release]
[![Flathub Downloads][flatpak-release-badge]][progress-flathub]
[![GitHub License][github-licence]](LICENSE)

![App Windows](pictures/progress-app-presentation.png)

Progress is a simple task management app focused on simplicity yet enough for
most workflows. It can be used on various contexts, from software development
to daily tasks. It is written in C++ using GTK 4(gtkmm) and libadwaita.

## Installation

### Flatpak

Progress is available to download at [Flathub][progress-flathub].

![flathub-badge][flathub-badge]

### Debian

A debian build is also available. The debian package can be obtained in the
[releases][github-release] page.

```sh
sudo apt install ./progress-tracker-1.7.deb
```

### Arch Linux (AUR)

Progress is available on the AUR (Arch User Repository). You can install it
using the `yay` command-line application:

```sh
yay -S progress-tracker
```

### Windows

Windows installers are also available in the [releases][github-release] section.
The installers were tested against the following environments.

- Windows 8
- Windows 10
- Windows 11

### Building from Source

For contributing to Progress source code, building from source might be the
better option.

#### Dependencies

- cmake
- python3 (required only if building on Windows)
- libgtkmm-4.0-dev
- libadwaita-1-dev
- libtinyxml2-dev
- libspdlog-dev
- gettext
- catch2 (optional)

#### Build (Linux)

1. Install the dependencies

   ```sh
   # Use your package manager
   sudo apt install cmake libgtkmm-4.0-dev libadwaita-1-dev libtinyxml2-dev \
    libspdlog-dev gettext catch2
   ```

2. Clone the project's repository

   ```sh
   git clone https://github.com/smolBlackCat/progress-tracker.git
   ```

3. Configure and compile project.

   ```sh
   cd progress-tracker/

   # Use -DDEVELOPMENT=ON to build the application in development mode
   cmake -S . -B build/ -DCMAKE_INSTALL_PREFIX=/usr
   cmake --build build/
   ```

4. Install the Project

   ```sh
   sudo ninja install -C build/
   sudo glib-compile-schemas /usr/share/glib-2.0/schemas/
   ```

#### Distribution (Linux)

Issuing the `cpack` command will generate both a tarball and debian package
containing the application. Install the appropriate package for your system.

> [!NOTE]
> The tarball contains the expected FHS structure of a POSIX system. Thus installing is as trivial as moving the files to the correct directories. However, the user will still need to update schemas manually.

#### Build (Windows)

The project relies on the MSYS2 environment to build on Windows. The first step
by this project is setting up a [MSYS2 environment](https://www.msys2.org/) and
downloading the MSYS dependencies similar to the one listed before. Currently,
the project build settings support only two environments: UCRT64 and MINGW32.

1. Install the dependencies

   ```sh
   # Replace env with either 'ucrt-x86_64' or 'i686'
   pacman -Syu mingw-w64-env-cmake mingw-w64-env-python mingw-w64-env-gtkmm4 \
       mingw-w64-env-libadwaita mingw-w64-env-tinyxml2 \
       mingw-w64-env-gettext mingw-w64-env-catch mingw-w64-env-spdlog mingw-w64-env-nsis git
   ```

2. Clone the project's repository

   ```sh
   git clone https://github.com/smolBlackCat/progress-tracker.git
   ```

3. Configure and compile project

   ```sh
   cd progress-tracker/
   cmake -S . -B build/
   cmake --build build/
   ```

4. Build the Progress installer

   ```sh
   cd build/

   # This will generate both an installer and a .zip file with the
   # application's files
   cpack
   ```

## Support

I love doing what I do and do not usually mind if I am getting paid for it or
not. However, if you feel like this project deserve to grow, you might like to
donate for the project's maintenance [PayPal][paypal-link]. Thanks for your support!

[paypal-link]: https://www.paypal.com/donate/?hosted_button_id=9E5ELM2GFRU7U
[progress-logo]: data/io.github.smolblackcat.Progress.svg
[progress-flathub]: https://flathub.org/apps/io.github.smolblackcat.Progress
[github-release]: https://github.com/smolBlackCat/progress-tracker/releases
[flathub-badge]: https://flathub.org/assets/badges/flathub-badge-i-en.png
[github-release-badge]: https://img.shields.io/github/v/release/smolBlackCat/progress-tracker?logo=github
[flatpak-release-badge]: https://img.shields.io/flathub/downloads/io.github.smolblackcat.Progress?logo=flathub
[github-licence]: https://img.shields.io/github/license/smolBlackCat/progress-tracker
[build-status]: https://img.shields.io/github/actions/workflow/status/smolBlackCat/progress-tracker/progress-ci.yml
