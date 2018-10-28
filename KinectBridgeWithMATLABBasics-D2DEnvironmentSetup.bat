@echo off

REM Check that we have administrator privileges
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo "Error: This script needs to be Run as Adminstrator"
	goto :eof
)

REM Check if the directory provided by the user has the Matlab executable in it.
REM If not, keep prompting the user for the actual directory
set SearchDirectory="C:\Program Files\MATLAB\R2012a"

if /i [%1] neq [] (
	set SearchDirectory=%1
) else (
	if not exist %SearchDirectory%\bin\matlab.exe (
		set SearchDirectory="C:\Program Files (x86)\MATLAB\R2012a"
	)
)

:DirectoryCheck
echo.
echo Checking %SearchDirectory% for MATLAB..
if not exist %SearchDirectory%\bin\matlab.exe (
	echo MATLAB not found in %SearchDirectory%
	echo.

	set /p SearchDirectory=Enter the MATLAB directory: 

	goto :DirectoryCheck
)

echo Found MATLAB in %SearchDirectory%.
echo.

echo.
echo Setting system environment variables...

REM Set MATLAB_DIR
echo.
echo Setting MATLAB_DIR = %SearchDirectory%
setx /m MATLAB_DIR %SearchDirectory%

echo.
echo.
echo DONE
echo Please log out and log back in for the new systen envirionment variables to take effect!