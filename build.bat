@echo off

set VISUAL_STUDIO_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"

pushd %VISUAL_STUDIO_PATH%
call vcvarsall x86
popd

call clean.bat
msbuild.exe currency_conversion.sln /t:Build /p:Configuration=Debug;Platform=x86