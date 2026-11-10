# learning-vulkan
Learning vulkan.

## Dependencies
- Windows

## How to set environments
- Install Vulkan SDK
- Install Visual Studio

## How to open Visual Studio Developer Command Prompt(the shell)
- It's on **start menu - application - visual studio 2026 - x64 Native Tools Command Prompt for VS**

## How to use nmake(make for Windows)
- Write makefile
- cd DIR
- nmake

## How to write makefile
```makefile
CC = cl
CFLAGS = /nologo /O2 /EHsc

main.exe: main.cpp
	$(CC) $(CFLAGS) main.cpp /Fe:main.exe

clean:
	del main.exe main.obj
```
