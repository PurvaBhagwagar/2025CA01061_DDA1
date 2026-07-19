#!/bin/bash
# test.sh for ch04-stack (Question 1)
# Run from inside ch04-stack/ after `make`.

set -e

echo "=== [1] Build ==="
make clean
make
ls -l *.ko

echo
echo "=== [2] Expect FAILURE: insmod bitsfeed before bitscore ==="
if sudo insmod ./bitsfeed.ko 2>&1; then
	echo "UNEXPECTED: bitsfeed loaded without bitscore present"
	sudo rmmod bitsfeed || true
	exit 1
else
	echo "OK: bitsfeed refused to load (unresolved symbols) -- see dmesg below"
	dmesg | tail -n 5
fi

echo
echo "=== [3] Load bitscore, then bitsfeed with nsamples=8 ==="
sudo insmod ./bitscore.ko
sudo insmod ./bitsfeed.ko nsamples=8
dmesg | tail -n 10

echo
echo "=== [4] lsmod: bitscore Used-by should show bitsfeed ==="
lsmod | grep bits

echo
echo "=== [5] Expect REFUSAL: rmmod bitscore while bitsfeed is loaded ==="
if sudo rmmod bitscore 2>&1; then
	echo "UNEXPECTED: bitscore was removed while still in use"
	exit 1
else
	echo "OK: bitscore removal refused (module in use)"
fi

echo
echo "=== [6] Install + depmod, then auto-load via modprobe ==="
sudo make install
sudo modprobe -r bitsfeed || true
sudo modprobe -r bitscore || true
sudo modprobe bitsfeed
lsmod | grep bits

echo
echo "=== [7] Git evidence ==="
git log --oneline -s | head

echo
echo "=== cleanup ==="
sudo rmmod bitsfeed
sudo rmmod bitscore
echo "done"
