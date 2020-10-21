@echo off

set tar=%1

del %tar%
type nul>%tar%

set modules=(frm, dll, app, dll2)

for %%i in %modules% do (
    echo %%i: >>%tar%
    git --git-dir=../%%i/.git rev-parse head >>%tar%
)