# Path to the built kernel source (KDIR = kernel directory)
KDIR := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/bb-kernel/KERNEL

# A declaration, not a target: kbuild reads this line and knows to build a module from led.o
obj-m += led.o

# Runs on "make": calls the kernel's make to build led.o, then led.ko
all:
# -C: jump into the kernel directory; M=: where my module lives; ARCH=arm: build for ARM; CROSS_COMPILE: cross compiler prefix
	make -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CC32HF) modules

# Runs on "make clean": removes led.o, led.ko and other generated files
clean:
	make -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CC32HF) clean 