@echo off
setlocal

pushd "%~dp0\.." || exit /b 1

set "QT_ROOT="
if exist "C:\Users\Liam\Qt\6.10.3\mingw_64\bin\qmake.exe" (
    set "QT_ROOT=C:\Users\Liam\Qt\6.10.3\mingw_64"
) else if exist "%USERPROFILE%\Qt\6.10.3\mingw_64\bin\qmake.exe" (
    set "QT_ROOT=%USERPROFILE%\Qt\6.10.3\mingw_64"
)

if defined QT_ROOT set "PATH=%QT_ROOT%\bin;C:\Users\Liam\Qt\Tools\mingw1310_64\bin;%USERPROFILE%\Qt\Tools\mingw1310_64\bin;%PATH%"

if not exist release\Polo.exe (
    call "%~dp0build-windows.bat"
    if errorlevel 1 (
        popd
        exit /b 1
    )
)

release\Polo.exe
set "RUN_EXIT=%errorlevel%"
popd
exit /b %RUN_EXIT%
