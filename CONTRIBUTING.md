# How to contribute to Progress

## Sending Issues

It's impossible to write a program that is absolutely perfect. If an application
don't have any issues, it usually means that there are not too much people using
it. Therefore, if any problems are detected in Progress or there are
enhancements to be suggested, feel free to send an issue report.

### Bugs

To help the maintainers solve the bug, there are some things to keep in mind:

* Describe the problem directly as possible
* Inform your system configurations (e.g OS and desktop environment)
* Whenever possible, inform how to reproduce the bug

## Submitting Patches

Before submit a pull request, ensure that:

* Your code builds successfully
* It adheres to the code style defined in `.clang-format`
* Your commits are well-written

### Commit Message

```txt
(component): Quick Summary

Detailed explanation of changes. You may use a list to indicate a series of
changes.
```

When writing a commit message, the component being changed have to be named.
Use these prefixes when writing commits:

* `(core)`: Chnages related to the core functionality or UI behaviour.
* `(ui)`: Changes affecting the app's appearance or stylesheets.
* `(i18n)`: Internationalisation support.
* `(chore)`: General project settings changes.

## Translating the Project

As a translator of this project, you may interact with the following files in
the project's source code to add or update translations:

* `po/CMakeLists.txt`: Contains the list of translations available
* `po/progress-tracker.pot`: App's POT file

Steps for translations:

1. Include the country code in `po/CMakeLists.txt`. The country codes list
is defined in the `PO_NAMES` variable.

    ```cmake
    # Add the country code in alphabetical order
    list(APPEND PO_NAMES bg es it nl pt_BR ru_RU tr uk_UA)
    ```

2. Use **msginit** to generate the new POT file. You may also use **msgmerge**
to update existent translation files.

    ```sh
    # This is an example of generating a Brazilian Portuguese POT file
    msginit -i po/progress-tracker.pot -o po/pt_BR.po -l pt_BR.utf8
    ```

    ```sh
    # Updating the Brazilian Portuguese POT file
    msgmerge -U po/pt_BR.po po/progress-tracker.pot

    # Alternatively, you can simply run ninja/cmake in the build directory, and
    # all the .po files will be updated.
    cd build/
    ninja
    ```

3. Translate the strings.

4. Whenever possible, build the source code and run the application to check if
the translations are properly set up. You can build the application using either
cmake (with `CMAKE_BUILD_TYPE` set to Debug) or Flatpak (GNOME Builder). Most
importantly, set the `LANGUAGE` environment variable to the language's country
code you are developing the translation for.

    ```sh
    # Starts the application in Dutch
    LANGUAGE=nl src/progress-tracker
    ```

    ```sh
    # Starts the flatpak application in Bulgarian
    LANGUAGE=bg flatpak run io.github.smolblackcat.Progress
    ```
