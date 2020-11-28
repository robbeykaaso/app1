@echo off

cd /d %~dp0
%1 start "" mshta vbscript:createobject("shell.application").shellexecute("""%~0""","::",,"runas",1)(window.close)&exit

netsh advfirewall firewall add rule name="MinIO" dir=in action=allow program="%cd%\minio.exe" enable=yes
netsh advfirewall firewall add rule name="MinIOSV" dir=in action=allow program="%cd%\minio-service.exe" enable=yes