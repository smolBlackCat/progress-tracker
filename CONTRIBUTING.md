# How to contribute to Progress

## Reporting bugs

To ensure bugs are successfully identified and fixed, bug report messages must
be clear and, whenever possible, include additional context that may be relevant
to solving the bug.

* First of all, ensure that there is no similar report already
* Describe the problem. Taking screenshots or screen recordings is recommended
* Outline your current configuration (e.g., operating system, desktop
environment, window compositor)

Example:

```md
## Problem
Progress is creating duplicates kanban boards instead of saving only one. This
issue occurs every time a new board is to be added to the main page.

[**Possible recording showing issue happening**]

## Environment

* Debian 14
* GNOME
* Wayland

## Logs

```Preferably place log text within a text block to not clog the document```

```

## Submitting Patches

Patches are very welcome and everyone is can propose changes to Progress.

* The patch must have zero conflicts with the main branch
* Provide a brief rationale on why such feature should be added
* Ensure all tests have passed or add more tests for the new feature
* The project builds

### Commit Message

Having a commit message standard simplifies project management and enables other
developers to understand right away what the commit is supposed to modify. 

```txt
(component): Quick Summary

Detailed explanation of changes. You may use a list to indicate a series of
changes.
```

Multiple components compose Progress. Sometimes, modifying one component may
require modifying another; therefore, it is appropriate to explicitly state the
situation in the commit message as well.

* `(core)`: Application core changes or additions.
* `(widgets)`: Widget implementation changes.
* `(window)`: Changes made to the application's window
* `(dialog)`: Application's dialogs changes
* `(i18n)`: Translation updates or a new language support.
* `(chore)`: General project settings changes.

## Translating the Project

As a translator of this project, you may interact with the following files in
the project's source code to add or update translations:

* `po/LINGUAS`: Standard list of available translations
* `po/CMakeLists.txt`: Contains the list of translations available
* `po/progress-tracker.pot`: App's POT file

Steps for translations:

1. Include the country code in `po/CMakeLists.txt` and `po/LINGUAS`. The country
codes list is defined in the `LINGUAS` variable.

    ```cmake
    # Add the country code in alphabetical order
    list(APPEND LINGUAS bg es it nl pt_BR ru_RU tr uk_UA)
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
cmake (preferably with -DDEVELOPMENT=ON) or Flatpak (GNOME Builder). Most
importantly, set the `LANGUAGE` or `LANG` environment variable to the language's
country code you are developing the translation for.

    ```sh
    # Starts the application in Dutch. Note that this assumes that program was
    # installed into the system
    LANGUAGE=nl progress-debug
    ```

    ```sh
    # Starts the flatpak application in Bulgarian
    LANGUAGE=bg flatpak run io.github.smolblackcat.Progress
    ```
