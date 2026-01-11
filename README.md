# InteractiveGraphicsProject

For intellisense in VScode make sure CMake Tools and C/C++ extensions are installed and enabled

Set Working Directory
```console
cd <directory-installed>/InteractiveGraphicsProject
```

Create Build
```console
cmake --preset default 
```

Compile Project
```console
cmake --build --preset default
```

Run Project
```console
.\build\Debug\GraphicsProject.exe 
```

As an alternative to cmake commands a batch file provided can call the identical commands, plus additional commands to do multiple instructions, as long as working directory
is correctly set at the root of the project
```console
.\Project.bat build
.\Project.bat compile
.\Project.bat run
.\Project.bat compile_run
.\Project.bat build_compile_run
```