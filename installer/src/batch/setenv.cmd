@echo off
echo Algorand Command-line Tools for Windows Environment Setup
echo.

pushd ..
for %%* in (.) do set ALGORAND_NETWORK=%%~nx*
set ALGORAND_TOOLS=%cd%
popd

set ALGORAND_DATA=%PROGRAMDATA%\Algorand\data\%ALGORAND_NETWORK%
set PATH=%PATH%;%ALGORAND_TOOLS%

echo * Set ALGORAND_DATA=%ALGORAND_DATA%
echo * Added "%ALGORAND_TOOLS%" to this session PATH.

echo Done.

if "%1" == "/walkback" cd..