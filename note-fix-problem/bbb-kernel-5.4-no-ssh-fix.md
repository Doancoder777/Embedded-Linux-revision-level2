# BBB: Boot kernel 5.4.288 được nhưng không SSH được

## 1. Triệu chứng

- Sau khi đổi `uname_r=5.4.288-bone69` trong `/boot/uEnv.txt` rồi `reboot`:
  - LED trên board nhấp nháy (kernel đang chạy).
  - Laptop **không thấy** card mạng USB, `ssh debian@192.168.7.2` không vào được.
- Cùng triệu chứng khi boot từ thẻ SD (Debian 10 + kernel 5.4).

## 2. Chẩn đoán (qua cáp USB-TTL)

Nối cáp USB-TTL (3.3V) vào header debug **J1** của BBB, mở `sudo screen /dev/ttyUSB0 115200`.

| Chân J1 | Nối với cáp |
|---|---|
| 1 (chấm trắng) | GND |
| 4 (RX board) | TX cáp |
| 5 (TX board) | RX cáp |

Không nối dây VCC (đỏ).

Kết quả đọc được:

- Hiện `BeagleBone login:` → kernel và rootfs **đã boot bình thường**.
- `uname -r` → `5.4.288-bone69`.
- `ip -br a` trên board chỉ có `lo`, `dummy0`, `can0`, `can1`, `eth0`. **Không có `usb0`.**

## 3. Nguyên nhân

Board **không nạp driver `g_ether`** khi chạy kernel 5.4, nên không tạo ra card mạng USB (`usb0`). Laptop không có mạng USB để SSH.

Chưa xác nhận: vì sao rootfs Debian 11 không tự nạp `g_ether` (nghi ngờ rootfs cấu hình USB gadget cho kernel 5.10). Chưa kiểm tra script khởi động.

## 4. Cách fix tạm (đã chạy, có hiệu quả)

Trên serial:

```
sudo modprobe g_ether
ip -br a
```

- Board xuất hiện `usb0`.
- Laptop thấy card `enx...` với IP `192.168.7.1`.
- `ssh debian@192.168.7.2` vào được.

Mất hiệu lực sau mỗi lần reboot.

## 5. Cách fix lâu dài (CHƯA kiểm chứng sau reboot)

Trên board (qua SSH):

```
echo g_ether | sudo tee -a /etc/modules
cat /etc/modules
```

Phải thấy dòng `g_ether`. Sau đó `sudo reboot`, đợi 1-2 phút, thử SSH lại.

Nếu vẫn không có mạng sau reboot: để cáp serial cắm sẵn, chạy lại `sudo modprobe g_ether`, rồi kiểm tra script khởi động USB gadget (ví dụ `/opt/scripts/boot/`, `systemctl status`).

Nếu mạng lên nhưng laptop không có IP, trên board thử:

```
sudo ip addr add 192.168.7.2/24 dev usb0
sudo ip link set usb0 up
```

## 6. Nạp led.ko

```
uname -r
modinfo ~/led.ko | grep vermagic
sudo insmod ~/led.ko
lsmod | grep led
dmesg | tail
sudo rmmod led
```

- `vermagic` phải là `5.4.288-bone69 SMP preempt mod_unload modversions ARMv7 p2v8`.
- Trạng thái: vermagic đã khớp. `insmod` chưa chạy.

## 7. Đường lui

- Đổi `uname_r=` về `5.10.168-ti-r72` trong `/boot/uEnv.txt` (đã có bản backup `/boot/uEnv.txt.bak`).
- Kernel 5.10 vẫn còn trong `/boot` và `/lib/modules`.

## 8. Mẹo khi dùng serial

- Serial hay rớt chữ: gõ chậm, từng lệnh ngắn, không dán nhiều dòng, dùng `Tab` để tự điền.
- Khi `usb0` lên, chuyển sang SSH cho đỡ lỗi.
- Dây GND và TX/RX phải cắm chắc vào J1.
- Giờ board sai lệch (file "in the future" khi `tar`) là bình thường, BBB không có pin đồng hồ.
