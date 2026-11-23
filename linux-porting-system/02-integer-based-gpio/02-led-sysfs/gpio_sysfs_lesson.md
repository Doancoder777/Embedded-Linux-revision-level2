# BUỔI HỌC: GPIO SUBSYSTEM + SYSFS (BeagleBone Black)

## 1. Các khái niệm

* **GPIO Subsystem**
  * `integer based`: Phương pháp sử dụng số nguyên điều khiển GPIO (cách cũ, legacy, deprecated).
  * `descriptor based`: Phương pháp dùng device tree (cách mới, được khuyến nghị).
* **Pinctrl Subsystem**: Quản lý việc cấu hình chân (pin mux) — một chân vật lý có thể làm GPIO, UART, I2C, SPI...
* **Linux Device Model**: Mô hình thiết bị của Linux (`device`, `driver`, `bus`, `class`).
* **Sysfs (System file system)**: Hệ thống file ảo, cho phép user-space xem/điều khiển thiết bị từ kernel qua các file trong `/sys`.

---

## 2. Ghi chú về GPIO Subsystem

* **Về phần cứng**: GPIO là một chân có thể hoạt động ở chế độ `input` hoặc `output`.
* **Về phần mềm**: GPIO là một đường số (digital line) chỉ có 2 giá trị: `1` (high) hoặc `0` (low).
* Trước khi dùng GPIO, phải **"claim"** (xin quyền sở hữu) với kernel để driver khác không truy cập cùng chân.
* Sau khi sở hữu, ta có thể:
  * Đặt `direction` (chiều: `in` / `out`)
  * Điều khiển mức output (`high` / `low`)
  * Đọc trạng thái nếu là input
  * Cấu hình ngắt (IRQ) theo cạnh/mức (`edge` / `level`)

### Hai cách làm việc với GPIO trong kernel

1. **Integer-based interface** (legacy, deprecated):
   ```c
   #include <linux/gpio.h>

   gpio_request(gpio, label);
   gpio_free(gpio);
   gpio_is_valid(number);
   gpio_direction_input(gpio);
   gpio_direction_output(gpio, value);
   gpio_set_debounce(gpio, debounce);   // debounce tính bằng ms
   ```
2. **Descriptor-based interface** (mới, khuyến nghị).

---

## 3. Điều khiển GPIO59 qua Sysfs

> Công thức tính: $\text{GPIO59} = \text{GPIO1\_27}$, vì $32 \times 1 + 27 = 59$.

### Các bước thực hiện
1. **Export the pin**: Đưa chân ra user-space.
2. **Set the pin direction**: Thiết lập chiều `input` hoặc `output`.
3. **Output**: Nếu là chân output, thiết lập mức `low` hoặc `high`.
4. **Input**: Nếu là chân input, đọc mức của chân (`low` hoặc `high`).
5. **Unexport**: Trả chân lại cho kernel khi dùng xong.

### Lệnh thực hành

```bash
# Bước 1: export và kiểm tra
echo 59 > /sys/class/gpio/export
ls /sys/class/gpio
ls /sys/class/gpio/gpio59/

# Bước 2: đặt direction
echo in  > /sys/class/gpio/gpio59/direction
echo out > /sys/class/gpio/gpio59/direction

# Bước 3: ghi mức output (khi direction = out)
echo 0 > /sys/class/gpio/gpio59/value
echo 1 > /sys/class/gpio/gpio59/value

# Bước 4: đọc mức input (khi direction = in)
cat /sys/class/gpio/gpio59/value

# Bước 5: unexport khi xong
echo 59 > /sys/class/gpio/unexport
```

---

## 3B. Cấu hình chân bằng `config-pin` (Pinmux / Pull-up)

**Vì sao cần:** Chân BeagleBone Black có thể mặc định bị kéo xuống (`pull-down`) hoặc đang ở chức năng khác (UART, I2C...). Khi đó nút bấm nối xuống GND sẽ luôn đọc ra `0`, bấm hay không cũng như nhau.

**Ví dụ:** Nút bấm nối chân `P8_14` (`gpio26` = `GPIO0_26`) xuống GND.

```bash
# Bật điện trở kéo lên (pull-up) cho P8_14
sudo config-pin P8_14 gpio_pu

# Xem chế độ hiện tại của chân
config-pin -q P8_14
# -> Current mode for P8_14 is: gpio_pu

# Kiểm tra mức chân trong sysfs
cat /sys/class/gpio/gpio26/value
# -> Không bấm nút: 1
# -> Giữ nút (nối GND): 0

# Đọc liên tục để thấy phản ứng khi bấm/nhả (thoát: Ctrl+C)
while true; do cat /sys/class/gpio/gpio26/value; sleep 0.3; done

# Xem trạng thái tất cả GPIO (in hi = mức cao, in lo = mức thấp)
sudo grep "gpio-26" /sys/kernel/debug/gpio
```

### Các chế độ thường dùng của `config-pin`
* `gpio`: GPIO bình thường (không kéo)
* `gpio_pu`: GPIO có pull-up (kéo lên)
* `gpio_pd`: GPIO có pull-down (kéo xuống)

Nếu chân đang ở chức năng khác (ví dụ `P9_13` mặc định là `UART4_TXD`):
```bash
sudo config-pin P9_13 gpio
```

> **LƯU Ý QUAN TRỌNG:**
> * `config-pin` chỉ có tác dụng **TẠM THỜI**. Reboot là mất, phải chạy lại sau mỗi lần khởi động.
> * Cách cố định là cấu hình pinmux trong Device Tree (sẽ học sau).
> * **Cảnh báo phần cứng:** CHỈ nối chân vào **GND** hoặc **3.3V**. TUYỆT ĐỐI không nối vào **5V**.

### Tạo script chạy nhanh sau mỗi lần boot

```bash
cat > ~/setup_pins.sh << 'EOF'
#!/bin/bash
sudo config-pin P8_14 gpio_pu
sudo config-pin P9_13 gpio
EOF

chmod +x ~/setup_pins.sh
~/setup_pins.sh
```

### Cách đọc kết quả khi thử nút bấm
| Trạng thái đọc được | Đánh giá & Nguyên nhân |
| :--- | :--- |
| Nhả ra `1`, giữ ra `0` | **Đúng**, mọi thứ ổn. |
| Nhả ra `0`, giữ ra `0` | Nút luôn thông (đấu sai cặp chân, xoay nút $90^\circ$, đấu hai chân chéo nhau). |
| Nhả ra `1`, giữ vẫn `1` | Nút hở hoặc chưa nối xuống GND. |
| Nhảy lung tung `0 1 0 1` | Dây lỏng hoặc nút bị nảy (bounce). |

---

## 3C. Thực hành: Bật / Tắt LED (`gpio31` = `P9_13`)

**Phần cứng:** LED đấu vào `P9_13`, nối tiếp điện trở khoảng $330\ \Omega$, chân dài (anode) về phía `P9_13`.

```bash
# Bước 0: đảm bảo chân ở chế độ GPIO (mặc định là UART4_TXD)
sudo config-pin P9_13 gpio
config-pin -q P9_13
# -> Current mode for P9_13 is: gpio

# Bước 1: vào thư mục chân (gpio31 do kernel export sẵn, không cần echo 31 > export)
cd /sys/class/gpio/gpio31

# Bước 2: đặt direction = out (BẮT BUỘC trước khi ghi value)
cat direction
# -> in
echo out > direction
cat direction
# -> out

# Bước 3: bật / tắt LED
echo 1 > value        # LED sáng
echo 0 > value        # LED tắt
cat value             # kiểm tra mức hiện tại

# Cách ngắn: đặt chiều và mức ban đầu cùng lúc
echo high > direction     # out, value = 1
echo low  > direction     # out, value = 0

# Cho LED nháy 5 lần
for i in 1 2 3 4 5; do
  echo 1 > value; sleep 0.5
  echo 0 > value; sleep 0.5
done
```

### Ghép nút bấm (`gpio26`) với LED (`gpio31`)

```bash
# Nhấn nút -> LED đổi (cần đã chạy: sudo config-pin P8_14 gpio_pu)
while true; do
  cat /sys/class/gpio/gpio26/value > /sys/class/gpio/gpio31/value
  sleep 0.05
done

# Vì pull-up: nhả nút = 1, nhấn nút = 0. Muốn "nhấn thì sáng":
echo 1 > /sys/class/gpio/gpio26/active_low
```

---

## Lỗi đã gặp và cách hiểu

| Lỗi / Hiện tượng | Nguyên nhân / Cách xử lý |
| :--- | :--- |
| `echo 0 > value` <br> `-> Operation not permitted` | Chân đang ở chế độ `"in"`. Chạy: `echo out > direction` |
| `sudo echo 0 > value` <br> `-> vẫn lỗi` | Dấu `>` do shell của bạn thực hiện, không phải `sudo`. <br> Dùng: `echo 0 \| sudo tee value` |
| `echo 117 > unexport` <br> `-> Invalid argument` | Chân do kernel export lúc boot, không unexport được. Bỏ qua. |
| `config-pin -q P8_14 gpio_pu` <br> `-> in ra usage` | Sai cú pháp. `-q` chỉ để XEM. Đặt chế độ: `config-pin P8_14 gpio_pu` |
| `config-pin -q P0_26` <br> `-> in ra usage` | Sai tên chân. Dùng tên header (`P8_14`), không dùng tên GPIO. |
| `cat value` luôn `= 0` khi bấm nút | Chân mặc định pull-down. Bật pull-up: `config-pin P8_14 gpio_pu` |
| `config-pin -q` ra `"default"` | Chân đang ở cấu hình mặc định lúc boot, chưa bị đổi. |
| Ảnh chụp bị cắt ký tự đầu dòng (`ebian`, `ctive_low`, `n`) | Chỉ là lỗi chụp màn hình, không phải lỗi lệnh. |

---

## Tra tên Header từ số GPIO

Không có công thức cố định. Tra bằng lệnh hoặc bảng pinout của BeagleBone Black:
```bash
sudo grep "gpio-26" /sys/kernel/debug/gpio
gpioinfo | grep "P8_"
```

| Số GPIO | Tên GPIO | Header | Ghi chú |
| :---: | :---: | :---: | :--- |
| `26` | `GPIO0_26` | `P8_14` | Nút bấm |
| `31` | `GPIO0_31` | `P9_13` | LED |
| `59` | `GPIO1_27` | `P8_17` | - |
| `60` | `GPIO1_28` | `P9_12` | - |

---

## Checklist cho lần thực hành sau (sau khi reboot)

- [ ] **1.** Board mới boot thì `config-pin` đã MẤT, phải chạy lại:
  ```bash
  sudo config-pin P8_14 gpio_pu      # nút, pull-up
  sudo config-pin P9_13 gpio         # LED
  ```
- [ ] **2.** Kiểm tra chế độ chân:
  ```bash
  config-pin -q P8_14
  config-pin -q P9_13
  ```
- [ ] **3.** Chân output: `echo out > direction` (rồi mới ghi `value`).
- [ ] **4.** Chân input: `echo in > direction` (rồi mới đọc `value`).
- [ ] **5.** Chỉ nối chân vào **GND** hoặc **3.3V**, KHÔNG nối **5V**.
- [ ] **6.** LED luôn có điện trở nối tiếp.
- [ ] **7.** Lỗi thì đọc kỹ thông báo:
  * `Operation not permitted` = sai `direction`
  * `Invalid argument` = chân của kernel
  * `Device or resource busy` = chân đang bị giữ

---

## 4. Lưu ý trên BeagleBone Black

* **Công thức tính số GPIO:**
  $$N = 32 \times (\text{số gpiochip}) + (\text{số line})$$
  *Ví dụ:* $\text{GPIO1\_28} = 32 \times 1 + 28 = 60$.
* Image Debian của BBB đã export sẵn nhiều chân `gpioN` lúc boot (do kernel). Những chân này **KHÔNG** unexport được:
  ```bash
  echo 117 > /sys/class/gpio/unexport
  # -bash: echo: write error: Invalid argument
  ```
  $\Rightarrow$ Bình thường, không phải lỗi của bạn. Chỉ unexport được chân do chính bạn export.
* Nếu export báo `Device or resource busy` $\Rightarrow$ chân đang bị kernel/driver khác giữ, hãy chọn chân khác.
* Nếu báo `Permission denied` $\Rightarrow$ dùng `sudo`:
  ```bash
  echo 59 | sudo tee /sys/class/gpio/export
  ```
* Ẩn bớt các `gpioN` có sẵn cho dễ nhìn:
  ```bash
  ls /sys/class/gpio | grep -v 'gpio[0-9]'
  ```
* `sysfs` GPIO đã **deprecated** từ kernel 4.8. Cách hiện đại là dùng **`libgpiod`**:
  ```bash
  gpiodetect
  gpioinfo gpiochip1
  gpioset gpiochip1 27=1
  ```

---

## 5. Từ vựng tiếng Anh

| Thuật ngữ | Ý nghĩa |
| :--- | :--- |
| **export / unexport** | đưa chân ra user-space / thu hồi lại |
| **direction** | chiều (`in` = input, `out` = output) |
| **value** | giá trị (`0` hoặc `1`) |
| **level** | mức (`low` / `high`) |
| **claim** | xin quyền sở hữu |
| **ownership** | quyền sở hữu |
| **debounce** | chống rung phím |
| **interrupt (IRQ)** | ngắt |
| **edge / level** | cạnh / mức |
| **legacy** | cũ, di sản |
| **deprecated** | lỗi thời, không khuyến khích dùng |
| **pinmux** | chọn chức năng cho chân (multiplexing) |
| **pull-up / pull-down** | điện trở kéo lên / kéo xuống |
| **floating** | thả nổi (không nối gì) |
| **bounce** | nảy phím |
| **temporary / persistent** | tạm thời / giữ lại sau reboot |
| **toggle** | bật tắt luân phiên |
| **blink** | nháy |
| **anode / cathode** | cực dương / cực âm của LED |
| **current-limiting resistor** | điện trở hạn dòng |
| **header** | dãy chân cắm trên board (`P8`, `P9`) |
| **pinout** | sơ đồ chân |
| **Invalid argument** | tham số không hợp lệ |
| **Permission denied** | không đủ quyền |
| **Device or resource busy** | thiết bị/tài nguyên đang bận |