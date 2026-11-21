# ============================================================
# Hello world kernel module for BeagleBone Black
# ============================================================

# ---------- 1. Paths ----------
# Path to the built kernel source (KDIR = kernel directory)
KDIR := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/bb-kernel/KERNEL

# ---------- 2. Toolchain ----------
# CC32HF is the cross compiler prefix, e.g. arm-linux-gnueabihf-
CROSS_CC := $(CC32HF)gcc

# ---------- 3. Kernel module config (kbuild reads these) ----------
# Build hello.ko from hello.c
obj-m := hello.o

# Extra warning flags for the module
ccflags-y := -Wall

# Tell the compiler to also search for header files in the lib folder
ccflags-y += -I$(src)/lib

# ---------- 4. Targets ----------
.PHONY: all module app clean

# Default target: build both the module and the app
all: module app

# Build the kernel module (hello.ko)
module:
	make -C $(KDIR) M=$(CURDIR) ARCH=arm CROSS_COMPILE=$(CC32HF) modules

# Build the user space app
app:
	$(CROSS_CC) -Wall test_app.c -o app

# Remove all generated files
clean:
	make -C $(KDIR) M=$(CURDIR) ARCH=arm CROSS_COMPILE=$(CC32HF) clean
	rm -rf app