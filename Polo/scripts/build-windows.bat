@echo off
setlocal

pushd "%~dp0\.." || exit /b 1

set "QT_ROOT="
set "MINGW_ROOT="
set "QMAKE="
set "MAKE="

if exist "C:\Users\Liam\Qt\6.10.3\mingw_64\bin\qmake.exe" (
    set "QT_ROOT=C:\Users\Liam\Qt\6.10.3\mingw_64"
    set "MINGW_ROOT=C:\Users\Liam\Qt\Tools\mingw1310_64"
) else if exist "%USERPROFILE%\Qt\6.10.3\mingw_64\bin\qmake.exe" (
    set "QT_ROOT=%USERPROFILE%\Qt\6.10.3\mingw_64"
    set "MINGW_ROOT=%USERPROFILE%\Qt\Tools\mingw1310_64"
) else if exist "C:\Users\Liam\Qt\6.10.3\msvc2022_64\bin\qmake.exe" (
    set "QT_ROOT=C:\Users\Liam\Qt\6.10.3\msvc2022_64"
) else if exist "%USERPROFILE%\Qt\6.10.3\msvc2022_64\bin\qmake.exe" (
    set "QT_ROOT=%USERPROFILE%\Qt\6.10.3\msvc2022_64"
)

if defined QT_ROOT (
    set "QMAKE=%QT_ROOT%\bin\qmake.exe"
    set "PATH=%QT_ROOT%\bin;%PATH%"
)

if defined MINGW_ROOT (
    set "PATH=%MINGW_ROOT%\bin;%PATH%"
    if exist "%MINGW_ROOT%\bin\mingw32-make.exe" set "MAKE=%MINGW_ROOT%\bin\mingw32-make.exe"
)

if not defined QMAKE (
    where qmake6 >nul 2>nul
    if not errorlevel 1 set "QMAKE=qmake6"
)

if not defined QMAKE (
    where qmake >nul 2>nul
    if not errorlevel 1 set "QMAKE=qmake"
)

if not defined QMAKE (
    echo qmake6 or qmake was not found. Install Qt or add Qt bin to PATH.
    popd
    exit /b 1
)

if not defined MAKE (
    where mingw32-make >nul 2>nul
    if not errorlevel 1 set "MAKE=mingw32-make"
)

if not defined MAKE (
    where nmake >nul 2>nul
    if not errorlevel 1 set "MAKE=nmake"
)

if not defined MAKE (
    where jom >nul 2>nul
    if not errorlevel 1 set "MAKE=jom"
)

if not defined MAKE (
    echo nmake, mingw32-make, or jom was not found. Install the matching Qt build tools.
    popd
    exit /b 1
)

"%QMAKE%" Polo.pro
if errorlevel 1 (
    popd
    exit /b 1
)

"%MAKE%"
set "BUILD_EXIT=%errorlevel%"
popd
exit /b %BUILD_EXIT%
