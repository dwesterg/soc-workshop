
********************************************************************
Welcome to the SoC Workshop

Assigning a Static IP address
----------------------------
To assign a static IP, use the following command

ifconfig eth0 192.168.10.2 

configure your PC/Laptop with an IP address in the same subnet, IE:
192.168.10.1

You will them be able to ssh to the board from your host.

Getting a DHCP IP address
-------------------------
The board should try to get a dhcp address.  To find the address type

ifconfig eth0

if a dhcp address is not obtained try

ifdown eth0
ifup eth0

Mounting the FAT partition on the SDCard
-----------------------------------------
To mount the FAT partition on the SDCard, do the following

mkdir /mnt/fat
mount /dev/mmcblk0p1 /mnt/fat

Make sure to unmount it with

umount /mnt/fat

when you are done

Board Webpage
-------------
The board does have a webserver running, just open your browser and point to the boards IP address.


Examples & Validator programs
-----------------------------
All examples and the validator program can be found in /examples.

********************************************************************
