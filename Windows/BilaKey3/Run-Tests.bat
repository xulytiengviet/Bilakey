@echo off
cd /d "%~dp0"
where node >nul 2>nul
if errorlevel 1 (echo Node.js is required & exit /b 1)
node src\smoke-test.js
pause
