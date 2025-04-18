# Installation (CMake)

This file documents the steps required to build, run, and debug the SIRVE executable.

## Table of Contents

<!--TOC-->
<!--TOC-->

This table of contents was generated using the [`md-toc`](https://pypi.org/project/md-toc/) Python package.

## Prerequisites

SIRVE requires the following software to be installed on your computer:

* Build Tools for Visual Studio 2019
* CMake
* Ninja
* Python
* Qt 6.6.0 (included with the project)

Note that Visual Studio actually comes with its own CMake and Ninja, but it's
still recommended to install the standalone versions for convenience.

It is also recommended to install 7-Zip, which makes it easier to extract files
that are not in the `.zip` format. You can download 7-Zip from
[7-Zip.org](https://www.7-zip.org/), or from the Maxar Software Center.

### Build Tools for Visual Studio 2019

On a Maxar-provided laptop, this can be installed from the Maxar Software Center.

* Download the Visual Studio 2019 installer from
  [Visual Studio Older Downloads](https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2019-and-other-products).
* Run the installer. 
* Select `Desktop Development with C++`, and click the `Install` button. You can
  choose other components, but they are not necessary for building SIRVE.

### CMake

On a Maxar-provided laptop, this can be installed from the Maxar Software Center.

* Download the Windows x64 installer for CMake: [cmake-3.31.5-windows-x86_64.msi](https://github.com/Kitware/CMake/releases/download/v3.31.5/cmake-3.31.5-windows-x86_64.msi)
* Run the installer and follow the instructions.

### Ninja

Ninja doesn't have an installer, so you'll need to download the binary and place
it in a directory in your system's PATH.

* Download the Windows x64 binary for Ninja: [ninja-win.zip](https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip)
* Unzip the file and place the `ninja.exe` in a directory that is in your 
  system's PATH. Alternatively, you can place the `ninja.exe` in the place 
  of your choice and add that directory to your system's PATH.

### Python

Note that if you have already installed Python as described in the
[INSTALLATION.md](INSTALLATION.md) file, you can skip this section.

You can also install Python from the Microsoft Store, or the Maxar Software 
Center. Any version 3.9 or later is fine.

* Download the Windows x64 installer for Python: [python-3.13.2-amd64.exe](https://www.python.org/ftp/python/3.13.2/python-3.13.2-amd64.exe)
* Run the installer and follow the instructions.

### Qt

Qt is already included in the SIRVE project, so you don't need to install it.

These are the steps taken to get the QT headers and libraries:

* Use `pip` to install the [`aqtinstall`](https://pypi.org/project/aqtinstall/) module.
* Run the following command to install QT (along with the `QtCharts` module) for
  this project:

```powershell
python -m aqt install-qt windows desktop 6.6.0 win64_msvc2019_64 -m qtcharts
```

## Dependencies

Note that if you have already installed the dependencies as described in the
[INSTALLATION.md](INSTALLATION.md) file, you can skip this section.

### Armadillo

* Download the `.tar.xz` file for Armadillo version 12.2.0:
  [armadillo-12.2.0.tar.xz](https://sourceforge.net/projects/arma/files/armadillo-12.2.0.tar.xz/download)
* Use 7-Zip or another tool to extract the contents of the file into the root
  directory of the SIRVE project. This should create a folder named
  `armadillo-12.2.0`.

If you don't have and don't want to install 7-Zip, you can use the the Git Bash
shell to extract the contents of the file. Here's how you can do it:

```bash
tar xvf armadillo-12.2.0.tar.xz
```

### OpenCV

* Download the self-installer `.exe` file for OpenCV version 4.7.0:
  [opencv-4.7.0-windows.exe](https://github.com/opencv/opencv/releases/download/4.7.0/opencv-4.7.0-windows.exe)
* Run the installer, select your SIRVE project directory as the installation
  directory, and click `Extract`.

### FFTW

FFTW is already included in the SIRVE project, so you don't need to install it.

The instructions on how to get the FFTW headers and libraries for Windows are 
here: [FFTW Installation on Windows](https://www.fftw.org/install/windows.html).

## Building SIRVE

To build SIRVE, you need to follow these steps:

1. Open a Visual Studio 2019 Developer Command Prompt.
2. Navigate to the root directory of the SIRVE project.
3. Create a build directory and navigate to it:

```powershell
mkdir build
cd build
```

4. Run CMake to configure the project:

```powershell
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install
```

5. Build the project:

```powershell
ninja
```

## Shipping SIRVE

Make sure you've followed the steps from the previous section.From the 
`SIRVE/build` directory, run the following command:

```powershell
ninja install
```

This will install SIRVE in the `build/install` directory. You can simply 
put that directory into a ZIP file and distribute it to other users, it contains
all the necessary files to run SIRVE.
