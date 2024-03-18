# Progress

![Application's Window](pictures/progress-app-presentation.png)

Progress is a TODO app that uses the kanban-style to organise tasks. The uses
varies from keeping track of a project's progress to organising personal tasks. It allows the user to edit the board using an idiomatic drag-and-drop system, and also
customise boards according to the user preference.

## Installation

The project offers some alternatives for installing and utilising Progress. The distribution used should not affect the normal usage of the application.

Currently, the project is available in the distributions described below:

* Debian package
* Build from source

### Debian-based distros

A debian package is provided so debian-based distro users don't need to download
the dependencies themselves. All they have to do is download the .deb package
and install it using apt-get (or apt).

```sh
sudo apt install ./progress-tracker-1.0.deb
```

After that, the system should have updated the desktop database and the
application will appear in the app menu to be started from there.

### Building from source

If you want to develop the app or distribute it in another format, you might
want to download the source code and build it yourself.

#### Dependencies

Keep it in mind that the dependencies' name listed here will change depending
on the distribution you are building the application on. The ones listed here
are Debian dependencies. Install the appropriate dependencies from your package
manager.

* cmake
* libgtkmm-4.0-dev
* libtinyxml2-dev
* gettext
* catch2 (for testing)

#### Build instructions

1. Install the dependencies needed to build the project

    ```sh
    # catch2 is totally optional. Install it only if you're going to run tests
    sudo apt install libgtkmm-4.0-dev libtinyxml2-dev gettext catch2
    ```

2. Clone the project's repo

    ```sh
    git clone https://github.com/smolBlackCat/progress-tracker.git
    ```

3. Configure and compile the project

    ```sh
    # Set -DCMAKE_BUILD_TYPE as "Debug" for debugging tasks
    cmake -DCMAKE_BUILD_TYPE=Release -S project-root -B build-dir
    ```

4. Install the project

    ```sh
    cd build-dir
    sudo make install
    ```

## Running the Project

After installing, simply issue the command `progress-tracker`, or you can also
start it from the applications menu of your desktop environment.

![Progress Icon](pictures/progress-in-appmenu.png)
