# Step-by-Step Guide — Assignment 1 (Purva Bhagwagar, 2025CA01061)

This assumes a fresh Ubuntu 22.04 VM (or your Raspberry Pi from the lab
setup) with nothing installed yet. Work through the sections in order.
Every command block is meant to be typed exactly as shown. "📸" marks a
point where you should take a screenshot or copy the terminal output into
your evidence folder.

---

## 0. Before you start

Open a terminal (`Ctrl+Alt+T`) and confirm your kernel version — the
assignment targets Linux 6.x:

```bash
uname -r
lsb_release -a
```

📸 **Screenshot 0**: this output. This is your proof of the environment
you built and tested on — put it as the first thing in `evidence/`.

If `uname -r` shows a 5.x kernel, don't worry, most of this still works,
but note it in your README; if your instructor requires 6.x specifically,
update Ubuntu (`sudo apt update && sudo apt full-upgrade -y && sudo
reboot`) or use the Raspberry Pi image from the lab.

---

## 1. Install the toolchain

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) git make
```

**Expected output:** apt should end with `Setting up linux-headers-...`
and no errors. Verify the headers actually landed where kbuild expects
them:

```bash
ls /lib/modules/$(uname -r)/build
```

You should see files like `Makefile`, `Kconfig`, `include/`, etc. If this
directory doesn't exist, module builds will fail later with "No rule to
make target" — stop and fix this first (usually means the headers
package didn't match your running kernel; check `apt list
--installed | grep linux-headers` against `uname -r`).

---

## 2. Get checkpatch.pl

The assignment wants `scripts/checkpatch.pl` run against every `.c` file.
The easiest way to get a working copy without downloading the entire
kernel source tree is to pull just the `scripts/` directory:

```bash
mkdir -p ~/tools
cd ~/tools
git clone --depth 1 https://github.com/torvalds/linux.git kernel-scripts \
  -n --filter=blob:none
cd kernel-scripts
git sparse-checkout set scripts
git checkout
```

If your network can't do a sparse checkout (or `--filter=blob:none` isn't
supported), the simple fallback is a shallow full clone of just what you
need:

```bash
cd ~/tools
git clone --depth 1 https://github.com/torvalds/linux.git
```

Either way, confirm it runs:

```bash
perl ~/tools/kernel-scripts/scripts/checkpatch.pl --version 2>/dev/null || \
perl ~/tools/linux/scripts/checkpatch.pl --version
```

Note the path you got working — you'll reuse it in Sections 5 and 8.
Add a shortcut so the rest of this guide is copy-pasteable:

```bash
echo 'export CHECKPATCH=~/tools/kernel-scripts/scripts/checkpatch.pl' >> ~/.bashrc
source ~/.bashrc
```

(adjust the path if you used the fallback clone).

---

## 3. Set up your git repository and course layout

```bash
mkdir -p ~/bits-ddrv-2025CA01061
cd ~/bits-ddrv-2025CA01061
git init
git config user.name "Purva Bhagwagar"
git config user.email "you@example.com"   # use your actual BITS email
mkdir -p ch04-stack ch06-bits7seg evidence
```

Now copy in the source files I've prepared for you (from the
`2025CA01061_DDA1` folder you downloaded) into the matching directories:

```bash
cp /path/to/2025CA01061_DDA1/ch04-stack/*   ~/bits-ddrv-2025CA01061/ch04-stack/
cp /path/to/2025CA01061_DDA1/ch06-bits7seg/* ~/bits-ddrv-2025CA01061/ch06-bits7seg/
cp /path/to/2025CA01061_DDA1/README.md      ~/bits-ddrv-2025CA01061/
chmod +x ~/bits-ddrv-2025CA01061/ch04-stack/test.sh
chmod +x ~/bits-ddrv-2025CA01061/ch06-bits7seg/test.sh
```

Make your first commit (use `-s` every time, as the assignment
requires Signed-off-by lines):

```bash
cd ~/bits-ddrv-2025CA01061
git add README.md
git commit -s -m "docs: add assignment README and repo layout"
```

---

## 4. Question 1 — bitscore.ko + bitsfeed.ko

### 4.1 Understand what you're building, then commit the skeleton

```bash
cd ~/bits-ddrv-2025CA01061/ch04-stack
git add bitscore.h bitscore_internal.h
git commit -s -m "ch04: add shared bitscore headers"
git add core.c stats.c
git commit -s -m "ch04: implement bitscore sample store and exported API"
git add bitsfeed.c
git commit -s -m "ch04: implement bitsfeed dependent module"
git add Makefile test.sh
git commit -s -m "ch04: add out-of-tree Makefile and test script"
```

That's your four meaningful commits for Question 1.

### 4.2 Build

```bash
make clean
make
ls *.ko
```

**Expected output:** `bitscore.ko  bitsfeed.ko` and no compiler warnings
mentioning your files (some harmless kernel-wide warnings about the
toolchain version are normal and not something you caused).

📸 **Screenshot 1**: the full `make` output plus `ls *.ko`.

### 4.3 Prove the dependency: load order matters

```bash
sudo insmod ./bitsfeed.ko
```

**Expected output:** `insmod: ERROR: could not insert module
./bitsfeed.ko: Unknown symbol in module` (exact wording varies slightly
by kernel version — the key part is "unknown symbol").

```bash
dmesg | tail
```

**Expected output:** lines like `bitsfeed: Unknown symbol
bitscore_add_sample (err -2)`.

📸 **Screenshot 2**: both the `insmod` error and this `dmesg` output.

### 4.4 Load in the correct order

```bash
sudo insmod ./bitscore.ko
sudo insmod ./bitsfeed.ko nsamples=8
dmesg | tail
```

**Expected output:** something like:

```
bitscore: loaded, sample store ready (capacity=64)
bitsfeed: fed 8 samples, total count=8 avg=<some number 0-99>
```

📸 **Screenshot 3**: this dmesg output — this is your "prefixed count +
average" evidence.

### 4.5 Show the refcount relationship

```bash
lsmod | grep bits
```

**Expected output:** a line for `bitscore` whose last column (Used by)
shows `1 bitsfeed`, and a line for `bitsfeed` showing `0`.

📸 **Screenshot 4**.

```bash
sudo rmmod bitscore
```

**Expected output:** `rmmod: ERROR: Module bitscore is in use by:
bitsfeed`.

📸 **Screenshot 5**.

### 4.6 Install + depmod, then auto-load via modprobe

```bash
sudo make install
sudo depmod -a
sudo modprobe -r bitsfeed
sudo modprobe -r bitscore
sudo modprobe bitsfeed
lsmod | grep bits
```

**Expected output:** both `bitscore` and `bitsfeed` show up in `lsmod`
even though you only asked to load `bitsfeed` — this proves modprobe
resolved the dependency automatically via the modules.dep database that
`depmod` built.

📸 **Screenshot 6**.

Clean up before moving on:

```bash
sudo rmmod bitsfeed
sudo rmmod bitscore
```

### 4.7 checkpatch

```bash
perl $CHECKPATCH --file --no-tree core.c
perl $CHECKPATCH --file --no-tree stats.c
perl $CHECKPATCH --file --no-tree bitsfeed.c
```

**Expected output:** ideally `total: 0 errors, 0 warnings`. If you see
ERRORs, checkpatch tells you the exact line and rule (e.g. "trailing
whitespace", "space required before the open parenthesis") — fix them in
the file, re-run, and only commit once errors are at zero. Warnings
should be minimized but a few are acceptable per the brief.

📸 **Screenshot 7**: the checkpatch output for all three files, ideally
all showing `0 errors`.

If you had to fix anything, commit that too:

```bash
git add -A
git commit -s -m "ch04: fix checkpatch findings"
```

### 4.8 Git log evidence

```bash
git log --oneline -s | head
```

📸 **Screenshot 8**, and also save this as text:

```bash
git log --oneline -s > ../evidence/git-log-ch04.txt
```

### 4.9 Or just run the whole thing at once

Once you're comfortable with each step above, `test.sh` automates the
whole sequence for a single evidence capture:

```bash
sudo ./test.sh 2>&1 | tee ../evidence/ch04-full-run.txt
```

---

## 5. Question 2 — bits7seg.ko

### 5.1 Commit the driver

```bash
cd ~/bits-ddrv-2025CA01061/ch06-bits7seg
git add bits7seg.c Makefile test.sh
git commit -s -m "ch06: implement bits7seg 4-minor emulated char driver"
```

### 5.2 Build and load

```bash
make clean
make
sudo insmod ./bits7seg.ko
ls -l /dev/bits7seg*
```

**Expected output:** four device nodes, `/dev/bits7seg0` through
`/dev/bits7seg3`, owned by root, character device type (`c` in the
permission bits), with the same major number and minors 0–3.

📸 **Screenshot 9**.

### 5.3 Confirm the dynamic major

```bash
grep bits7seg /proc/devices
```

**Expected output:** a line like `240 bits7seg` — some number, not a
fixed/well-known major, proving `alloc_chrdev_region` was used rather
than a hardcoded major.

📸 **Screenshot 10**.

### 5.4 Exercise read/write

```bash
echo -n 7 | sudo tee /dev/bits7seg2
sudo cat /dev/bits7seg2
```

**Expected output:** `digit=7 en=1 blank=0`.

📸 **Screenshot 11**.

### 5.5 Exercise the sysfs attribute

```bash
echo 3 | sudo tee /sys/class/bits7seg/bits7seg2/digit
sudo cat /sys/class/bits7seg/bits7seg2/digit
sudo cat /dev/bits7seg2
```

**Expected output:** sysfs read shows `3`, and the character-device read
now also reflects `digit=3 ...` — proving both interfaces touch the same
underlying register block.

📸 **Screenshot 12**.

### 5.6 Exercise the -EINVAL path

```bash
echo -n x | sudo tee /dev/bits7seg0
```

**Expected output:** `tee` reports something like `tee: /dev/bits7seg0:
Invalid argument`, because your driver's `write()` returned `-EINVAL` for
a non-digit byte.

📸 **Screenshot 13**.

### 5.7 Run the full test script

```bash
sudo ./test.sh 2>&1 | tee ../evidence/ch06-full-run.txt
```

**Expected output:** ends with `=== all bits7seg tests passed ===`.

### 5.8 Unload and confirm clean teardown

```bash
sudo rmmod bits7seg
ls /dev/bits7seg* 2>&1   # should say "No such file or directory"
dmesg | tail             # should show "unloaded", no oops/warning traces
```

📸 **Screenshot 14**.

### 5.9 checkpatch

```bash
perl $CHECKPATCH --file --no-tree bits7seg.c
```

Fix anything reported, then:

```bash
git add -A
git commit -s -m "ch06: fix checkpatch findings"
```

📸 **Screenshot 15**.

---

## 6. Assemble evidence and README

```bash
cd ~/bits-ddrv-2025CA01061
mkdir -p evidence
# Move every screenshot / .txt capture you took above into evidence/,
# named clearly, e.g. evidence/q1-step3-dmesg.png
```

Edit `README.md` to briefly describe what each screenshot in `evidence/`
corresponds to (one line each is enough) — this is what your grader
reads first.

---

## 7. Package the submission

From the parent of your repo:

```bash
cd ~
zip -r 2025CA01061_DDA1.zip bits-ddrv-2025CA01061 \
  -x "bits-ddrv-2025CA01061/ch04-stack/*.ko" \
  -x "bits-ddrv-2025CA01061/ch04-stack/*.o" \
  -x "bits-ddrv-2025CA01061/ch04-stack/*.mod*" \
  -x "bits-ddrv-2025CA01061/ch04-stack/.tmp_versions/*" \
  -x "bits-ddrv-2025CA01061/ch06-bits7seg/*.ko" \
  -x "bits-ddrv-2025CA01061/ch06-bits7seg/*.o" \
  -x "bits-ddrv-2025CA01061/ch06-bits7seg/*.mod*" \
  -x "bits-ddrv-2025CA01061/ch06-bits7seg/.tmp_versions/*"
unzip -l 2025CA01061_DDA1.zip | head -30
```

Confirm the zip contains source, Makefiles, test scripts, README.md, and
your evidence folder — not build artifacts (`.ko`/`.o`/`.mod.c` files are
regenerated by `make` and shouldn't bloat the submission, though a stray
one won't cost you marks; the important thing is everything required is
present).

📸 **Final screenshot**: `unzip -l` output, proving the archive's
contents, saved as the last item in your evidence folder.

---

## 8. Final checklist before you submit

- [ ] Every `.c` file has `MODULE_LICENSE("GPL")`, `MODULE_AUTHOR` (your
      name + BITS ID), `MODULE_DESCRIPTION`.
- [ ] `perl $CHECKPATCH --file --no-tree <file>.c` shows 0 errors for
      every `.c` file.
- [ ] At least 4 commits in `ch04-stack`'s history, all `git commit -s`.
- [ ] All required demonstration commands from the assignment brief have
      a matching screenshot/log in `evidence/`.
- [ ] Zip is named exactly `2025CA01061_DDA1.zip`.
- [ ] You did **not** share code with another student — submissions are
      checked pairwise.

If any command above prints something you don't recognize, paste the
exact output back to me and I'll help you read it and fix it.
