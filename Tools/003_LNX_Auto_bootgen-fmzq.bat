@echo off
mode con cols=180 lines=45 
title=FMSH Bootgen BAT V1.0
echo. 
echo ****************************************************************************
echo *                                                                          *
echo *                                                                          *
echo *"     $$$$$$$$\      $$\      $$\       $$$$$$\       $$\   $$\          "*
echo *"     $$  _____|     $$$\    $$$ |     $$  __$$\      $$ |  $$ |         "*
echo *"     $$ |           $$$$\  $$$$ |     $$ /  \__|     $$ |  $$ |         "*
echo *"     $$$$$\         $$\$$\$$ $$ |     \$$$$$$\       $$$$$$$$ |         "*
echo *"     $$  __|        $$ \$$$  $$ |      \____$$\      $$  __$$ |         "*
echo *"     $$ |           $$ |\$  /$$ |     $$\   $$ |     $$ |  $$ |         "*
echo *"     $$ |           $$ | \_/ $$ |     \$$$$$$  |     $$ |  $$ |         "*
echo *"     \__|           \__|     \__|      \______/      \__|  \__|         "*
echo *                                                                          *
echo *                    FMSH Bootgen BAT V1.0 for Windows                     *                          
echo *                         by mengfanqiang                                  *                    
echo ****************************************************************************
echo   This BAT help you to generate BOOT.bin file under Windows  
echo   command line without opening Procise "create boot image" window.  
echo.
echo   Usage: 
echo     Double click this BAT file.  enjoy it! 
echo.



@REM set PROCISE_DIR=D:/software/Vulture/Fudan Micro/FlashloaderBootgen
set PROCISE_DIR=%VULTURE_PATH%
set TCL_LIBRARY=%PROCISE_DIR%/tcl8.4
set PATH=%PROCISE_DIR%/dll;%PROCISE_DIR%/bin;%PATH%
set ICTIME_HOME=%PROCISE_DIR%
set APP_DIR=%PROCISE_DIR%
set FMSH_DB=%PROCISE_DIR%/db


setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "IMAGES_DIR=%SCRIPT_DIR%..\Images"
cd /d "%IMAGES_DIR%"


echo STEP2: generate outpu.bif file
echo //arch = fmzq; split = false; format = BIN; device = fmzq                      > output.bif
echo the_ROM_image:                                                                 >> output.bif
echo {                                                                              >> output.bif
echo 	[bootloader, destination_device = ps]%cd%\fsbl.out                      	>> output.bif
echo 	[destination_cpu = apu_0, destination_device = ps]%cd%\CA53_BOOT.out       	>> output.bif	
echo 	[fsbl_config]apu_x64       	                                                >> output.bif	
echo }                                                                              >> output.bif



echo STEP3: generate tcl file
echo create_boot_image output.bif BOOT.bin > .auto_bootgen.tcl

echo STEP4: generate BOOT.bin file
psoc_program.exe .auto_bootgen.tcl

if %errorlevel%==0 ( 
	echo "BOOT.bin is successfully generated"
	echo.
	echo bif Files Information :
	type output.bif
	echo.
	echo BOOT.bin location is :
	echo %cd%\BOOT.bin	
)else (
	echo "Failed to generate BOOT.bin!"
	echo "Please check the output.bif file"
)

REM del .auto_bootgen.tcl
echo .
@REM pause

