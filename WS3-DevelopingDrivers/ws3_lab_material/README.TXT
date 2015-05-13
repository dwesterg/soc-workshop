--------------------------------------------------------------------------------
INSTALL THE LAB MATERIALS

If you have not already installed the toolchain and kernel source tree required
to build the lab work, please do that first.  You should find a README.TXT file
at <SD CARD>/WS3-DevelopingDrivers which will describe how to get the toolchain
and kernel source tree installed onto your development host.

--------------------------------------------------------------------------------
SETUP DEVELOPMENT ENVIRONMENT

With the toolchain and kernel source tree installed make sure that you have
setup your environment to properly point to the toolchain and kernel source
directories.  The installation flow that was suggested above should have
produced a script called setup_env.src that you can source into your environment
to get the ARCH, CROSS_COMPILE, and OUT_DIR variables configured properly for
your environment.

You could do this by executing:

. <path-to>/setup_env.sh

--------------------------------------------------------------------------------
BUILD THE LAB WORK

With your environment properly setup, you should proceed into each of the four
subdirectories provided and view the README.TXT files located within each
directory.  The README.TXT file will explain how to go about building the kernel
module or test application within that directory.

./lab_module/README.TXT
./lab_module_test/README.TXT
./uio_module/README.TXT
./uio_module_test/README.TXT

--------------------------------------------------------------------------------
COPY THE LAB WORK TO YOUR TARGET

Copy the files that you've built over to the target for execution.  There are
many ways that you could do this, using the ssh 'scp' command is fairly
convenient.

scp \
	lab_module/lab_module.ko \
	uio_module/my_uio_pdrv_genirq.ko \
	lab_module_test/lab_module_test \
	uio_module_test/uio_module_test \
	root@<IP-ADDR>:/root

You'll need to enter the root password, which is 'password'.

If you run into issues with ssh keys, or if you'd prefer for your host system to
not use ssh keys while you do this copy, add these arguments to the 'scp'
command:

-o StrictHostKeyChecking=no
-o UserKnownHostsFile=/dev/null

Like this:

scp \
	-o StrictHostKeyChecking=no \
	-o UserKnownHostsFile=/dev/null \
	lab_module/lab_module.ko \
	uio_module/my_uio_pdrv_genirq.ko \
	lab_module_test/lab_module_test \
	uio_module_test/uio_module_test \
	root@<IP-ADDR>:/root

Another way to copy these files over to your target would be to copy them onto
the SD card that you use to boot the workshop design on your target.  To do this
mount the USB mass storage volume provided by the target to your host, and copy
the files to it.  Then unmount the USB mass storage volume.

Now on the target board you can mount the FAT partition by performing these
commands:

mkdir /mnt/fat
mount /dev/mmcblk0p1 /mnt/fat

After you mount the FAT partition you can copy the files from /mnt/fat to /root.

When you finish with the FAT partition you can unmount it like this:

umount /mnt/fat

--------------------------------------------------------------------------------
EXECUTE THE LAB WORK

On the target, run the module tests.

First insert the lab_module:
# insmod ./lab_module.ko

Then run the lab_module_test:
# ./lab_module_test

You should see the following output:
Success
End of test...

Then remove the lab_module:
# rmmod lab_module

And insert the my_uio_pdrv_genirq module:
# insmod my_uio_pdrv_genirq.ko

Then run the uio_module_test:
# ./uio_module_test

You should see the following output:
Success
End of test...

Then remove the lab_module:
# rmmod my_uio_pdrv_genirq

--------------------------------------------------------------------------------
VALIDATE YOUR RESULTS

Once you've proven out your modules and test programs on the target, validate
your results by running the lab validator program.

You run this validator program on the target:
# /examples/drivers/ws3_lab_validator

If the validator runs successfully you should see this output:

Validation Complete
Please forward these validation files to prove that you completed the lab work:
validation_archive.tar.gz
validation_archive.tar.gz.sign

--------------------------------------------------------------------------------
COPY YOUR VALIDATION OFF YOUR TARGET

Copy the two validation files from your target back to your development host.
This can be done a number of ways, but two methods described above to copy to
the target may be reversed to copy from the target.

Here is an example of how to do this with 'scp':

scp \
	-o StrictHostKeyChecking=no \
	-o UserKnownHostsFile=/dev/null \
	root@<IP-ADDR>:/root/validation_archive.tar.gz \
	root@<IP-ADDR>:/root/validation_archive.tar.gz.sign .

--------------------------------------------------------------------------------
SUBMIT YOUR VALIDATION

Email the two validation files to your Altera representative for authentication.

--------------------------------------------------------------------------------

