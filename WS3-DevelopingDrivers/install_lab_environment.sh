#!/bin/sh

DOWNLOAD_DIR="$(pwd)/download"
TOOLCHAIN_DIR="$(pwd)/toolchain"

pushd $(dirname $0) > /dev/null 2>&1

ABS_PATH_TO_SCRIPT_DIR="$(pwd)"

popd > /dev/null 2>&1

WS3_LAB_ENVIRONMENT_SRC="${ABS_PATH_TO_SCRIPT_DIR}/ws3_lab_environment.src"

[ -f "${WS3_LAB_ENVIRONMENT_SRC}" ] || {
	echo ""
	echo "ERROR: cannot locate file:"
	echo "'${WS3_LAB_ENVIRONMENT_SRC}'"
	echo ""
	exit 1	
}

. "${WS3_LAB_ENVIRONMENT_SRC}"

PATH_TO_LINUX_DEFCONFIG="${ABS_PATH_TO_SCRIPT_DIR}/${LINUX_DEFCONFIG:?}"

[ -f "${PATH_TO_LINUX_DEFCONFIG}" ] || {
	echo ""
	echo "ERROR: cannot locate file:"
	echo "'${PATH_TO_LINUX_DEFCONFIG}'"
	echo ""
	exit 1	
}

PATH_TO_PATCHES="${ABS_PATH_TO_SCRIPT_DIR}/../patches/${LINUX_BRANCH:?}"

[ -d "${PATH_TO_PATCHES}" ] || {
	echo ""
	echo "ERROR: cannot locate directory:"
	echo "'${PATH_TO_PATCHES}'"
	echo ""
	exit 1	
}

PATH_TO_LAB_MATERIAL="${ABS_PATH_TO_SCRIPT_DIR}/ws3_lab_material"

[ -d "${PATH_TO_LAB_MATERIAL}" ] || {
	echo ""
	echo "ERROR: cannot locate directory:"
	echo "'${PATH_TO_LAB_MATERIAL}'"
	echo ""
	exit 1	
}

mkdir -p "${DOWNLOAD_DIR}" || {
	echo ""
	echo "ERROR: creating directory:"
	echo "'${DOWNLOAD_DIR}'"
	echo ""
	exit 1	
}

[ -f "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE:?}" ] && {
	echo "Skipping toolchain archive download, already exists."
} || {
	echo "Downloading toolchain package."
	wget -O "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE}" "${TOOLCHAIN_SOURCE_PACKAGE:?}" || {
		echo ""
		echo "ERROR: wget toolchain package"
		echo ""
		exit 1	
	}
}

[ -f "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE_TAR:?}" ] && {
	echo "Skipping toolchain archive decompression, already exists."
} || {
	echo "Decompressing toolchain archive."
	cat "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE}" | xz -d > "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE_TAR}" || {
		echo ""
		echo "ERROR: decompressing toolchain package"
		echo ""
		exit 1	
	}
}

echo "Removing toolchain directory."
rm -Rf "${TOOLCHAIN_DIR}" || {
	echo ""
	echo "ERROR: removing toolchain directory"
	echo ""
	exit 1	
}

echo "Creating toolchain directory."
mkdir -p "${TOOLCHAIN_DIR}" || {
	echo ""
	echo "ERROR: creating toolchain dir:"
	echo "'${TOOLCHAIN_DIR}'"
	echo ""
	exit 1	
}

echo "Extracting toolchain archive."
tar xf "${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE_TAR}" --strip-components 1 -C "${TOOLCHAIN_DIR}" || {
	echo ""
	echo "ERROR: extracting archive:"
	echo "'${DOWNLOAD_DIR}/${TOOLCHAIN_SOURCE_TAR}'"
	echo ""
	exit 1	
}

export CROSS_COMPILE="$(find "${TOOLCHAIN_DIR}" -name "arm-linux-gnueabihf-gcc" | sed -e "s/arm-linux-gnueabihf-gcc/arm-linux-gnueabihf-/")"
export ARCH=arm

[ -f "${DOWNLOAD_DIR}/${LINUX_BRANCH}.tgz" ] && {
	echo "Skipping kernel archive download, already exists."
} || {
	echo "Downloading kernel package."
	wget -O "${DOWNLOAD_DIR}/${LINUX_BRANCH}.tgz" "${LNX_SOURCE_PACKAGE:?}" || {
		echo ""
		echo "ERROR: wget kernel package"
		echo ""
		exit 1	
	}
}

echo "Removing kernel directory."
rm -Rf "${LINUX_BRANCH}" || {
	echo ""
	echo "ERROR: kernel directory"
	echo ""
	exit 1	
}

echo "Creating kernel directory."
mkdir -p "${LINUX_BRANCH}" || {
	echo ""
	echo "ERROR: creating kernel dir:"
	echo "'${LINUX_BRANCH}'"
	echo ""
	exit 1	
}

echo "Extracting kernel archive."
tar xzf "${DOWNLOAD_DIR}/${LINUX_BRANCH}.tgz" --strip-components 1 -C "${LINUX_BRANCH}" || {
	echo ""
	echo "ERROR: extracting archive:"
	echo "'${DOWNLOAD_DIR}/${LINUX_BRANCH}.tgz'"
	echo ""
	exit 1	
}

echo "Copying linux defconfig."
cp "${PATH_TO_LINUX_DEFCONFIG}" "${LINUX_BRANCH}/arch/${ARCH}/configs/${LINUX_DEFCONFIG_TARGET:?}" || {
	echo ""
	echo "ERROR: copying linux defconfig"
	echo ""
	exit 1	
}

echo "Applying kernel patches."
for NEXT_PATCH in ${PATH_TO_PATCHES}/*.patch ; do
	echo "Applying patch: $(basename ${NEXT_PATCH})"
	patch -d ${LINUX_BRANCH} -p1 < "${NEXT_PATCH}"
done

echo "Make defconfig."
make -C "${LINUX_BRANCH}" "KBUILD_BUILD_VERSION=${KBUILD_BUILD_VERSION:?}" "${LINUX_DEFCONFIG_TARGET}" || {
	echo ""
	echo "ERROR: making defconfig"
	echo ""
	exit 1	
}

echo "Make modules."
make -C "${LINUX_BRANCH}" "KBUILD_BUILD_VERSION=${KBUILD_BUILD_VERSION}" "INSTALL_MOD_PATH=${LINUX_BRANCH}" modules || {
	echo ""
	echo "ERROR: making modules"
	echo ""
	exit 1	
}

echo "Copying lab materials."
cp -R "${PATH_TO_LAB_MATERIAL}" . || {
	echo ""
	echo "ERROR: copying lab materials"
	echo ""
	exit 1	
}

echo "Creating setup_env.src file."
echo "export ARCH=${ARCH}" > setup_env.src
echo "" >> setup_env.src
echo "export CROSS_COMPILE=${CROSS_COMPILE}" >> setup_env.src
echo "" >> setup_env.src
echo "export OUT_DIR=$(pwd)/${LINUX_BRANCH}" >> setup_env.src

echo "Installation complete."
echo "Please source the 'setup_env.src' file into your environment."
echo "Lab materials have been copied here: '$(basename ${PATH_TO_LAB_MATERIAL})'"
exit 0

