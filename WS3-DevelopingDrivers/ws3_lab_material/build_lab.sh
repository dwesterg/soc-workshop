#!/bin/sh

cd $(dirname "${0}")

# test environment
[ "${ARCH:?undefined, try sourcing setup_env.src?}" == "arm" ] || {
	echo ""
	echo "ERROR: ARCH environment variable defined to something other than 'arm'"
	echo ""
	exit 1
}

type -P ${CROSS_COMPILE:?undefined, try sourcing setup_env.src?}gcc 2>&1 > /dev/null || {
	echo ""
	echo "ERROR: CROSS_COMPILE environment variable not defined to expected tools"
	echo ""
	exit 1
}

[ -f ${OUT_DIR:?undefined, try sourcing setup_env.src?}/.config ] || {
	echo ""
	echo "ERROR: OUT_DIR environment variable not defined to expected kernel output tree"
	echo ""
	exit 1
}

# build lab_module directory
BUILD_DIR="lab_module"
[ -d "${BUILD_DIR}" ] || {
	echo ""
	echo "ERROR: could not find directory '${BUILD_DIR}'"
	echo ""
	exit 1
}

pushd "${BUILD_DIR}" > /dev/null

cat <<EOF > Kbuild
ccflags-y := "-Wno-date-time"
obj-m  := lab_module.o
EOF

cat <<EOF > Makefile
ifndef OUT_DIR
    \$(error OUT_DIR is undefined, bad environment, you point OUT_DIR to the linux kernel build output directory)
endif

KDIR ?= \$(OUT_DIR)

default:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD

clean:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD clean

help:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD help

modules:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules

modules_install:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules_install
EOF

[ -f "lab_module.c" ] && {
	sed -i \
		-e "s/\"FILE=\"/\"FILE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"DATE=\"/\"DATE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"TIME=\"/\"TIME=\*\*AUTO\*BUILD\*\*\"/" \
		"lab_module.c"
} || {
	echo ""
	echo "ERROR: cannot find file 'lab_module.c' in:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

make || {
	echo ""
	echo "ERROR: build failed in '${BUILD_DIR}' directory:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

sed -i -e "s/\*\*AUTO\*BUILD\*\*//" "lab_module.c"

popd > /dev/null

# build uio_module directory
BUILD_DIR="uio_module"
[ -d "${BUILD_DIR}" ] || {
	echo ""
	echo "ERROR: could not find directory '${BUILD_DIR}'"
	echo ""
	exit 1
}

pushd "${BUILD_DIR}" > /dev/null

UIO_FILE="${OUT_DIR}/drivers/uio/uio_pdrv_genirq.c"
[ -f "${UIO_FILE}" ] || {
	echo ""
	echo "ERROR: could not find file '${${UIO_FILE}}'"
	echo ""
	exit 1
}

MY_UIO_FILE="my_$(basename ${UIO_FILE})"
cp "${UIO_FILE}" "${MY_UIO_FILE}"

sed -i.bak -e '
/\#define DRIVER_NAME/ s/\"\(.*\)\"/\"my_\1\"/
/uio_of_genirq_match\[\] = / a\
	{.compatible = "demo,driver-1.0"},
' ${MY_UIO_FILE}

cat <<EOF > Kbuild
obj-m  := $(echo -n ${MY_UIO_FILE} | sed -e "s/\.c/\.o/")
EOF

cat <<EOF > Makefile
ifndef OUT_DIR
    \$(error OUT_DIR is undefined, bad environment, you point OUT_DIR to the linux kernel build output directory)
endif

KDIR ?= \$(OUT_DIR)

default:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD

clean:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD clean

help:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD help

modules:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules

modules_install:
	\$(MAKE) -C \$(KDIR) M=\$\$PWD modules_install
EOF

make || {
	echo ""
	echo "ERROR: build failed in '${BUILD_DIR}' directory:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

popd > /dev/null

# build lab_module_test directory
BUILD_DIR="lab_module_test"
[ -d "${BUILD_DIR}" ] || {
	echo ""
	echo "ERROR: could not find directory '${BUILD_DIR}'"
	echo ""
	exit 1
}

pushd "${BUILD_DIR}" > /dev/null

[ -f "lab_module_test.c" ] && {
	sed -i \
		-e "s/\"FILE=\"/\"FILE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"DATE=\"/\"DATE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"TIME=\"/\"TIME=\*\*AUTO\*BUILD\*\*\"/" \
		"lab_module_test.c"
} || {
	echo ""
	echo "ERROR: cannot find file 'lab_module_test.c' in:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

${CROSS_COMPILE:?}gcc \
        -march=armv7-a \
        -mfloat-abi=hard \
        -mfpu=vfp3 \
        -mthumb-interwork \
        -mthumb \
        -O2 \
        -g \
        -feliminate-unused-debug-types  \
        -std=gnu99 \
        -W \
        -Wall \
        -Werror \
        -Wc++-compat \
        -Wwrite-strings \
        -Wstrict-prototypes \
        -pedantic \
	-o lab_module_test \
	lab_module_test.c || {
	echo ""
	echo "ERROR: build failed in '${BUILD_DIR}' directory:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

sed -i -e "s/\*\*AUTO\*BUILD\*\*//" "lab_module_test.c"

popd > /dev/null

# build uio_module_test directory
BUILD_DIR="uio_module_test"
[ -d "${BUILD_DIR}" ] || {
	echo ""
	echo "ERROR: could not find directory '${BUILD_DIR}'"
	echo ""
	exit 1
}

pushd "${BUILD_DIR}" > /dev/null

[ -f "uio_module_test.c" ] && {
	sed -i \
		-e "s/\"FILE=\"/\"FILE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"DATE=\"/\"DATE=\*\*AUTO\*BUILD\*\*\"/" \
		-e "s/\"TIME=\"/\"TIME=\*\*AUTO\*BUILD\*\*\"/" \
		"uio_module_test.c"
} || {
	echo ""
	echo "ERROR: cannot find file 'uio_module_test.c' in:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

${CROSS_COMPILE:?}gcc \
        -march=armv7-a \
        -mfloat-abi=hard \
        -mfpu=vfp3 \
        -mthumb-interwork \
        -mthumb \
        -O2 \
        -g \
        -feliminate-unused-debug-types  \
        -std=gnu99 \
        -W \
        -Wall \
        -Werror \
        -Wc++-compat \
        -Wwrite-strings \
        -Wstrict-prototypes \
        -pedantic \
	-o uio_module_test \
	uio_module_test.c || {
	echo ""
	echo "ERROR: build failed in '${BUILD_DIR}' directory:"
	echo "'$(pwd)'"
	echo ""
	exit 1
}

sed -i -e "s/\*\*AUTO\*BUILD\*\*//" "uio_module_test.c"

popd > /dev/null

