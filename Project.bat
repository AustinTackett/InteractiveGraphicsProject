@echo off

if /i "%~1"=="build" (
    cmake --preset default     
) else if /i "%~1"=="compile" (
    cmake --build --preset default
) else if /i "%~1"=="run" (
    .\build\Debug\GraphicsProject.exe res/assets/teapot.obj
) else if /i "%~1"=="compile_run" (
    cmake --build --preset default
    .\build\Debug\GraphicsProject.exe res/assets/teapot.obj
) else if /i "%~1"=="build_compile_run" (
    cmake --preset default
    cmake --build --preset default
    .\build\Debug\GraphicsProject.exe res/assets/teapot.obj
) else (
    echo Unknown command: %~1
)