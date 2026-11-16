for q in -1 100000 50000; do
  rm -f rootfs/app/state.json*; for c in cpu memory pids; do rmdir /sys/fs/cgroup/$c/box 2>/dev/null; done
  ./minibox.sh box $q 64 32 /app/telemd 9000 2 /app/state.json > box.out 2>/dev/null &
  sleep 0.5; T=$(pgrep -x telemd)
  nsenter -t $T -n rootfs/app/telesend 1000000 4 9000; sleep 1
  echo "quota=$q: $(grep -o 'pid=[0-9]*' box.out), $(grep nr_throttled /sys/fs/cgroup/cpu/box/cpu.stat), $(grep throttled_time /sys/fs/cgroup/cpu/box/cpu.stat)"
  kill -TERM $T; wait; tail -1 box.out
done
