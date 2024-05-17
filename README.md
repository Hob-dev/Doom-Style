Doom-Style Level Editor

Instructions -



Prerequisites -
1. GCC GNU compiler
2. GDB/MinGW64-w64

For Windows, use MSYS2 to install both MinGW and GCC (https://www.msys2.org/#installation)

For MacOS, sudo port install mingw-w64 (installs GCC as well)

Building and Running -

Doom-Style comes with executables, but you can also build them if you want.

If you want to build them:

(g++ compiles cpp as exectuables)

Editor:

  g++ -fdiagnostics-color=always -g "\path\to\Doom-Style\src\editor.cpp" -o "\path\to\Doom-Style\editor.exe" "-IC:/path/to/Doom-Style/src/include" "-LC:/path/to/Doom-Style/src/lib" -lSDL2 -lSDL2main

Renderer:

  g++ -fdiagnostics-color=always -g "\path\to\Doom-Style\src\rendering.cpp" -o "\path\to\Doom-Style\rendering.exe" "-IC:/path/to/Doom-Style/src/include" "-LC:/path/to/Doom-Style/src/lib" -lSDL2 -lSDL2main

If you want to run them:

(Note: executables must be in folder with SDL2.dll)

cd "path\to\Doom-Style"

editor.exe

rendering.exe


or


run the exectuables from the folder
