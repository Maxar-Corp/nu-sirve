# SIRVE

This file documents the steps required to build (compile, link) and run the SIRVE executable.
This file will be the primary source of technical documentation for building the SIRVE project and should be maintained as development continues.

## PREREQUISITES

In order to build and run this code, a computer must have the following prerequisites. Full installation steps can be found below.
This README assumes you are building the code _on_ and _for_ a 64-bit Windows machine.

- The MSVC C++ compiler and linker and C++ runtime, which can be installed through the Build Tools for Visual Studio
- QT (currently pegged at version 5.15.2)
- Requisite C++ libraries (boost, armadillo, fftw, opencv)

## INSTALLING THE PREREQUISITES

#### MSVC: To install the MSVC C++ compiler and linker as well as the C++ runtime libraries, install the "Build Tools for Visual Studio 2019."
This should place a suite of folders at "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC" that contains the required C++ dependencies.

#### QT: The full QT source was committed and included alongside SIRVE, so there should be no installation steps required.
To document the steps I took, however, I installed Python 3.9.13, the pip module `aqtinstall` at version 3.1.5, and then following these instructions:
https://aqtinstall.readthedocs.io/en/latest/getting_started.html
The specific `aqt` command executed to install QT (along with the `QtCharts` module) for this project is as follows:
`python -m aqt install-qt windows desktop 5.15.2 win64_msvc2019_64 -m qtcharts`

#### C++ LIBRARIES: All C++ libraries should live in the same top-level (root) folder as this README.md file.

##### Boost: The boost library, used for logging, is installed by following these steps:
    1. Download the boost library from https://www.boost.org/doc/libs/1_81_0/more/getting_started/windows.html#get-boost
    2. Execute the `bootstrap` and `b2` commands per these installation instructions: https://www.boost.org/doc/libs/1_81_0/more/getting_started/windows.html#simplified-build-from-source
    3. NOTE: There is an annoying bug in the current source of the boost library for our configuration. 
	- In order to fix it, you must modify the `boost/mpl/bitand.hpp` file by adding the two changes in this PR: https://github.com/boostorg/mpl/pull/57
        - A reference to this bug causing issues with QT can be found here: https://bugreports.qt.io/projects/QBS/issues/QBS-1632?filter=allopenissues
        - We so far only need to patch the `bitand.hpp` file but we may need to fix the other files as well.  
    POSTREQ: Ensure the `boost_1_81_0` folder exists alongside this README.md file and it contains a `stage` folder.

##### Armadillo: Armadillo, a C++ math library, is installed by following these steps:
    1. Download the .tar.xz file from https://arma.sourceforge.net/download.html
    2. Unzip that file with the command (I run this from git bash, it needs to be modified for the Windows cmd prompt): `tar xvf armadillo-12.2.0.tar.xz`
    Fortunately, the Armadillo library comes pre-packaged with its x64 Windows dependencies, so no additional steps are needed.
    POSTREQ: Ensure the `armadillo-12.2.20` folder exists alongside this README.md file.

##### OpenCV: The opencv library is installed by following these steps:
    1. Download the opencv self-installer `.exe` file for version 4.7.0 from https://opencv.org/releases/
    2. Run the `.exe` file, which will extract it to a folder
    POSTREQ: Ensure the `opencv` folder exists alongside this README.md file.

##### FFTW: The fftw library was committed and included alongside SIRVE, so there should be no installation steps required.
    To document the steps I took, however, I downloaded the -dll64.zip file from https://www.fftw.org/install/windows.html, unzipped it, and used the `lib` commands in a VS Developer command prompt per the site's instructions.
    It is possible to compile a newer version of fftw (e.g. 3.3.10) using a combination of `cmake .` and `msbuild` or similar, but I would want to benchmark performance and haven't gone down this path yet.

## HOW THE EXE IS BUILT

This project is a QT Gui application. As such, the build/link steps are based on the included QT project (`.pro`) file, which fully documents the project requirements.
Any generated Makefiles, `.vcxproj` files, or other build-related configuration files are created as a by-product of the `.pro` file and should not be committed.
A brief overview of QT project files can be found here: https://doc.qt.io/qt-5/qmake-project-files.html

At a high level, the project file includes sections for:
    - The HEADER, FORMS, and SOURCE files, which are the application source code
    - The INCLUDEPATH lines, which point at the source code for dependencies
    - The LIBS lines, which point at the libraries for dependencies
    - The INSTALLS lines, which move all required files alongside the deployable `.exe`

## BUILDING THE EXE

Once the pre-requisites are properly installed, the included `.pro` file can be used to generate the `.exe` output.

1. Open the "x64 Native Tools Command Prompt for VS 2019". This will open a `cmd` prompt with the right environment variables set to build the `.exe`.
    - Alternative method to configure the right environment variables: open a regular command prompt and execute the `vcvars64.bat` file from "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build"
2. Navigate to the subdirectory in this repo with the source code (one folder down, `SirveApp`)
3. Run the following command: `..\5.15.2\msvc2019_64\bin\qmake.exe`.
    This will generate three `Makefile` files.
4. Run the following command: `nmake`.
    This will use those generated Makefiles to compile and link all the code. The resulting `.exe` file will be placed in a new `release` folder alongside this README file.
5. Run the following command: `nmake install`.
    This will place all the required runtime dependencies alongside the `.exe` file.

If everything succeeded, the release folder alongside this README file will be fully sufficient to run the code on a machine with the C++ runtime installed.

To have a fully portable distribution folder, which will execute even if the users don't have VS installed, the release folder should also contain the C++ runtime DLLs.
It's unclear to me right now if that is a distribution requirement for SIRVE. Documentation around finding the runtime DLLs can be found here: https://learn.microsoft.com/en-us/cpp/windows/determining-which-dlls-to-redistribute
