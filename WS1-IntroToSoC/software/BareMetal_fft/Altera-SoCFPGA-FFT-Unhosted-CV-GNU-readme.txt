Introduction

This HardwareLibs project is meant as an example for using UART for printf output, instead of semihosting.

This project can be run from Debugger or booted from SD/MMC.

====

Target Boards:
- Altera Cyclone V SoC Development Board rev D

====

Notes:
- This example was developed with and tested against SoC EDS 14.0b200.
- The example relies on the Preloader setting up the UART appropriately (baud rate, parity, stop bits).
- A special Preloader was created for this example. The following settings were used:
    - BOOT_FROM_SDMMC: checked
    - FAT_SUPPORT: checked
    - FAT_BOOT_PARTITION 1
    - FAT_LOAD_PAYLOAD_NAME: hello-mkimage.bin
    - WATCHDOG_ENABLE: unchecked
    - SDRAM_SCRUBBING: checked
    - SDRAM_SCRUB_REMAINING_REGION: checked
        
  This Preloader was already compiled and it is provided with the project.

=====

Source Files

The following are descriptions of the relevant files and folders contained in this
project:

hello.c

  Contains main function, which has a single printf statement.

io.c

  Contains the low-level I/O functions required by the compiler. They implement a very basic output over UART.
  
cycloneV-dk-ram-modified.ld

  Linker script. It is based on the deafault linker script shipped with SoC EDS, with the change that 64 bytes are
  freed at the beginning of the SDRAM window. This is to make room for the mkimage signature to be placed in SDRAM by
  the Preloader
  
preloader_scrubbing/uboot-socfpga/spl/u-boot-spl

  Preloader ELF file, used when running the application from Debugger

preloader_scrubbing/preloader-mkpimage.bin

   Preloader image file, to be written to SD card when booting from SD.
   
  
=====

Building Example

Before running the example, the target executable first needs to be built.

1. In DS-5, build the application:
  1a. Switch to the C/C++ Perspective if not already in that perspective by
      selecting the menu: Window >> Open Perspective >> C/C++.
  1b. In the "Project Explorer" panel, right-mouse-click 
      "Altera-SoCFPGA-HardwareLib-Unhosted-CV-GNU" and select "Build Project".

The Console panel (bottom of the UI) should detail the progress of the build
and report any warnings or errors.

=====

System Setup

1. Connect the USB to serial bridge to the host computer.
2. Connect the USB-BlasterII to the host computer.
3. Install the USB to serial bridge driver on the host computer if that driver
   is not already present. Consult documentation for the DevKit for
   instructions on installing the USB to serial bridge driver.
4. Install the USB-BlasterII driver on the host computer if that driver is not
   already present. Consult documentation for QuartusII for instructions on
   installing the USB-BlasterII driver.
5. In DS-5, configure the launch configuration.
  5a. Select the menu: Run >> Debug Configurations...
  5b. In the options on the left, expand "DS-5 Debugger" and select
      "Altera-SoCFPGA-HardwareLib-Unhosted-CV-GNU".
  5c. In the "Connections" section near the bottom, click Browse.
  5d. Select the appropriate USB-BlasterII to use. Multiple items will be
      presented if there is more than one USB-BlasterII connection attached to
      the host computer.
  5e. Click "Apply" then "OK" to apply the USB-BlasterII selection.
  5f. Click "Close" to close the Debug Configuration. Otherwise click "Debug"
      run the example in the debugger.

=====

Running the Example from Debugger

After building the example and setting up the host computer system, the example
can be run by following these steps.

1. Start serial terminal on host computer, use 115,200-8-N-1 and connect to the board.
2. In DS-5, launch the debug configuration.
  2a. Switch to the Debug Perspective if not already in that perspective by
      selecting the menu: Window >> Open Perspective >> DS-5 Debug.
  2b. In the "Debug Control" panel, right-mouse-click
      "Altera-SoCFPGA-HardwareLib-Unhosted-CV-GNU" and select
      "Connect to Target".

Connecting to the target takes a moment to load the preloader, run the
preloader, load the executable, and run executable. After the debug connection
is established, the debugger will pause the execution at the main() function.
Users can then set additional break points, step into, step out of, or step one
line using the DS-5 debugger. Consult documentation for DS-5 for more
information on debugging operations.

=====

Booting the Example from SD/MMC

1. Write bootable SD image to an SD card. You can for example use the file 
<SoC EDS Installation Folder>\embedded\embeddedsw\socfpga\prebuilt_images\sd_card_linux_boot_image.tar.gz.
2. Update the preloader on the SD card using the tool provided with SoC EDS. Use the Preloader file 
preloader_scrubbing/preloader-mkpimage.bin. The command is 
alt-boot-disk-util.exe -p preloader_scrubbing/preloader-mkpimage.bin -a write -d X
Replace 'X' with the name of your SD card drive in Windows.
The command is to be run from the Embedded Command Shell, with the current directory being the
project directory.
3. Write the application image file hello-mkimage.bin to the FAT partition on the SD card 
4. Configure BSEL pins to boot from SD/MMC
5. Connect the USB to serial bridge to the host computer.
6. Start serial terminal on host computer, use 115,200-8-N-1 and connect to the board.
7. Power up (or reset) the board

=====

Sample output, including Preloader output, when ran from Debugger:

U-Boot SPL 2013.01.01 (Jul 07 2014 - 11:47:21)
BOARD : Altera SOCFPGA Cyclone V Board
CLOCK: EOSC1 clock 25000 KHz
CLOCK: EOSC2 clock 25000 KHz
CLOCK: F2S_SDR_REF clock 0 KHz
CLOCK: F2S_PER_REF clock 0 KHz
CLOCK: MPU clock 925 MHz
CLOCK: DDR clock 400 MHz
CLOCK: UART clock 100000 KHz
CLOCK: MMC clock 50000 KHz
CLOCK: QSPI clock 370000 KHz
SDRAM: Initializing MMR registers
SDRAM: Calibrating PHY
SEQ.C: Preparing to start memory calibration
SEQ.C: CALIBRATION PASSED
SDRAM: 1024 MiB
SDRAM: Scrubbing 0x01000000 - 0x02000000
SDRAM: Scrubbing success with 25 ms
SDRAM: Scrubbing 0x00000000 - 0x01000000
SDRAM: Scrubbing 0x02000000 - 0x40000000
SDRAM: ECC Enabled
Hello World!

Sample output, including Preloader output, when booted from SD card:

U-Boot SPL 2013.01.01 (Jul 07 2014 - 11:47:21)
BOARD : Altera SOCFPGA Cyclone V Board
CLOCK: EOSC1 clock 25000 KHz
CLOCK: EOSC2 clock 25000 KHz
CLOCK: F2S_SDR_REF clock 0 KHz
CLOCK: F2S_PER_REF clock 0 KHz
CLOCK: MPU clock 925 MHz
CLOCK: DDR clock 400 MHz
CLOCK: UART clock 100000 KHz
CLOCK: MMC clock 50000 KHz
CLOCK: QSPI clock 370000 KHz
SDRAM: Initializing MMR registers
SDRAM: Calibrating PHY
SEQ.C: Preparing to start memory calibration
SEQ.C: CALIBRATION PASSED
SDRAM: 1024 MiB
SDRAM: Scrubbing 0x01000000 - 0x02000000
SDRAM: Scrubbing success with 24 ms
SDRAM: Scrubbing 0x00000000 - 0x01000000
SDRAM: Scrubbing 0x02000000 - 0x40000000
SDRAM: ECC Enabled
ALTERA DWMMC: 0
reading hello-mkimage.bin
reading hello-mkimage.bin
SDRAM: Scrubbing success with consuming additional 1518 ms
Hello World!
