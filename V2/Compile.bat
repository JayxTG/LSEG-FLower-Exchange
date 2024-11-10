@echo off
echo Compiling Order Matching Program...
g++ -std=c++11 main.cpp order_matcher.cpp -o order_matcher
if %ERRORLEVEL% neq 0 (
    echo Compilation failed.
    pause
    exit /b %ERRORLEVEL%
)
echo Compilation successful.
pause
