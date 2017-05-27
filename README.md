# revengeMusic

Written in C++11

A command line music program for Windows/Linux.
Inter-process communication [(IPC)](https://en.wikipedia.org/wiki/Inter-process_communication) is utilized to ensure only one instance of revengeMusic is used at a time. When a second instance of revengeMusic is launched (Client), it's used to command the first instance (Player). Designed for use with shell, it enables the user to find creative uses by combining shortcut keys and program arguments.

Inspired by rofi; one of the goals is to work seamlessly with rofi.

# Building
## Dependencies
  - [FMOD API](http://www.fmod.org/download/)
    - **For Linux**
      - Install the 'low level' headers to `/usr/include/fmodex/`
      - Install the 'low level' libraries to `/usr/lib/`
    - **For Windows**
      - FMOD has its own installer, nothing needs to be done manually
  - [Boost 1.58.0](http://www.boost.org/users/download/)
  - [CMake 3.4](https://cmake.org/download/) (recommended)

## Compiling
  - **For Linux**
    ``` bash
    mkdir -p build/
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make
    ```
  - **For Windows**
    - Make sure cmake and msbuild is in PATH
    - Run `build.cmd`
    - Move a copy of fmod.dll from the FMOD install folder into `revengeMusic/build/Debug/`

# Usage
```
./revengeMusic (--commands | <path>)
    commands:
        -h, --help      Shows this message
        -v, --version   Show version number
        -subdir         Specify a specific folder within the Music directory
        kill            Exits revengeMusic
        play            Unpause song
        pause           Pause song
        next            Play next song, next song is decided based on whether shuffle toggled on/off
        prev            Play previous song
        shuffle         Toggles shuffle on/off
        loop-file       Loops the current song
```
