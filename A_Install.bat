@echo off
set aa=MessageBus.exe -service
%~dp0%aa%
if errorlevel 0 net start DCTSMessageBus
timeout /nobreak /t 1
set aa=MessageBus.exe -unregserver
%~dp0%aa%
if errorlevel 0 goto INSTALL_DONE
@echo MessageBus 服务卸载失败
:INSTALL_DONE
@echo MessageBus 服务卸载成功 
pause