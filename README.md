# InteractiveGraphicsProject

For intellisense in VScode make sure CMake Tools and C/C++ extensions are installed and enabled

1. Set Working Directory
```console
cd <directory-installed>/InteractiveGraphicsProject
```

2. Create Build System Using CMake in Build Directory:

```console
cmake --build build
```

3. Create Build With Executable in Build Directory:
```console
cmake --build build
```
4. Run Project Executable and Pass Obj File Path to Render as Argument:
```console
.\build\Debug\GraphicsProject.exe res/assets/teapot.obj
```

To use another build directory the previous commands can be substituted with

```console
cmake -B <other-build-directory>

cmake --build <other-build-directory>

.\<other-build-directory>\Debug\GraphicsProject.exe res/assets/teapot.obj
```