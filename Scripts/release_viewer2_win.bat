@echo off
setlocal
set CURDIR=%~dp0
set BASEDIR=%CURDIR%..
set BUILDDIR=%BASEDIR%\Build
set DEPENDDIR=%BUILDDIR%\Depends
set TOOLSDIR=%BASEDIR%\Tools
for /f "tokens=2 delims==" %%I in (
  'wmic cpu get architecture /value'
) do set "cpuArch=%%I"
if "%cpuArch%"=="12" (
  set HOST_ARCH=arm64
) else (
  set HOST_ARCH=x64
)
set TARGET_ARCH=%HOST_ARCH%
@echo on

if not "%1" == "" (
  set TARGET_ARCH=%1
)

call "%CURDIR%\build_viewer2_win.bat" Release %TARGET_ARCH% || exit /b 1

pushd %BASEDIR%
rmdir /S /Q viewer_sample_2
mkdir viewer_sample_2
copy "%BUILDDIR%\Viewer2\cmakeBuild\SSView2_artefacts\Release\SSViewer2.exe" viewer_sample_2\
set ZIPNAME=viewer_sample_2_%TARGET_ARCH%
powershell compress-archive viewer_sample_2 %ZIPNAME%.zip
move /y %ZIPNAME%.zip %TOOLSDIR%\
rmdir /S /Q viewer_sample_2
popd