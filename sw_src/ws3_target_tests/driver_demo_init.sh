#!/bin/sh

APP_LIST="
demo_devmem
demo_map_test
demo_uio_test
ioctl_test
"

MOD_LIST="
demo_module_01.ko
demo_module_01t.ko
demo_module_02.ko
demo_module_03.ko
demo_module_04.ko
demo_module_05.ko
demo_module_05t.ko
demo_module_06.ko
demo_module_07.ko
demo_module_08.ko
demo_module_09.ko
demo_module_10.ko
demo_module_11.ko
demo_module_11t.ko
"
#
# create /root/driver_demo_material
#
[ -d "/root/driver_demo_material" ] || {
	mkdir "/root/driver_demo_material" || {
		echo ""
		echo "ERROR: could not create directory '/root/driver_demo_material'"
		echo ""
		exit 1
	}
}

cd "/root/driver_demo_material" || {
	echo ""
	echo "ERROR: could not change to directory '/root/driver_demo_material'"
	echo ""
	exit 1
}

#
# copy files into /root/driver_demo_material
#
for NEXT_APP in ${APP_LIST} ; do
	find /examples/drivers -name "${NEXT_APP}" -exec cp \{\} . \;
done

for NEXT_MOD in ${MOD_LIST} ; do
	find /lib/modules -name "${NEXT_MOD}" -exec cp \{\} . \;
done

#
# verify files exist in /root/driver_demo_material
#
for NEXT_APP in ${APP_LIST} ; do
	[ -f "${NEXT_APP}" ] || {
		echo ""
		echo "ERROR: application '${NEXT_APP}' did not copy into '/root/driver_demo_material'"
		echo ""
		exit 1
	}
done

for NEXT_MOD in ${MOD_LIST} ; do
	[ -f "${NEXT_MOD}" ] || {
		echo ""
		echo "ERROR: module '${NEXT_MOD}' did not copy into '/root/driver_demo_material'"
		echo ""
		exit 1
	}
done

echo "Driver demo material setup is complete."
echo "Directory '/root/driver_demo_material' has been initialized for demonstration."

