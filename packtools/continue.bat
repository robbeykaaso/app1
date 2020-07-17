cd ../build3
xcopy Release\* ..\qtinstall\mypackages\content\data\ /e /y
xcopy ..\packtools\* ..\qtinstall\mypackages\content2\data\ /e /y
D:/qt-installer/bin/binarycreator -c ../qtinstall/config/config.xml -p ../qtinstall/mypackages DeepInspectionInstaller.exe -v 