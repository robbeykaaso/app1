@echo off

for /f "skip=1 delims=" %%i in (.version) do set "rea=%%i"&goto recoverrea
:recoverrea
git clone https://github.com/robbeykaaso/rea.git
git --git-dir=./rea/.git --work-tree=./rea checkout %rea% -b local

for /f "skip=3 delims=" %%i in (.version) do set "shp=%%i"&goto recovershp
:recovershp
git clone https://github.com/robbeykaaso/rea-shape.git
git --git-dir=./rea-shape/.git --work-tree=./rea-shape checkout %shp% -b local

for /f "skip=5 delims=" %%i in (.version) do set "app=%%i"&goto recoverapp
:recoverapp
git clone https://github.com/robbeykaaso/app1.git
git --git-dir=./app1/.git --work-tree=./app1 checkout %app% -b local

for /f "skip=7 delims=" %%i in (.version) do set "img=%%i"&goto recoverapp
:recoverapp
git clone https://github.com/robbeykaaso/rea-image.git
git --git-dir=./rea-image/.git --work-tree=./rea-image checkout %img% -b local