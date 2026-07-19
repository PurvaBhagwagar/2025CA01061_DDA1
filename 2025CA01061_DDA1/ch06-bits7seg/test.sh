#!/bin/bash
# test.sh for ch06-bits7seg (Question 2)
# Run from inside ch06-bits7seg/ after `make` and `sudo insmod ./bits7seg.ko`.

set -e

echo "=== [1] Device nodes ==="
ls -l /dev/bits7seg*

echo
echo "=== [2] Dynamic major ==="
grep bits7seg /proc/devices

echo
echo "=== [3] Write a distinct digit to each minor, then read it back ==="
for i in 0 1 2 3; do
	digit=$((i + 1))
	echo "-- /dev/bits7seg$i : write $digit --"
	echo -n "$digit" | sudo tee /dev/bits7seg$i > /dev/null
	echo "-- /dev/bits7seg$i : read --"
	sudo cat /dev/bits7seg$i
done

echo
echo "=== [4] Poke digit through sysfs on minor 2 ==="
echo 3 | sudo tee /sys/class/bits7seg/bits7seg2/digit > /dev/null
sudo cat /sys/class/bits7seg/bits7seg2/digit
sudo cat /dev/bits7seg2

echo
echo "=== [5] -EINVAL path: invalid byte on minor 0 ==="
if echo -n x | sudo tee /dev/bits7seg0 > /dev/null 2>&1; then
	echo "UNEXPECTED: write of 'x' succeeded"
	exit 1
else
	echo "OK: write correctly rejected with -EINVAL"
fi

echo
echo "=== all bits7seg tests passed ==="
