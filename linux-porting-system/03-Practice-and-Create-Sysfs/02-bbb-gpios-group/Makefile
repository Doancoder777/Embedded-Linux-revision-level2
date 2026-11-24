# ============================================================
# bbb-sysfs kernel module for BeagleBone Black
# ============================================================

# ---------- 1. Paths ----------
# Path to the built kernel source (KDIR = kernel directory)
KDIR := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/bb-kernel/KERNEL

# ---------- 2. Kernel module config (kbuild reads these) ----------
# Build bbb-sysfs.ko from bbb-sysfs.c
obj-m := bbb-sysfs.o

# Extra warning flags for the module
ccflags-y := -Wall

# ---------- 3. Targets ----------
.PHONY: all clean

# Default target: build the module (CC32HF = cross compiler prefix, set in your environment)
all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=arm CROSS_COMPILE=$(CC32HF) modules

# Remove all generated files
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=arm CROSS_COMPILE=$(CC32HF) clean
