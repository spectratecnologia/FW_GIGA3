1- reboot_and_update_firmware.bat sends a custom message(##BOOTLOADER##) rebooting uC to bootloader, wait some seconds and program using USB DFU.
2- While running, uC controller has a virtual COM, install stm32_vcp.zip driver.
3- While in bootloader mode, uC controller has a DFU device, install usb driver for DFU with zadig.exe ( Select desired device and as driver, WinUSB).
4-Make sure virtual COM is named COM3.


dfu-util can be downloaded from: http://dfu-util.gnumonks.org/

The patch to reboot uC after programming with DFU can be found at https://gitorious.org/dfu-util/dfu-util/commit/cd6489aab8777cba0567076c2e814988ccc708d0?format=patch


Jonas