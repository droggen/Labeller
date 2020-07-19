@echo off

echo Packaging...

IF "%1"=="" GOTO HAVE_0

rd /q /s windows
del /q labeller-win.zip
md windows
%1\windeployqt.exe build-labeller-Desktop_Qt_5_15_0_MSVC2017_64bit-Release\release\labeller.exe --dir windows --force

echo Copy binary

copy build-labeller-Desktop_Qt_5_15_0_MSVC2017_64bit-Release\release\labeller.exe windows

echo Copy additional dlls

copy "%1\libstdc++-6.dll" windows
copy %1\libwinpthread-1.dll windows
rem copy %1\libgcc_s_dw2-1.dll windows
copy %1\libgcc_s_seh-1.dll windows


echo Zipping

cd windows
zip -r ..\labeller-win.zip * -x *.svn*
cd..
zip -r labeller-win.zip examples\* -x *.svn*

echo Packaging done

exit /b

:HAVE_0
echo Specify the path to qt. Example: %0 c:\Qt\5.15.0\msvc2017_64\bin
