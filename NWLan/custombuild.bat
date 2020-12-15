set buildApp=%1
set buildType=%2

xcopy %buildApp%\%buildType%\* %buildApp%\qtinstall\mypackages\content\data\ /e /y