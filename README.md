# Updater

Software updater, leveraging Git for updating software.

This software is free for open-source software. If you'd like to buy a license for other software, please [contact me](mailto:jonas.aschenbrenner@gmail.com).

Currently only Windows is supported.

## How it works

Basically you install your software as a Git repo with the latest version checked out.
To update the software to the latest version, you copy the Updater.exe into a temporary folder, run it with the required arguments and exit your software.
The updater waits until the software has exited and then updates the software via a Git fetch followed by a checkout of the latest commit.
Then it starts your software and exits.

## Hosting updates repo

### GitHub

You can host your updates repo as a private repo on GitHub, add a deployment key with read-only access to it and embed the private and public key into the updater.

## Build requirements

- Visual Studio
- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-cmd)
