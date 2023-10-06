# Debugging

SIRVE should be debuggable inside an IDE like VSCode - however, there are a variety of issues that have not yet been resolved in the compilation and linking steps. Steps to debug the code using WinDbg can be found below.

## PREREQUISITES

In order to debug the SirveApp executable, you must have followed [the INSTALLATION README's](./INSTALLATION.md) instructions at least once to successfully generate a non-debug executable.

You must have the debug tools for Visual Studio 2019 installed in order to run and use WinDbg. If correctly installed, you will find the `WinDbg.exe` at C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe. Note that on a Maxar-provided laptop, the Software Center "Build Tools for Visual Studio 2019" installation will not include the debuggers. As of May 2023, a support ticket must be created to get that installed.

## Building the Debug Exe

Building the debug `SirveApp.exe` file is as simple as running `nmake debug` instead of just `nmake`. It will build the debug executable into a `..\debug` folder (instead of the `..\release` folder) and should additionally place a `.pdb` file alongside the `.exe` (`.pdb` files include debug symbols).

## Running the Debug Exe

In order to run the debug exe, the `config`, `icons`, and `platforms` folders as well as all of the dependency `.dll` files must be copied from the `..\release` folder into the `..\debug` folder. However, some of the dlls must be replaced with _debuggable_ dll variants:

- All `Qt5___.dll` files must be replaced with `Qt5___d.dll` files
- The `platforms\qwindows.dll` file must be replaced with `platforms\qwindowsd.dll` file.

The source of these files can be found in the `SirveApp.pro` INSTALLS directives (those directives are used by the `nmake install` command in the INSTALLATION instructions).

If the appropriate `.dll` files are copied into the `..\debug` folder, the exe should execute as normal. It will simply be slower than the normal SirveApp executable.

## Debugging the Exe

One way to debug the executable is to first run the debuggable executable and, while it is running, open WinDbg (x64) and use the "File->Attach to a Process" command to attach to `SirveApp.exe`. When it is attached, it will freeze execution of SirveApp and wait for instructions.

In the WinDbg command window, the following list of commands will set a breakpoint and evaluate the state of the program at said breakpoint. Note that all of this can also be done visually within the options on WinDbg's top bar.

1.	Type `bu SirveApp!SirveApp::ui_load_osm_file` for example to set a breakpoint at your choice of ClassName::method_name. 
2.	Type `bl` to list the breakpoints you’ve set so you can see what’s happening.
3.	Type `g` to execute the program until it hits a breakpoint
4.	When WinDbg has stopped program execution at a breakpoint, a variety of commands like `dv` or `x` to display local variables, `?` to execute any C++ command you wish, etc. can be used to debug the application state.

A full WinDbg reference with all commands [can be found here](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/commands).