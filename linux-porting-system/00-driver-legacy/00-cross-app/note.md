# Bài: Hello World cross compile cho BBB (BeagleBone Black)

## 1. Mục tiêu

Viết chương trình C đơn giản trên **laptop (host, x86)**, biên dịch ra file chạy được trên **BBB (target, ARM)**, rồi chuyển file sang BBB để chạy.

> "I build the program on my host machine and run it on the target board."

---

## 2. Các file trong bài

### main.c

```c
#include <stdio.h>

int main(void)
{
    printf("Hello BBB\n");
    return 0;
}
```

### Makefile

> **Quan trọng:** tên file là `Makefile` (hoặc `makefile`). Các dòng lệnh dưới `all:` và `clean:` phải bắt đầu bằng **phím Tab**, không dùng dấu cách, nếu không sẽ báo lỗi `missing separator`.

```makefile
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
```

**Cách Makefile hoạt động:**

| Lệnh | `CC` là gì | Kết quả |
|---|---|---|
| `make` | `gcc` của laptop | file `hello` chạy trên **x86** (laptop) |
| `make ARCH=arm` | `arm-linux-gnueabihf-gcc` | file `hello` chạy trên **ARM** (BBB) |
| `make clean` | | xóa file `hello` |

---

## 3. Cách build (trên laptop)

Prompt của laptop có dạng `sigma@sigma-GF63...:~$`.

```bash
cd <thư mục chứa main.c và Makefile>
make clean
make ARCH=arm
file hello
```

Kết quả của `file hello` **phải có** `ELF 32-bit LSB executable, ARM`. Nếu thấy `x86-64` nghĩa là bạn quên `ARCH=arm`, file đó không chạy được trên BBB.

Nên chạy `make clean` trước khi build lại để tránh nhầm file x86 cũ với file ARM mới.

---

## 4. Cách chạy trên BBB

Prompt của BBB có dạng `debian@BeagleBone:~$`.

```bash
chmod +x hello
./hello
```

Kết quả mong đợi:

```
Hello BBB
```

---

## 5. Cách gửi file qua mạng (scp)

`scp` (secure copy) chép file từ laptop sang BBB qua SSH. Cách này **nhanh và ít lỗi nhất**, không phụ thuộc USB hay NTFS.

### Bước 1: Tìm địa chỉ IP của BBB

Trên BBB:

```bash
ip a
```

Tìm dòng `inet ...` của card mạng đang dùng. Nếu cắm BBB vào laptop bằng cáp USB thì IP thường là `192.168.7.2`. Nếu dùng cáp mạng/Wi-Fi thì IP do router cấp.

### Bước 2: Kiểm tra laptop thấy BBB

Trên laptop:

```bash
ping <IP_BBB>
```

### Bước 3: Gửi file

Trên laptop (đứng trong thư mục có file `hello`):

```bash
scp hello debian@<IP_BBB>:~/
```

Ví dụ:

```bash
scp hello debian@192.168.7.2:~/
```

- `debian` là user trên BBB
- `~/` là thư mục home của user `debian` trên BBB
- Lần đầu sẽ hỏi `yes/no` (gõ `yes`), sau đó nhập mật khẩu của user `debian`

### Bước 4: Chạy trên BBB

```bash
ls -l ~/hello
chmod +x ~/hello
./hello
```

### Một số lệnh scp hữu ích

```bash
scp file1 file2 debian@<IP_BBB>:~/       # gửi nhiều file
scp -r thu_muc debian@<IP_BBB>:~/        # gửi cả thư mục
scp debian@<IP_BBB>:~/hello ./           # lấy file từ BBB về laptop
```

---

## 6. Cách gửi file qua USB

USB hay được format **NTFS** (từ Windows). Cách này dùng được nhưng có nhiều điểm dễ sai, nên làm cẩn thận từng bước.

### Bước 1: Chép file vào USB (trên laptop)

Tên USB trên laptop có thể là `/media/sigma/sigma` (xem bằng `lsblk -f`).

```bash
cp hello /media/sigma/sigma/temp/
sync
ls -l /media/sigma/sigma/temp/hello
```

- `sync`: ép hệ thống ghi dữ liệu xuống USB
- `ls -l`: **xác nhận file thật sự nằm trên USB**. Đừng tin vào cửa sổ Files vì nó có thể hiện dữ liệu cũ trong bộ nhớ đệm (cache)

### Bước 2: Eject USB đúng cách (trên laptop)

```bash
sudo umount /media/sigma/sigma
```

Không báo lỗi mới rút USB. Nếu báo `busy`, đóng cửa sổ Files đang mở USB rồi chạy lại.

### Bước 3: Cắm USB vào BBB và mount

Tên thiết bị USB có thể đổi mỗi lần cắm (`sda1`, `sdb1`...), nên luôn kiểm tra trước:

```bash
lsblk
```

Giả sử USB là `sda1`:

```bash
sudo mount -o ro,uid=1000,gid=1000 /dev/sda1 /media/usb
ls /media/usb/temp
```

- `-o ro`: mount chỉ đọc (read-only), an toàn cho USB
- `uid=1000,gid=1000`: để user `debian` đọc được, không cần `sudo` mỗi lần
- Nếu báo `mount point does not exist`: tạo thư mục bằng `sudo mkdir -p /media/usb`

### Bước 4: Chép về home và chạy

**Không chạy trực tiếp trên USB** vì NTFS không giữ quyền execute. Hãy chép về home:

```bash
cp /media/usb/temp/hello ~/
chmod +x ~/hello
./hello
```

### Bước 5: Gỡ USB đúng cách (trên BBB)

```bash
sudo umount /media/usb
```

Sau đó mới rút USB.

---

## 7. Lỗi thường gặp

| Lỗi | Nguyên nhân | Cách xử lý |
|---|---|---|
| `Permission denied` khi `./hello` | File chưa có quyền execute | `chmod +x hello` |
| `chmod777: command not found` | Thiếu dấu cách | Viết `chmod 777 hello` hoặc `chmod +x hello` |
| `Exec format error` | Build nhầm cho x86, không phải ARM | `make clean` rồi `make ARCH=arm`, kiểm tra bằng `file hello` |
| `Permission denied` khi `ls /media/usb/...` | Mount NTFS với quyền chỉ root | Mount với `uid=1000,gid=1000` hoặc dùng `sudo ls` |
| `mount point does not exist` | Chưa có thư mục `/media/usb` | `sudo mkdir -p /media/usb` |
| Không thấy file trên BBB dù thấy trên laptop | File chưa ghi xuống USB (cache) hoặc rút USB chưa eject | `sync`, `umount`, và kiểm tra bằng `ls -l` |
| `missing separator` khi chạy `make` | Dòng lệnh trong Makefile dùng dấu cách | Thay bằng phím **Tab** |
| `command not found: arm-linux-gnueabihf-gcc` | Sai đường dẫn `TOOLCHAIN` | Kiểm tra đường dẫn trong Makefile có đúng không |

**Mẹo:** nhìn **prompt** để biết bạn đang ở máy nào:
- `sigma@sigma-GF63...:~$` là laptop
- `debian@BeagleBone:~$` là BBB

---

## 8. Phân biệt `+x` và `777`

- `chmod +x file`: chỉ **thêm quyền chạy** (execute). Nên dùng.
- `chmod 777 file`: cho **mọi người** quyền đọc, ghi, chạy (`7 = 4+2+1 = r+w+x`). Không cần thiết, kém an toàn hơn.

---

## 9. Câu tiếng Anh để nhớ

- "I cross-compile the program on my laptop and run it on the BeagleBone."
- "Use `scp` to copy the file from the host to the target."
- "The file needs execute permission before I can run it."
- "Always unmount the USB drive before removing it."

## 10. Từ vựng

| English | Tiếng Việt |
|---|---|
| cross compile | biên dịch chéo |
| host / target | máy phát triển / board đích |
| executable | file chạy được |
| permission | quyền truy cập |
| mount / unmount | gắn / gỡ thiết bị lưu trữ |
| read-only | chỉ đọc |
| cache | bộ nhớ đệm |
| secure copy (scp) | sao chép an toàn qua SSH |
| IP address | địa chỉ IP |