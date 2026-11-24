@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
cd /d %~dp0

if "%1"=="web" (
    echo Opening CineBrew Web Playground in browser...
    start web\index.html
    exit /b 0
)

if not exist build mkdir build
cd build
if not exist CMakeCache.txt cmake ..
cmake --build .
cd ..

set EXEPATH=
if exist build\bin\cinebrew.exe set EXEPATH=build\bin\cinebrew.exe
if not defined EXEPATH if exist build\bin\cinebrew set EXEPATH=build\bin\cinebrew

if not defined EXEPATH (
    echo [ERROR] CineBrew executable not found in build\bin\
    exit /b 1
)

if "%1"=="" goto MENU
if /i "%1"=="snake" goto RUN_SNAKE
if /i "%1"=="pong" goto RUN_PONG
if /i "%1"=="hello" goto RUN_HELLO
if /i "%1"=="factorial" goto RUN_FACT

%EXEPATH% %*
exit /b 0

:MENU
echo =======================================================
echo          CINEBREW COMPILER & GAME ENGINE
echo =======================================================
echo  [1] Run Snake Game (60 FPS Native Win32)
echo  [2] Run Pong Game (60 FPS Native Win32)
echo  [3] Run Hello World
echo  [4] Open Interactive Web Playground in Browser
echo  [5] Exit
echo =======================================================
set /p choice="Choose an option [1-5]: "

if "%choice%"=="1" goto RUN_SNAKE
if "%choice%"=="2" goto RUN_PONG
if "%choice%"=="3" goto RUN_HELLO
if "%choice%"=="4" goto OPEN_WEB
if "%choice%"=="5" exit /b 0
echo Invalid choice.
exit /b 1

:RUN_SNAKE
echo Starting Snake Game...
%EXEPATH% examples\snake_game.cb
exit /b 0

:RUN_PONG
echo Starting Pong Game...
%EXEPATH% examples\pong_game.cb
exit /b 0

:RUN_HELLO
%EXEPATH% examples\hello.cb
exit /b 0

:RUN_FACT
%EXEPATH% examples\factorial.cb
exit /b 0

:OPEN_WEB
echo Opening CineBrew Web Playground in browser...
start web\index.html
exit /b 0
