# Đường dẫn tới thư mục bin/ của toolchain ARM (chạy trên laptop x86, tạo code cho ARM)
TOOLCHAIN := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin

# Tiền tố tên công cụ: ARM + Linux + glibc + hard-float
PREFIX := arm-linux-gnueabihf-

# Chỉ khi chạy "make ARCH=arm" mới dùng gcc ARM (cross compile)
# Không truyền ARCH=arm thì CC giữ mặc định là gcc của laptop (ra file x86)
ifeq (${ARCH}, arm)
	CC := $(TOOLCHAIN)/$(PREFIX)gcc
endif

# Target mặc định khi gõ "make": biên dịch main.c thành file hello
all:
	$(CC) -o hello main.c

# "make clean": xóa file hello
clean:
	rm -rf hello