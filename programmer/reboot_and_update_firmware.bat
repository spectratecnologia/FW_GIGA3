E:
cd E:\Work\Proj_New\MULTIPLEX3\GIGA3\SW.C\Firmware\programmer
echo "Rebooting.."
SerialSend COM5 ##BOOTLOADER_GIGA3##
echo "Waiting 2s for device reboot.."
ping 1.1.1.1 -n 1 -w 2000 > nul
echo "Updating firmware.."
dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000:leave -D ..\GIGA3\Debug\bin\GIGA3.bin
