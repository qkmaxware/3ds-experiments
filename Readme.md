<div class="center" style="text-align:center;">
    <img src="environment/default_icon.png">
</div>

# 3DS Homebrew Projects
My home for 3ds homebrew projects in one place. Uses podman+podman-compose to orchestrate building of multiple projects where it is easy to add or remove projects as need be.

To compile projects see [compose.yml](#composeyml).

To test projects, any 3DS emulator can run homebrew apps, but I've been using Azahar for my own testing. 

To run projects on real hardware you must first have a 3DS console capable of running homebrew apps. Afterwards simply download the .3dsx files from the releases section, put them on your SD card under the 3ds folder and launch them from the homebrew app. 

## Files & Directories
### LIB
The lib directory contains c++ files that are shared between all projects. In particular it contains `ez3ds.hpp` which is my 3ds abstraction layer which provides a small number of convenient classes and functions for quickly prototyping 3ds apps.

### ENVIRONMENT
A minimal setup for a container image which is capable of compiling 3ds homebrew apps. This image is built with all required dependencies and then run against each sub-project to build project executables.

### PROJECTS
The projects directory contains several sub-projects each of which generates their own 3ds homebrew app. 

### BIN
The bin directory is where compiled homebrew executables (*.3dsx) will be generated for all projects.

### compose.yml
The compose.yml file lists all the sub-projects in the projects directory and provides a mapping between the build image and the project source files.

To build all projects just use:
```sh
podman compose -f compose.yml run --build --rm all
```

To build a specific project just replace `all` with the name of the project to build. For instance, to build the dvd-bounce project type:
```sh
podman compose -f compose.yml run --build --rm dvd-bounce
```