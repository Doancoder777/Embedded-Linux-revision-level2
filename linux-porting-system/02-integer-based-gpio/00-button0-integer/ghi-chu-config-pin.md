# Ghi chú: nút bấm không ăn trên BBB, dùng config-pin

Bài: `00-button0-integer/button.c` (điều khiển LED bằng nút, ngắt GPIO kiểu số nguyên)

## 1. Triệu chứng

- `insmod button.ko` thành công, dmesg có dòng `Hello! Initizliaze successfully!`
- `/proc/interrupts` có dòng `btn_irq` trên chân 26 nhưng **số đếm luôn bằng 0**
- Bấm nút không có dòng `BTN interrupt - LED state is: ...`

## 2. Nguyên nhân

Không phải lỗi code, cũng không phải đấu dây sai.

- Nút nối chân P8_14 xuống GND, code dùng `IRQF_TRIGGER_FALLING` (ngắt khi chân đi từ 1 xuống 0).
- Nhưng lệnh kiểm tra cho thấy `gpio-26 ... in lo`: lúc **không bấm**, chân đã ở mức 0 sẵn.
- Chân ở mức 0 rồi thì bấm nối GND cũng không tạo ra cạnh xuống, nên không có ngắt.
- Cần **điện trở kéo lên (pull-up)** để lúc nhả nút chân ở mức 1.
- Việc cấu hình chân (pinmux, pull-up) do hệ điều hành lo, **không nằm trong code C của module** (với `gpio_*` kiểu cũ).

## 3. Cách sửa tạm thời: config-pin

Mất khi reboot, phải chạy lại mỗi lần khởi động.

```
sudo config-pin P8_14 gpio_pu
sudo grep "gpio-26" /sys/kernel/debug/gpio
```

Kết quả mong đợi: dòng `gpio-26` có `in hi` (mức cao khi không bấm).

Sau đó bấm nút, kiểm tra:

```
sudo dmesg | tail -5
cat /proc/interrupts | grep btn
```

Số đếm phải tăng mỗi lần bấm.

### Chân LED (P9_13)

Từ output cũ: `gpio-31 (P9_13 [uart4_txd])` nghĩa là chân có thể đang ở chế độ UART4. Nếu LED không sáng thì chạy:

```
sudo config-pin P9_13 gpio
```

## 4. Cách sửa cố định: Device Tree (chưa học)

- Khai báo P8_14 là GPIO input có pull-up (pinmux) trong file `.dts` hoặc overlay.
- Device Tree được nạp lại mỗi lần boot nên cấu hình giữ lại.
- Nếu chỉ nạp overlay tạm bằng lệnh thì vẫn mất khi reboot; muốn giữ phải ghi vào file cấu hình boot (thường là `/boot/uEnv.txt` trên Debian của BBB).
- Học ở Khóa 2: Bài 05 Device Tree, Bài 06 Practice Device Tree, Bài 07-08 Pincontrol.

## 5. Bản đồ chân

| Tên trong code | Số GPIO | Chân trên header |
|---|---|---|
| GPIO_BTN = 26 | gpio0_26 | P8_14 |
| GPIO_LED = 31 | gpio0_31 | P9_13 |

`gpiochip0` = `44e07000.gpio`, nên số 26 và 31 trong code là đúng.

## 6. Các lỗi code đã sửa trong bài này

- `pr_infor` đổi thành `pr_info`
- `qpio_request` đổi thành `gpio_request`
- Thêm `module_init(btn_init);` và `module_exit(btn_exit);`
- Thêm `MODULE_LICENSE("GPL");`
- Thêm `gpio_to_irq` + `request_irq` và kiểm tra `retval`
- `gpiod_set_debounce` (nhận con trỏ) đổi thành `gpio_set_debounce` (nhận số nguyên)
- `IRQF_TRIGGER_RISING` đổi thành `IRQF_TRIGGER_FALLING`

## 7. Chưa kiểm chứng

- Chưa xác nhận kết quả sau khi chạy `config-pin P8_14 gpio_pu` (ngày ghi chú này). Khi chạy xong, ghi kết quả vào đây.
- `gpio_request` chưa kiểm tra giá trị trả về trong code.
- `pr_info` trong handler in trạng thái LED **cũ** (trước khi đổi).

## 8. Từ vựng

| English | Tiếng Việt |
|---|---|
| pinmux (pin multiplexing) | chọn chức năng cho chân |
| pull-up resistor | điện trở kéo lên |
| falling edge | cạnh xuống (1 xuống 0) |
| rising edge | cạnh lên (0 lên 1) |
| device tree | cây thiết bị (file mô tả phần cứng) |
| overlay | lớp phủ cấu hình |
| debounce | chống dội phím |
| persistent | giữ lại sau khi khởi động lại |
