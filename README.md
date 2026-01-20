# InteractiveGraphicsProject

For intellisense in VScode make sure CMake Tools and C/C++ extensions are installed and enabled

Set Working Directory
```console
cd <directory-installed>/InteractiveGraphicsProject
```

Create Build System Using CMake
```console
cmake -B build 
```
Create Build
```console
cmake --build build
```
Run Project
```console
.\build\Debug\GraphicsProject.exe res/assets/teapot.obj
```