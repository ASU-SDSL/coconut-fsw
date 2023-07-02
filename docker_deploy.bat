docker_build.bat

echo Copying firmware file to RPI filesystem...
copy .\docker_build\COCONUTFSW.uf2 E:\
if %errorlevel%==0 (
    echo Firmware Flashed Successfully!
) else (
    echo Copy failed, make sure you held BOOTSEL while connecting the RPI board!
)