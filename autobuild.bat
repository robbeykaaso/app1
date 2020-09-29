@echo off

set msbuild="C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe"

set srcRea="../frm" 
set buildRea="../buildRea"

if not exist %buildRea% (
    mkdir %buildRea%
) else (
    echo %buildRea% exist
)

cmake -S %srcRea% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildRea% -DMS=ON
%msbuild% %buildRea%/ALL_BUILD.vcxproj /p:Configuration=Release

set srcApp="../app"
set buildApp="..\buildApp"

if not exist %buildApp% (
    mkdir %buildApp%
) else (
    echo %buildApp% exist
)

cmake -S %srcApp% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildApp%
%msbuild% %buildApp%/ALL_BUILD.vcxproj /p:Configuration=Release

set srcShp="../dll"
set buildShp="../buildShp"

if not exist %buildShp% (
    mkdir %buildShp%
) else (
    echo %buildShp% exist
)

cmake -S %srcShp% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildShp% -DMS=%buildApp%/Release/plugin
%msbuild% %buildShp%/ALL_BUILD.vcxproj /p:Configuration=Release

xcopy %buildApp%\Release\plugin\Release\* %buildApp%\Release\plugin /e /y
rd /s /q %buildApp%\Release\plugin\Release

cd pack
WINPACK.exe
cd ..

mkdir %buildApp%\Release\minIO
xcopy .\minIO\* %buildApp%\Release\minIO /e /y
