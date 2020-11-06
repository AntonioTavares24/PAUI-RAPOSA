@echo off
echo Press any key to send configuration requests to guest machine...
pause > nul
echo ============================
echo Configuring script folder...
echo ============================
call callurl http://localhost:50001/scripts/home/sikulixrunserver
echo.
echo =========================
echo Starting script runner...
echo =========================
call callurl http://localhost:50001/startp
call callurl http://localhost:50001/run/buttonOperation
echo SikuliX RunServer is ready to receive requests.
echo.
pause