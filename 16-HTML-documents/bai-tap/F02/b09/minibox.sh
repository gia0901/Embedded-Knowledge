#!/bin/sh
# minibox.sh: container tối giản = namespace (unshare) + root riêng (chroot) + giới hạn (cgroup v1).
# Lời giải Lab 9.3. Chạy với quyền root, từ thư mục chứa rootfs/:
#   ./minibox.sh <tên> <cpu quota µs mỗi 100 ms | -1> <bộ nhớ MB> <số process tối đa> <lệnh...>
# Máy lab dùng cgroup v1. Trên cgroup v2 (Ubuntu 22.04+ mặc định) ba dòng echo tương ứng là:
#   echo "$QUOTA 100000" > cpu.max;  echo ${MEM}M > memory.max;  echo $PIDS > pids.max
set -e
NAME=$1; QUOTA=$2; MEM=$3; PIDS=$4; shift 4
CG=/sys/fs/cgroup
for c in cpu memory pids; do mkdir -p $CG/$c/$NAME; done
echo "$QUOTA" > $CG/cpu/$NAME/cpu.cfs_quota_us
echo $((MEM * 1024 * 1024)) > $CG/memory/$NAME/memory.limit_in_bytes
echo "$PIDS" > $CG/pids/$NAME/pids.max
for c in cpu memory pids; do echo $$ > $CG/$c/$NAME/cgroup.procs; done   # shell này và mọi con cháu
# namespace mới cho pid, mount, hostname, IPC, mạng; rồi đổi root sang rootfs/ và chạy lệnh làm PID 1
exec unshare --pid --fork --mount --uts --ipc --net chroot rootfs /bin/sh -c '
  mount -t proc proc /proc
  hostname "$0"
  ip link set lo up
  exec "$@"' "$NAME" "$@"
