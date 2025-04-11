# Building Qt

## Download and Unzip Qt

* Download Qt from https://qt.mirror.constant.com/archive/qt/6.6/6.6.0/single/qt-everywhere-src-6.6.0.zip
* Unzip the downloaded file to a directory of your choice, e.g., `D:\path\to\src\qt-everywhere-src-6.6.0`.

# Building Qt

Run the following commands in a Developer PowerShell for VS 2019 terminal from your `qt-everywhere-src-6.6.0` 
directory, being sure to replace `D:\path\to\SIRVE` with the actual path to your 
SIRVE repository root:

```powershell
mkdir build
cd build
..\configure -debug -prefix D:\path\to\SIRVE\6.6.0\msvc2019_64
cmake --build . --parallel
cmake --install .
del CMakeCache.txt
..\configure -release -prefix D:\path\to\SIRVE\6.6.0\msvc2019_64
cmake --build . --parallel
cmake --install .
```

This sequence of commands will build both the debug and release versions of Qt, but makes sure that the tools are
built in release mode.
