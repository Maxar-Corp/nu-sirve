# Installation

This file documents the steps required to build (compile, link) and run the SIRVE executable. This file will be the primary source of technical documentation for building the SIRVE project.

## PREREQUISITES

In order to build and run this code, a computer must have the following prerequisites. This INSTALLATION procedure assumes you are building the code _on_ and _for_ a 64-bit Windows machine.

- The MSVC 2019 C++ compiler and linker and C++ runtime, which can be installed through the Build Tools for Visual Studio
- QT (currently pegged at version 5.15.2)
- Some C++ libraries (armadillo, fftw, opencv)

## INSTALLING THE PREREQUISITES (One-Time Installation Steps)

<details>
    <summary>Click Here to View One-Time Installation Instructions</summary>
    <p>

        #### MSVC: To install the MSVC 2019 C++ compiler and linker as well as the C++ runtime libraries, install the "Build Tools for Visual Studio 2019."  As of 5/21/2024, this was available at https://visualstudio.microsoft.com/vs/older-downloads/

        On a Maxar-provided laptop, this can be installed from the Software Center. On any other system, it can be obtained from https://visualstudio.microsoft.com/vs/older-downloads/. Note that this code will not run with Visual C++ 2022 - the installation and codebase assumes Visual C++ 2019 in a number of places.

        This should place a suite of folders at "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC" that contains the required C++ dependencies.

        #### QT: The QT dependency was committed and included alongside SIRVE. There are no installation steps required.
        The following is documentation of the steps taken to obtain the QT headers and libraries.
        - I installed Python 3.9.13
        - I installed the pip module `aqtinstall` at version 3.1.5
        - I followed these instructions: https://aqtinstall.readthedocs.io/en/latest/getting_started.html
        - The specific `aqt` command I executed to install QT (along with the `QtCharts` module) for this project is as follows:
        `python -m aqt install-qt windows desktop 5.15.2 win64_msvc2019_64 -m qtcharts`
        - I then copied the 5.15.2 folder alongside the source code and checked it into git

        #### C++ LIBRARIES: All C++ libraries should live in the same top-level (root) folder as the README.md file (the 'SIRVE' folder).

        ##### Armadillo: Armadillo, a C++ math library, is installed by following these steps:
            1. Download the .tar.xz file from https://sourceforge.net/projects/arma/files/armadillo-12.2.0.tar.xz/download
            2. Unzip that file with the command (I run this from git bash, it needs to be modified for the Windows cmd prompt): `tar xvf armadillo-12.2.0.tar.xz`
            Fortunately, the Armadillo library comes pre-packaged with its x64 Windows dependencies, so no additional steps are needed.
            
            POSTREQ: Ensure the `armadillo-12.2.20` folder exists alongside the README.md file.

        ##### OpenCV: The opencv library is installed by following these steps:
            1. Download the opencv self-installer `.exe` file for version 4.7.0 from https://opencv.org/releases/
            2. Run the `.exe` file, which will extract it to a folder
            
            POSTREQ: Ensure the `opencv` folder exists alongside the README.md file.

        ##### FFTW: The fftw library was committed and included alongside SIRVE. There are no additional installation steps required.
            The following is documentation of the steps taken to obtain the FFTW library.
            - I downloaded the -dll64.zip file from https://www.fftw.org/install/windows.html
            - I unzipped that file
            - I used the three `lib` commands in a VS Developer command prompt per the site's instructions
            - Note that it may be possible to compile a newer version of fftw (e.g. 3.3.10) using a combination of `cmake .` and `msbuild` or similar, but I would want to benchmark performance and haven't gone down this path yet.
    
    </p>
</details>

## HOW THE EXE IS BUILT

This project is a QT Gui application. As such, the build/link steps are based on the included QT project (`.pro`) file, which fully documents the project requirements. Any generated Makefiles, `.vcxproj` files, or other build-related configuration files are created as a by-product of the `.pro` file and should not be committed. A brief overview of QT project files can be found here: https://doc.qt.io/qt-5/qmake-project-files.html

At a high level, the project file includes sections for:
    - The HEADER, FORMS, and SOURCE files, which are the application source code
    - The INCLUDEPATH lines, which point at the source code for dependencies for compilation
    - The LIBS lines, which point at the libraries for dependencies for linking
    - The INSTALLS lines, which move all files required for execution alongside the deployable `.exe`

## BUILDING THE EXE

Once the pre-requisites are properly installed, the included `.pro` file can be used to generate the `.exe` output.

1. Open the "x64 Native Tools Command Prompt for VS 2019". This will open a `cmd` prompt with the right environment variables set to build the `.exe`.
    - Alternative method to configure the right environment variables: open a regular command prompt and execute the `vcvars64.bat` file from "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build"
2. Navigate to the subdirectory in this repo with the source code (one folder down, `SirveApp`)
3. Run the following command: `..\5.15.2\msvc2019_64\bin\qmake.exe`. (Yes, that is a '..' followed by a slash.  You will be executing qmake by dropping down a directory and then going UP three directories!  This command must be copied exactly as shown and then pasted to command line while you are in  the 'SirveApp' folder!!)
    This will generate three `Makefile` files. Note that you can manually delete these `Makefile`s and re-run qmake if problems arise.
4. Once you have used qmake to make the nmake files, run the following command: `nmake`.  This will use those generated Makefiles to compile and link all the code. The resulting `.exe` file will be placed in a `release` folder alongside the README file.
5. If you have not yet run it, run the following command: `nmake install`. This will copy all the required runtime dependencies alongside the `.exe` file. It only needs run once, but the SIRVE executable will fail to run if these libraries are not available.  Note that the list of files copied over can be found in the SirveApp.pro file INSTALLS directives.

If everything succeeded, the release folder alongside the README.md file will be fully sufficient to run the code on a machine with the C++ runtime installed.

## Shipping the EXE

The release folder (including the exe, all dll files, `config` folder, `icons` folder, and `platforms` folder) is sufficient to run SIRVE on any 64-bit Windows computer with MSVC2019 installed. Artifacts from running SIRVE, such as the `logs` or `workspace` folders, should be left out.

To have a fully portable distribution folder, which will execute even if the users don't have VS installed, the release folder should also contain the C++ runtime DLLs or the `vc_redist.x64.exe` file which will install the required C++ runtime DLLs. It's unclear to me right now if that is a distribution requirement for SIRVE. Documentation around finding the runtime DLLs can be found here: https://learn.microsoft.com/en-us/cpp/windows/determining-which-dlls-to-redistribute
