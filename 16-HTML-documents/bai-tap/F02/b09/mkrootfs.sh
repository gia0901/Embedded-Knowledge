#!/bin/sh
# mkrootfs.sh: dựng root file system tối thiểu cho minibox: busybox tĩnh + telemd, telesend build tĩnh.
# Cần: sudo apt install busybox-static. Chạy trong ~/oslab/f02/b09.
set -e
mkdir -p rootfs/bin rootfs/proc rootfs/sys rootfs/tmp rootfs/etc rootfs/app rootfs/dev
cp /usr/bin/busybox rootfs/bin/
for c in sh ps hostname ip ls cat sleep top mount id echo true; do ln -sf busybox rootfs/bin/$c; done
[ -e rootfs/dev/null ] || mknod -m 666 rootfs/dev/null c 1 3     # sh cần /dev/null cho job chạy nền
# build tĩnh: không cần chép libstdc++, libc vào rootfs
g++ -O2 -std=c++20 -pthread -static -o rootfs/app/telemd ../b08/telemd.cpp
g++ -O2 -std=c++20 -static -o rootfs/app/telesend ../telesend.cpp
du -sh rootfs
