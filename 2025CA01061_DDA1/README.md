# Assignment 1 — Kernel Build System, Module Stacking & Emulated MMIO

**Name:** Purva Bhagwagar
**BITS ID:** 2025CA01061
**Course:** Device Drivers (Masters, BITS Pilani)

## Layout

```
2025CA01061_DDA1/
├── ch04-stack/          Question 1 — bitscore.ko + bitsfeed.ko
│   ├── core.c
│   ├── stats.c
│   ├── bitsfeed.c
│   ├── bitscore.h
│   ├── bitscore_internal.h
│   ├── Makefile
│   └── test.sh
├── ch06-bits7seg/       Question 2 — bits7seg.ko
│   ├── bits7seg.c
│   ├── Makefile
│   └── test.sh
├── evidence/            dmesg captures, terminal transcripts, screenshots
└── README.md
```

## How to build and run

See `STEP_BY_STEP_GUIDE.md` for the full walkthrough (environment setup,
build, load, test, checkpatch, packaging, and exactly where to capture
evidence). Short version:

```bash
cd ch04-stack && make && sudo ./test.sh
cd ../ch06-bits7seg && make && sudo insmod ./bits7seg.ko && sudo ./test.sh
```

## Evidence

All required dmesg output, terminal transcripts, and screenshots are in
`evidence/` (or inlined below this line once you fill them in), one file
per required demonstration step, named to match the step number in the
assignment brief.

## Git

Work was developed in `bits-ddrv-2025CA01061` with meaningful, signed-off
commits (`git commit -s`). See `evidence/git-log.txt` for
`git log --oneline -s`.
