# Note: Linux Porting - BeagleBone Black (BBB)

## 1. BSP (Board Support Package)
Bộ đồ nghề để xây dựng phần mềm cho **một board cụ thể** (ở đây là BBB).
Gồm 4 thành phần chính:

| Thành phần | Folder (máy mình) | Vai trò | Ví dụ quán ăn |
|---|---|---|---|
| Toolchain | `gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf` | Biên dịch code cho chip ARM | Bếp nấu |
| Bootloader | `u-boot` | Chạy đầu tiên khi cấp nguồn, nạp kernel | Người mở cửa |
| Kernel | `bb-kernel` | Điều khiển phần cứng, chia việc cho app | Bếp trưởng |
| Rootfs | `debian-10.10-minimal-armhf-2021-06-20` | Cây thư mục `/bin`, `/etc`, `/home`... | Bàn ghế, thực đơn |

Thư mục làm việc: `~/Desktop/emebedded-linux-lv2/linux-porting-system/BBB`

Tự build (như thầy dạy) = 4 folder nguyên liệu trên laptop.
OS có sẵn (image) = một khối đã trộn xong, không thấy nguyên liệu riêng.

## 2. Toolchain và cross compile
- Laptop dùng chip **x86**, BBB dùng chip **ARM** -> hai ngôn ngữ máy khác nhau.
- Build trên máy này để chạy trên máy khác = **cross compile**.
- Toolchain chỉ nằm trên **laptop**, board không cần.
- Biến `CROSS_COMPILE=${CC32HF}` chỉ cho `make` biết trình biên dịch ARM nằm ở đâu.
  Không có nó, `make` dùng gcc của laptop -> ra file x86, không chạy được trên BBB
  (lỗi thường gặp: `unrecognized command-line option '-mfpu=vfp'`, và dòng `CROSS_COMPILE=` bị trống).
- Tên toolchain đọc theo quy ước `<chạy trên>_<tạo ra cho>`, ví dụ
  `gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf`: chạy trên x86_64, tạo ra cho ARM + Linux.

### Tiền tố (prefix) của công cụ
- Trong `bin/` của toolchain, mọi công cụ đều có tiền tố, ví dụ `arm-linux-gnueabihf-gcc`.
- Tác dụng: phân biệt với `gcc`, `ld`, `objdump`... của laptop (x86), và cho `make` ghép tên công cụ từ một biến
  (`CROSS_COMPILE`, hoặc `PREFIX` trong Makefile của thầy).
- Đọc tiền tố: `arm` (chip ARM 32-bit) - `linux` (chạy Linux) - `gnueabihf` (glibc + EABI + hard-float).
- Gõ `gcc` rồi bấm **Tab** (không bấm Enter) để liệt kê lệnh. Bấm Enter sẽ chạy `/usr/bin/gcc` của laptop và báo `no input files`.
- Máy mình có sẵn `gcc`, `g++` của laptop (x86): chúng ra file x86, KHÔNG chạy được trên BBB.
- Biến môi trường `export` chỉ sống trong terminal hiện tại; muốn mọi terminal đều có thì ghi vào `~/.bashrc`.
  `CC32HF` đã nằm trong `~/.bashrc`.

### libc
- **libc** = thư viện C chuẩn (`printf`, `malloc`, `open`, `read`...). Cầu nối giữa app và kernel.
- Kernel không dùng libc (tự có `printk`, `kmalloc`).
- Toolchain đầy đủ có `g++`, `gdb`, `gfortran`, `gprof`, `ld.gold`... giống bộ của thầy.
- `gnueabi` (soft-float) vs `gnueabihf` (hard-float): khác nhau ở cách xử lý số thực.
  App chạy trên rootfs Debian armhf phải là `hf`.
- KHÔNG BAO GIỜ xóa libc trên laptop Ubuntu (máy sẽ hỏng). KHÔNG gỡ `python3`.

### Toolchain của thầy đã chết
- Khóa học thầy dùng Linaro GCC 6.5.0 (2018), link `releases.linaro.org` đã bị chuyển hướng (301)
  về trang liên hệ, mirror `releases-ie.linaro.org` bị `Connection timed out`.
- Bộ toolchain không hỏng, chỉ là chỗ tải biến mất (Linaro ngừng làm binary, Arm tiếp quản).
- Repo khóa học đã bị xóa. Repo GitHub cá nhân của người lạ chứa toolchain `aarch64` (ARM 64-bit)
  là loại SAI cho BBB, và nguồn không đáng tin -> không dùng.
- Các trang chia sẻ file lạ (kết quả Google) cũng đã `object not found` hoặc chỉ nhắc tên file -> không dùng.
- Mẹo: sau `wget`, kiểm tra dung lượng và dòng `301 Moved ... Location:`. File `.tar.xz` chỉ vài chục KB = tải nhầm trang web.
- Thay thế: **Arm GNU Toolchain 8.3-2019.03** (mục 4.5), tiền tố giống hệt thầy.

### Quyết định
- Dùng toolchain `CC32HF` cho tất cả (app, kernel module, U-Boot), giống bộ của thầy.
- Giữ kernel 5.4.288 để bám khóa của thầy. Không đổi sang bản mới nhất.
- Khác biệt về *công cụ* (link chết, phiên bản gcc) thì dùng bản thay thế; khác biệt về *thứ thầy dạy* (kernel 5.4) thì giữ như thầy.
- Nạp kernel 5.4 qua **thẻ microSD**, giữ eMMC (Debian 11, kernel 5.10) làm bản dự phòng.

## 3. Setup bb-kernel (kernel 5.4, giống máy thầy)

### Thông tin đã xác nhận
- Repo: https://github.com/RobertCNelson/bb-kernel
- Nhánh: `am33x-v5.4`
- Commit bb-kernel: `8616e46ec61ee5552e070077a2bac6016143ee4b`
- Kernel sau khi patch: tag `v5.4.288-bone69` (VERSION=5, PATCHLEVEL=4, SUBLEVEL=288)
- Host đã test: Ubuntu 24.04, x86_64
- Kernel đã build xong (file kernel nằm trong `bb-kernel/KERNEL` và `deploy/`).

### Cách xác nhận đã giống thầy
Trong `KERNEL/arch/arm/boot/dts/`:
- `am33xx.dtsi` dòng 1-9 là license kiểu cũ, KHÔNG có dòng `// SPDX`.
- `am335x-boneblack-common.dtsi` có `&am33xx_pinmux` ở dòng 28 với `nxp_hdmi_bonelet_pins` (LCD_DATA0..15).
- `am335x-bone-common.dtsi` dòng 22-33 là node `leds` (led2: gpio1 21, heartbeat).

### Các bước
```bash
sudo apt-get update
sudo apt-get install git bison device-tree-compiler flex gettext libmpc-dev \
    u-boot-tools lz4 bc libssl-dev libgmp-dev lzop
# (Nếu script đòi thêm: bindgen rust-src rustc rustfmt rust-clippy)

git clone https://github.com/RobertCNelson/bb-kernel
cd bb-kernel
git checkout am33x-v5.4
git checkout 8616e46ec61ee5552e070077a2bac6016143ee4b   # ghim đúng commit (tùy chọn)
./build_kernel.sh
```

### Lưu ý khi chạy build_kernel.sh
- Lần đầu mất vài chục phút (clone kernel ~2.5 GB vào `ignore/linux-src`).
- Báo "Failed dependency check": cài gói được liệt kê rồi chạy lại.
- Menuconfig cần terminal tối thiểu 19 dòng x 80 cột. Khi hiện: Esc 2 lần -> Yes để lưu (không cần chỉnh gì).
- Build xong: kernel, DTB, modules nằm trong `deploy/`; `arch/arm/boot` có `zImage`, `Image`.
- Đổi nhánh: `git checkout am33x-v5.10` (hoặc `am33x-v4.19`) rồi `./build_kernel.sh` lại.

### Kiểm tra nhanh
```bash
head -n 5 KERNEL/Makefile
head -n 9 KERNEL/arch/arm/boot/dts/am33xx.dtsi
```

### Mẹo VS Code
- Ctrl+P không tìm được file trong `KERNEL/` (bị .gitignore ẩn) -> mở từ Explorer.
- "Collapse Folders in Explorer" để thu gọn cây thư mục.
- Xem cây thư mục chỉ có folder: `tree -d -L 2` (cài bằng `sudo apt install tree`).

### Ngoài phạm vi repo này
`00-driver-legacy`, `01-practice`, `02-integer-based-gpio`, `03-practice`, `led.c` là bài tập của thầy (repo course), không nằm trong bb-kernel.

## 4. Setup toolchain, U-Boot, rootfs

### 4.1 U-Boot (nhánh vá sẵn của BeagleBoard, theo hướng dẫn Robert Nelson)
```bash
sudo apt install bc bison flex libgnutls28-dev libssl-dev python3-dev python3-setuptools swig uuid-dev
git clone -b v2022.04-bbb.io-am335x-am57xx https://github.com/beagleboard/u-boot.git --depth=1
make -C ./u-boot/ CROSS_COMPILE=${CC32HF} am335x_evm_defconfig
make -C ./u-boot/ -j4 CROSS_COMPILE=${CC32HF}
ls -lh u-boot/MLO u-boot/u-boot-dtb.img
```
- Đã build xong: `MLO` 105K, `u-boot-dtb.img` 1.1M.
- U-Boot là source, toolchain là công cụ biến source thành file chạy được trên ARM.
- `MLO` = bootloader giai đoạn đầu (nhỏ). `u-boot-dtb.img` = U-Boot chính.

### 4.2 Rootfs (Debian 10.10 minimal armhf)
```bash
wget -c https://rcn-ee.com/rootfs/eewiki/minfs/debian-10.10-minimal-armhf-2021-06-20.tar.xz
tar xf debian-10.10-minimal-armhf-2021-06-20.tar.xz
```
- Link còn sống (136 MB). Bên trong có `armhf-rootfs-debian-buster.tar`.
- Rootfs là file đã build sẵn, KHÔNG cần toolchain.

### 4.3 Kết quả `ls`
```
bb-kernel  debian-10.10-minimal-armhf-2021-06-20  debian-10.10-minimal-armhf-2021-06-20.tar.xz
gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf  note.md  u-boot
```

### 4.4 Việc tiếp theo
Chia thẻ SD (boot + rootfs), chép `MLO`, `u-boot-dtb.img`, kernel (`deploy/`), rootfs vào thẻ.
Trước khi chia ổ: gõ `lsblk` để xác nhận đúng tên thẻ (tránh xóa nhầm ổ laptop).

### 4.5 Toolchain: Arm GNU Toolchain 8.3-2019.03
```bash
cd ~/Desktop/emebedded-linux-lv2/linux-porting-system/BBB
wget -c https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/8.3-2019.03/binrel/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz
ls -lh gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz     # 245 MB là đúng
tar xf gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz

echo 'export CC32HF=$HOME/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-' >> ~/.bashrc
source ~/.bashrc
${CC32HF}gcc --version     # GCC 8.3.0
${CC32HF}g++ --version     # GCC 8.3.0
```
- Đã tải thành công (244M). `bin/` giống ảnh của thầy: có `g++`, `gdb`, `gfortran`, `gprof`, `ld.gold`...
- Tiền tố `arm-linux-gnueabihf-` **trùng hệt thầy** (khác bản 10.3 của Arm là `arm-none-linux-gnueabihf-`).
- `CC32HF` kết thúc bằng dấu `-` và chưa có chữ `gcc`: `make` tự ghép `${CC32HF}` + `gcc`.
- Nén backup để link chết không mất công tải lại:
  `tar czf toolchain-hf-backup.tar.gz gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf`

### 4.6 gdb: `arm-linux-gnueabihf-gdb` của bộ 8.3 KHÔNG chạy được trên Ubuntu 24.04
- Lỗi lần lượt: `libtinfo.so.5` -> `libncursesw.so.5` -> `libpython2.7.so.1.0`.
- Hai cái đầu vá bằng liên kết `.5` -> `.6` (đã tạo trên máy mình):
  `sudo ln -s /lib/x86_64-linux-gnu/libtinfo.so.6 /lib/x86_64-linux-gnu/libtinfo.so.5`
  `sudo ln -s /lib/x86_64-linux-gnu/libncursesw.so.6 /lib/x86_64-linux-gnu/libncursesw.so.5`
  Gỡ nếu muốn: `sudo rm /lib/x86_64-linux-gnu/libtinfo.so.5 /lib/x86_64-linux-gnu/libncursesw.so.5`
- `libpython2.7` thì KHÔNG vá được (Ubuntu 24.04 không còn Python 2.7, không trỏ sang Python 3 được). Không cài Python 2.7.
- Cách dùng thay: `sudo apt install gdb-multiarch` (đã cài, GDB 15.1, dùng Python 3 sẵn có, không đụng python3 của hệ thống).
  - `gdb-multiarch ./hello` rồi `set architecture arm`
  - Debug từ xa: `target remote 192.168.7.2:1234` (chạy `gdbserver` trên board).
- Đừng chạy `apt autoremove` bừa trên máy này (danh sách có gói nvidia).

### 4.7 Bài `00-cross-app` của thầy (app `hello`, có `printf`)
Thư mục có: `hello`, `main.c`, `Makefile`. Makefile của thầy (14 dòng):
```makefile
# Toolchain
TOOLCHAIN := /home/phong/working_space/Linux-Porting-N-System/BBB/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin
PREFIX := arm-linux-gnueabihf-

# ARCH
ifeq (${ARCH}, arm)
	CC := $(TOOLCHAIN)/$(PREFIX)gcc
endif

all:
	$(CC) -o hello main.c

clean:
	rm -rf hello
```
- Ý nghĩa: `make ARCH=arm` thì `CC` trỏ gcc ARM; không truyền `ARCH=arm` thì `make` dùng `cc` của laptop (ra file x86).
- Bản chỉnh cho máy mình: đổi đúng một dòng `TOOLCHAIN`:
```makefile
TOOLCHAIN := /home/sigma/Desktop/emebedded-linux-lv2/linux-porting-system/BBB/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin
PREFIX := arm-linux-gnueabihf-
```
```bash
make ARCH=arm
file hello      # phải thấy: ELF 32-bit LSB executable, ARM, EABI5
```
- Dòng lệnh dưới `ifeq`, `all:`, `clean:` phải bắt đầu bằng **Tab**, không phải dấu cách (không thì `missing separator`).
- Chạy trên board mà báo `GLIBC_2.xx not found`: thêm `-static` vào lệnh build.
- Bài này tự gọi gcc nên cần `TOOLCHAIN` + `PREFIX`. Bài kernel module thì không (xem mục 5).

## 5. Kernel headers và kernel module
- Kernel = cả source (`.c` và `.h`). Kernel headers = chỉ các file `.h`.
- Build module cho BBB cần: `bb-kernel` (source đã build) + toolchain `CC32HF`.
- Phiên bản kernel trên board phải khớp phiên bản đã build (`uname -r`), lệch -> `insmod` lỗi `Invalid module format`.
- Makefile mẫu (đã có comment):
```makefile
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
```
- `obj-m` là **biến** (khai báo) mà kbuild đọc, KHÔNG phải target và không tự sinh `led.o`.
- Luồng chạy: gõ `make` -> chạy `all` -> `all` gọi make của kernel (`-C $(KDIR)`) -> make của kernel quay lại đọc
  Makefile của mình (`M=$(PWD)`), thấy `obj-m += led.o` -> lúc đó mới tạo `led.o` rồi `led.ko`.
- `CROSS_COMPILE` nhận `CC32HF` (đã gồm đường dẫn + tiền tố, thiếu chữ `gcc`), kbuild tự ghép thêm `gcc`.
- Nếu `CROSS_COMPILE=` in ra trống: biến chưa có trong terminal, kbuild dùng gcc x86 -> lỗi `-mfpu=vfp`.
  Kiểm tra bằng `echo $CC32HF`.
- Kiểm tra sau khi build: `modinfo led.ko | grep vermagic` (phải thấy `5.4.288`).
- `all` và `clean` không tạo file cùng tên nên nên thêm `.PHONY: all clean`.

## 6. Cài OS nhanh cho BBB (image có sẵn)
- BBB có 2 nơi chứa OS: **eMMC** (trong) và **thẻ microSD**.
- Board 2nd của mình: Debian 11 (bullseye), kernel `5.10.168-ti-r72`, boot từ eMMC (`mmcblk1`).
- Mặc định: user `debian`, mật khẩu `temppwd` -> nên đổi bằng `passwd`.
- Boot từ SD: tắt nguồn, cắm thẻ, **giữ nút User Boot**, cắm nguồn. Không giữ nút = boot từ eMMC.
- `lsblk`: `/` ở `mmcblk0p1` = chạy từ SD, `mmcblk1p1` = chạy từ eMMC.
- Image **flasher** tự ghi đè eMMC. Chỉ không để nổi bật để tránh lỡ tay, không phải bị cấm.
- SSH: `ssh debian@192.168.7.2`. Lỗi host key: `ssh-keygen -R 192.168.7.2`.
- glibc trên board Debian 11 là 2.31 (kiểm tra: `ldd --version | head -n 1`).

## 7. Lưu ý code kernel module
- Board là 5.10, thầy dạy 5.4, laptop Ubuntu kernel mới.
- `class_create`: kernel < 6.4 dùng `class_create(THIS_MODULE, "m_class")`; kernel >= 6.4 dùng `class_create("m_class")`.
  Dùng `#include <linux/version.h>` + `LINUX_VERSION_CODE` để một file chạy cả hai nơi.
- `class_create` / `device_create` trả về **error pointer** -> kiểm tra bằng `IS_ERR()`, không dùng `== NULL`.
- Major number = driver nào; Minor number = thiết bị nào của driver đó.
- Bài `led` (`00-driver-legacy/01-led-blinking`): tên macro trong `led.c` phải khớp `led.h`
  (`GPIO_OE_OFFSET` vs `GPIO0_OE0_OFFSET`, v.v.), không khớp thì build lỗi.

## 8. Câu tiếng Anh để nhớ
- "A BSP contains the toolchain, bootloader, kernel, and root filesystem."
- "I build the module on my host machine and run it on the target board."
- "U-Boot is just source code, so I need the cross compiler to build it for the ARM board."
- "The old toolchain is no longer available, so I use a newer one with the same target."
- "The tool name has a prefix so it does not clash with the host compiler."
- "I use one full toolchain for both the kernel module and user applications."
- "obj-m is a declaration that kbuild reads, not a target."

## 9. Từ vựng
| English | Tiếng Việt |
|---|---|
| BSP (Board Support Package) | bộ phần mềm hỗ trợ board |
| toolchain / cross compiler | bộ công cụ biên dịch chéo |
| bootloader | chương trình khởi động |
| root filesystem (rootfs) | hệ thống file gốc |
| libc | thư viện C chuẩn |
| host / target | máy phát triển / board đích |
| patch | bản vá |
| redirect / dead link | chuyển hướng / link chết |
| checksum | mã kiểm tra toàn vẹn file |
| prefix | tiền tố |
| hard-float / soft-float | số thực bằng phần cứng / bằng phần mềm |
| shared library | thư viện chia sẻ (`.so`) |
| symlink | liên kết mềm |
| kbuild | hệ thống build của kernel |
| environment variable | biến môi trường |