set buildApp=%1
set buildType=%2

xcopy %buildApp%\%buildType%\* %buildApp%\qtinstall\mypackages\content\data\ /e /y
::xcopy .\pack\* %buildApp%\qtinstall\mypackages\content2\data\ /e /y
xcopy %buildApp%\%buildType%\* %buildApp%\qtinstall\mypackages\content3\data\ /e /y
xcopy ..\frm\install\* ..\frm-company\install /e /y
xcopy ..\frm\include\* ..\frm-company\include /e /y
xcopy ..\dll2\* ..\frm-company\plugin-image\ /e /y
xcopy ..\dll3\* ..\frm-company\plugin-test\ /e /y
xcopy ..\dll4\* ..\frm-company\plugin-gui\ /e /y
xcopy "G:\Qt\Qt5.13.2\5.13.2\msvc2017_64\resources\*" %buildApp%\qtinstall\mypackages\content3\data\ /e /y
copy "G:\Qt\Qt5.13.2\5.13.2\msvc2017_64\bin\QtWebEngineProcess.exe" %buildApp%\qtinstall\mypackages\content3\data\QtWebEngineProcess.exe