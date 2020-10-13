@echo off

set msbuild="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"

set srcRea="..\frm" 
set buildRea="..\buildRea"

if exist %buildRea% (
    rd /s /q %buildRea%
)
mkdir %buildRea%

cmake -S %srcRea% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildRea% -DMS=ON
%msbuild% %buildRea%\ALL_BUILD.vcxproj /p:Configuration=Release

set srcApp="..\app"
set buildApp="..\buildApp"

if not exist %buildApp% (
    mkdir %buildApp%
) else (
    echo %buildApp% exist
)

cmake -S %srcApp% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildApp%
%msbuild% %buildApp%\ALL_BUILD.vcxproj /p:Configuration=Release

set srcShp="..\dll"
set buildShp="..\buildShp"

if exist %buildShp% (
    rd /s /q %buildShp%
)
mkdir %buildShp%

cmake -S %srcShp% -DCMAKE_BUILD_TYPE=Release -A x64 -B %buildShp% -DMS=%buildApp%\Release\plugin
%msbuild% %buildShp%\ALL_BUILD.vcxproj /p:Configuration=Release

call recordVersion %buildApp%\Release\.version

xcopy %buildApp%\Release\plugin\Release\* %buildApp%\Release\plugin /e /y
rd /s /q %buildApp%\Release\plugin\Release

mkdir %buildApp%\Release\minIO
xcopy .\minIO\* %buildApp%\Release\minIO /e /y
mkdir %buildApp%\Release\qtinstall
xcopy .\qtinstall\* %buildApp%\qtinstall /e /y

xcopy %buildApp%\Release\* %buildApp%\qtinstall\mypackages\content\data\ /e /y
xcopy .\pack\* %buildApp%\qtinstall\mypackages\content2\data\ /e /y
xcopy %buildApp%\Release\* %buildApp%\qtinstall\mypackages\content3\data\ /e /y

cd pack
WINPACK.exe
cd ..

D:\qt-installer\bin\binarycreator.exe -c %buildApp%\qtinstall\config\config.xml -p %buildApp%\qtinstall\mypackages DeepInspectionV4.exe -v 

xcopy ..\frm\install\* ..\frm-company\install /e /y
xcopy ..\frm\include\* ..\frm-company\include /e /y
xcopy ..\dll2\* ..\frm-company\plugin /e /y
xcopy DeepInspectionV4.exe ..\frm-company /y
:: "C:/Program Files/7-Zip/7z.exe" a -tzip DeepInspectionBinary.zip Release/*