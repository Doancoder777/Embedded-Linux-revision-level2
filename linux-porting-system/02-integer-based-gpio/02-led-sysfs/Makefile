# ============================================================
# LED sysfs app for BeagleBone Black
#   make            -> cross compile with the ARM toolchain (runs on BBB)
#   make ARCH=x86   -> build with the laptop gcc (x86 file, runs on the laptop only)
# ============================================================

# ---------- 1. Toolchain ----------
# Path to the bin/ folder of the ARM toolchain (runs on x86 laptop, makes code for ARM)
TOOLCHAIN := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin

# Tool name prefix: ARM + Linux + glibc + hard-float
PREFIX := arm-linux-gnueabihf-

# Default is ARM, so just typing "make" cross compiles for BBB.
# To build for the laptop instead, run: make ARCH=x86
# (":=" also ignores any ARCH variable already set in the shell;
#  a value typed on the command line still wins.)
ARCH := arm

# When ARCH is arm, use the ARM gcc (cross compile).
# Otherwise CC keeps the default gcc of the laptop (x86 output).
ifeq ($(ARCH),arm)
    CC := $(TOOLCHAIN)/$(PREFIX)gcc
endif

# ---------- 2. Build settings ----------
# -Wall -Wextra : show more warnings, helps find bugs early
# -O2           : optimize the code
CFLAGS  := -Wall -Wextra -O2

# Name of the program we want to create
TARGET  := app

# Source files to compile
SRCS    := app.c gpio.c

# ---------- 3. Targets ----------
.PHONY: all clean

# Default target: runs when you type just "make"
all: $(TARGET)

# Rebuild "app" when any .c file or gpio.h changes.
#
# Automatic variables (make fills them in for you):
#   $@  = the TARGET name           -> app
#   $^  = ALL the prerequisites     -> app.c gpio.c gpio.h
#   $<  = the FIRST prerequisite    -> app.c
# Remember: it is "$@" (dollar first, then at), NOT "@$".
#
# With the default ARCH=arm the line below expands to:
#   <TOOLCHAIN>/arm-linux-gnueabihf-gcc -Wall -Wextra -O2 -o app app.c gpio.c
$(TARGET): $(SRCS) gpio.h
	$(CC) $(CFLAGS) -o $@ $(SRCS)

# "make clean": remove the generated program
clean:
	rm -f $(TARGET)