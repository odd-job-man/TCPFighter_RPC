@echo off
setlocal enabledelayedexpansion

echo Starting Stub.exe...
start "" "%~dp0Stub.exe"

echo Starting Proxy.exe...
start "" "%~dp0Proxy.exe"
pause
