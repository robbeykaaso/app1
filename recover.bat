@echo off

for /f "tokens=1,2,3 delims=:" %%a in (.version) do (
    git clone https://github.com/robbeykaaso/%%b.git
    git --git-dir=./%%b/.git --work-tree=./%%b checkout %%c -b local
    ren %%b %%a
)