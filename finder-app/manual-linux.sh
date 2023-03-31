#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
TOOLCHAIN_BIN=/home/shaswath/work/arm-cross-compiler/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    # Step-1 Target Clean
    echo "make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper"
    make ARCH=${ARCH} \
         CROSS_COMPILE=${CROSS_COMPILE} \
         mrproper
    if [ $? -eq 0 ]; then
        echo "Clean build with flag mrproper"
    else
        echo "failed: Target Clean with flag mrproper"
        exit 1
    fi

    # Step-2 Target defconfig (Configure for “virt” arm dev board to simulate in QEMU)
    echo "make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig"
    make ARCH=${ARCH} \
         CROSS_COMPILE=${CROSS_COMPILE} \
         defconfig
    if [ $? -eq 0 ]; then
        echo "Configure for “virt” arm dev board to simulate in QEMU"
    else
        echo "failed: Target defconfig"
        exit 1
    fi

    # Step-3 Target vmlinux (Build a kernel image for booting with QEMU)
    echo "make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} vmlinux"
    make -j4 ARCH=${ARCH} \
         CROSS_COMPILE=${CROSS_COMPILE} \
         all
    if [ $? -eq 0 ]; then
        echo "Build kernel image for booting with QEMU"
    else
        echo "failed: Target vmlinux"
        exit 1
    fi

    # Step-4 Target modules (Build any kernel modules)
    make ARCH=${ARCH} \
         CROSS_COMPILE=${CROSS_COMPILE} \
         modules
    if [ $? -eq 0 ]; then
        echo "Build any kernel modules"
    else
        echo "failed: Target modules"
        exit 1
    fi

    # Step-5 Target devicetree (Build the devicetree)
    make ARCH=${ARCH} \
         CROSS_COMPILE=${CROSS_COMPILE} \
         dtbs
    if [ $? -eq 0 ]; then
        echo "Build the devicetree"
    else
        echo "failed: Target devicetree"
        exit 1
    fi
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
if [ $? -eq 0 ]; then
    echo "Add ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image to ${OUTDIR} success!"
else
    echo "failed: To copy image to ${OUTDIR}"
    exit 1
fi

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
if [ ! -d "$OUTDIR/rootfs" ]; then
    mkdir -p $OUTDIR/rootfs
fi

if [ -d "$OUTDIR/rootfs" ]; then
    cd "$OUTDIR/rootfs"
    # Create filesystem structure only if the dir rootfs does not exist.
	mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
    mkdir -p usr/bin usr/lib usr/sbin
    mkdir -p var/log
    mkdir -p home/conf
else
    echo "${OUTDIR}/rootfs: does not exist to create filesystem structure"
    exit 1;
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
else
    cd busybox
fi

# TODO: Make and install busybox
# The BusyBox build process is similar to the Linux kernel build:
make defconfig 

make ARCH=${ARCH} \
     CROSS_COMPILE=${CROSS_COMPILE}

make CONFIG_PREFIX=${OUTDIR}/rootfs \
     ARCH=${ARCH} \
     CROSS_COMPILE=${CROSS_COMPILE} \
     install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
cp $TOOLCHAIN_BIN/lib/* ${OUTDIR}/rootfs/lib/
if [ $? -eq 0 ]; then
    echo "Add library dependencies to rootfs success!"
else
    echo "failed: To Add library dependencies to rootfs ${OUTDIR}/rootfs/lib/"
    exit 1
fi

cp $TOOLCHAIN_BIN/lib64/* ${OUTDIR}/rootfs/lib64/
if [ $? -eq 0 ]; then
    echo "Add library dependencies to rootfs success!"
else
    echo "failed: To Add library dependencies to rootfs ${OUTDIR}/rootfs/lib/"
    exit 1
fi

# TODO: Make device nodes
sudo mknod ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
cd ~/work/assignment-1-shaswathts/finder-app/
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
pwd
cp ~/work/assignment-1-shaswathts/finder-app/conf/* ${OUTDIR}/rootfs/home/conf
cp ~/work/assignment-1-shaswathts/finder-app/finder.sh ${OUTDIR}/rootfs/home/
cp ~/work/assignment-1-shaswathts/finder-app/finder-test.sh ${OUTDIR}/rootfs/home/
cp ~/work/assignment-1-shaswathts/finder-app/writer ${OUTDIR}/rootfs/home/ 
cp ~/work/assignment-1-shaswathts/finder-app/autorun-qemu.sh ${OUTDIR}/rootfs/home/
cp ~/work/assignment-1-shaswathts/finder-app/start-qemu-app.sh ${OUTDIR}/rootfs/home/

# TODO: Chown the root directory
cd ${OUTDIR}/rootfs/
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs/
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
