set installer="D:\qt-installer\bin\binarycreator.exe"
set buildApp="..\buildApp"
set packProject=DeepInspection

set dir=%packProject%V4.1
set current_date=%date:~0,4%-%date:~5,2%-%date:~8,2%
set dir=%dir%-%current_date%
set dir=%dir%.exe
if exist %installer% ( 
    %installer% -c %buildApp%\qtinstall\config\config.xml -p %buildApp%\qtinstall\mypackages %dir% -v 
)