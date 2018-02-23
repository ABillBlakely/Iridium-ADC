# Source compilation notes.

If mbed cli is setup, the project can be compiled by executing make in this directory.

## Mbed CLI setup:

This guide will help setup a new install of mbed cli on windows. It is reccomended to use a good terminal such as [cmder](http://cmder.net/), although it should work fine in cmd.exe or powershell. If you are not using cmder (which includes git), you will probably need to install [git](https://git-scm.com/) as well.

### First time setup:

This setup uses a virtual env so that mbed can coexist with python 3 installs.

1. install python 2.7
2. setup a python2 virtual env. For example:
```bash
virtualenv -p "path/to/python27/python.exe" mbed-cli
```
3. activate the virtual env:
```bash
mbed-cli\Scripts\Activate.bat
```
4. Check the python version:
```bash
python -V
> Python 2.7.14
```
4. install mbed-cli with pip
```bash
pip install mbed-cli
```
5. Configure compiler path
    * Install a compiler such as [GCC Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
    * configure mbed with location to compilers:
```bash
mbed config -G GCC_ARM_PATH "%USERPROFILE%\AppData\Local\Programs\ARMGCC\7-2017-q4-major\bin"
```
    * Alternative compilers can be setup similarly, see [mbed: compiling code](https://github.com/ARMmbed/mbed-cli#compiling-code) for more.


If a new shell session is started, **the virtual environment must be activated, as in step 3, before any mbed commands may be run.** If using cmder, the shell prompt should change to read "(mbed-cli)" when the virtual environment is active.

### New project setup:

This assumes that an existing, git-managed folder is already setup for the project.

1. Navigate to src directory
```bash
cd src
```
2. Create new program, do not use mbeds source control.
```bash
mbed new --scm=none ./
```



