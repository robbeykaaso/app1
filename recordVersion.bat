@echo off

set tar=%1
set packProject=%2

if exist %tar% (
    del %tar%
)
type nul>%tar%

::https://www.cnblogs.com/HeviLUO/p/12349169.html
for /f "tokens=1,2 delims=:" %%i in (%packProject%/.module) do (
    >>%tar% set /p=%%i:%%j:<nul
    git --git-dir=../%%i/.git rev-parse head >>%tar%
)