cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -A x64 -DCMAKE_PREFIX_PATH=G:/QT/5.12.2/msvc2015_64;D:\awss3c++\aws-sdk-cpp\buildtest2\install\lib\cmake;D:\awss3c++\aws-sdk-cpp\buildtest2\install
msbuild ALL_BUILD.vcxproj /p:Configuration=Release
cd ../packtools
WIN_QUICKPACKAGE.exe
cd ../build
mkdir "Release/minIO"
mkdir "Release/plugin"
xcopy D:\build-deepinspection-Desktop_Qt_5_12_2_MSVC2015_64bit-Release\minIO\* Release\minIO /e /y
xcopy D:\build-deepinspection-Desktop_Qt_5_12_2_MSVC2015_64bit-Release\plugin\* Release\plugin /e /y
xcopy Release\* ..\qtinstall\mypackages\content\data\ /e /y
xcopy ..\packtools\* ..\qtinstall\mypackages\content2\data\ /e /y
D:/qt-installer/bin/binarycreator -c ../qtinstall/config/config.xml -p ../qtinstall/mypackages DeepInspectionInstaller.exe -v 
"C:/Program Files/7-Zip/7z.exe" a -tzip DeepInspectionBinary.zip Release/*