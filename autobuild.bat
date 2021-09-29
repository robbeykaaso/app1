@echo off

if "%1"=="" (
    set buildType=Release
) else (
    set buildType=%1
)

if "%2"=="" (
    set packProject=DeepInspection
) else (
    set packProject=%2
)

set msbuild="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
set installer="D:\qt-installer\bin\binarycreator.exe"

set srcRea="..\frm" 
set buildRea="..\buildRea"

if exist %buildRea% (
    rd /s /q %buildRea%
)
mkdir %buildRea%

set srcApp="..\app"
set buildApp="..\buildApp"

if exist %buildApp% (
    rd /s /q %buildApp%
)
mkdir %buildApp%

cmake -S %srcRea% -DCMAKE_BUILD_TYPE=%buildType% -A x64 -B %buildRea% -DMS=%buildApp%\%buildType%\plugin
%msbuild% %buildRea%\ALL_BUILD.vcxproj /p:Configuration=%buildType%

cmake -S %srcApp% -DCMAKE_BUILD_TYPE=%buildType% -DCMAKE_CUSTOM_PROJECT=%packProject% -A x64 -B %buildApp%
%msbuild% %buildApp%\ALL_BUILD.vcxproj /p:Configuration=%buildType%

::https://stackoverflow.com/questions/7005951/batch-file-find-if-substring-is-in-string-not-in-a-file
for /f "delims=:" %%i in (%packProject%/.module) do (
    echo.%%i | findstr /C:"dll">nul && (
        if exist ..\build%%i (
            rd /s /q ..\build%%i
        )
        mkdir ..\build%%i

        echo %buildApp%\%buildType%\plugin
        cmake -S ..\%%i -DCMAKE_BUILD_TYPE=%buildType% -A x64 -B ..\build%%i -DMS=%buildApp%\%buildType%\plugin
        %msbuild% ..\build%%i\ALL_BUILD.vcxproj /p:Configuration=%buildType%
    )
)

set srcKey="..\key"
if exist %srcKey% (
    set buildKey="..\buildKey"

    if exist %buildKey% rd /s /q %buildKey%
    mkdir %buildKey%

    cmake -S %srcKey% -DCMAKE_BUILD_TYPE=%buildType% -A x64 -B %buildKey% -DMS=%buildApp%
    %msbuild% %buildKey%\ALL_BUILD.vcxproj /p:Configuration=%buildType%
    copy "..\key\key.bat" %buildApp%\%buildType%\key.bat
)


call recordVersion %buildApp%\%buildType%\.version %packProject%

xcopy %buildApp%\%buildType%\plugin\%buildType%\* %buildApp%\%buildType%\plugin /e /y
rd /s /q %buildApp%\%buildType%\plugin\%buildType%

mkdir %buildApp%\%buildType%\qtinstall
xcopy .\%packProject%\qtinstall\* %buildApp%\qtinstall\ /e /y

call .\%packProject%\custombuild %buildApp% %buildType%

if "%3"=="true" (
    type Nul >"..\buildApp\qtinstall\mypackages\content3\data\.internal"
)

cd pack
WINPACK.exe ../%packProject%/config_.json
cd ..

if exist %installer% ( 
    %installer% -c %buildApp%\qtinstall\config\config.xml -p %buildApp%\qtinstall\mypackages %packProject%V4.1.exe -v 
)

:: xcopy DeepInspectionV4.exe ..\frm-company /y
:: "C:/Program Files/7-Zip/7z.exe" a -tzip DeepInspectionBinary.zip %buildType%/*