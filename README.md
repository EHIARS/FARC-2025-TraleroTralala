# 🤖 Điều Khiển Robot 3 Bánh & Servo Bằng Tay Cầm PS2

Dự án này sử dụng tay cầm PS2 để điều khiển một robot sử dụng 3 bánh omni và 2 servo quay, với khả năng đọc tín hiệu analog từ joystick để điều khiển hướng và tốc độ, đồng thời điều khiển cơ cấu nâng/hạ bằng động cơ thứ 4 (`motor4`) và hai servo (`servo1`, `servo2`).

## 🎮 Bảng Thao Tác Tay Cầm PS2

| **Nhóm Điều Khiển**     | **Nút PS2**             | **Biến Liên Quan** | **Chức Năng**                                                       |
|-------------------------|--------------------------|---------------------|----------------------------------------------------------------------|
| **Motor 4 (nâng/hạ)**   | `▲ (PAD_UP)`             | `slider = 2`        | Motor4 chạy tiến (nâng vừa)                                         |
|                         | `▼ (PAD_DOWN)`           | `slider = -1`       | Motor4 chạy lùi (hạ xuống)                                          |
|                         | `▲ + ▼`                  | `slider = 4`        | Motor4 chạy tiến nhanh (tối đa 4/5 MAX_PWM)                         |
|                         | Không nhấn               | `slider = 0`        | Motor4 dừng                                                         |
|                         | (dùng kèm `digitalRead(36)`) | --              | Chỉ cho phép motor4 chạy khi encoder ở mức LOW                      |
| **Tăng/Giảm tốc độ**    | `R1`                     | `speedScale ↑`      | Tăng dần tốc độ lên tới `maxSpeedScale`                             |
|                         | `L1`                     | `speedScale ↓ nhanh`| Giảm tốc nhanh hơn                                                  |
|                         | Không nhấn R1/L1         | `speedScale ↓ chậm` | Tự động giảm tốc độ dần đều                                        |
|                         | Đổi hướng joystick       | `speedScale -= 0.2` | Giảm tốc nhẹ khi đổi hướng đột ngột                                 |
| **Robot di chuyển**     | Analog: `LX`, `LY`, `RX` | `lx`, `ly`, `rx`    | Điều khiển hướng di chuyển + quay vòng                              |
|                         |                          | `m1`, `m2`, `m3`    | Tính toán tốc độ cho từng motor (omni wheel)                        |
| **Servo thả banh**      | `△ (PINK)`               | `servo1 = 1`        | Quay thuận chiều kim đồng hồ (PWM = `CW`)                           |
|                         | `☐ (GREEN)`              | `servo1 = -1`       | Quay ngược chiều kim đồng hồ (PWM = `CCW`)                          |
|                         | Không nhấn               | `servo1 = 0`        | Dừng servo 1 (PWM = `MAX_PWM / 2`)                                  |
| **Servo thả nông sảng** | `✖ (BLUE)`               | `servo2 = 1`        | Quay thuận chiều kim đồng hồ                                        |
|                         | `○ (RED)`                | `servo2 = -1`       | Quay ngược chiều kim đồng hồ                                        |
|                         | Không nhấn               | `servo2 = 0`        | Dừng servo 2                                                        |

## ⚙️ Thông Số Kỹ Thuật & Kết Nối

- **Controller:** Tay cầm PS2
- **Main board:** ESP32
- **Motor driver:** PCA9685 (16-channel PWM)
- **PWM frequency:** 50Hz
- **Motor max PWM value:** `MAX_PWM = 4059`
- **Encoder:** `pin 36`, sử dụng `INPUT_PULLDOWN`
- **Servo PWM giá trị:**
  - `CW = 1015`
  - `CCW = 3045`
  - `Trung tâm = MAX_PWM / 2`

## 📁 Cấu Trúc Dự Án

- `main.ino` — vòng lặp điều khiển chính.
- `motor_servo_ps2.h` — chứa logic đọc tay cầm, điều khiển motor và servo.
- `Server-adjustion.h` *(nếu có)* — điều chỉnh tham số qua WiFi và web server.

## 🚀 Tính Năng Mở Rộng

- Điều chỉnh thông số qua web server `/config`
- Xem trực tiếp các giá trị `m1`, `m2`, `m3` real-time
- Có thể mở rộng thêm động cơ và servo khác nếu cần.

---

## 💡 Gợi Ý Phát Triển

- Hiển thị trạng thái các nút tay cầm qua Serial Monitor hoặc OLED
- Giao diện web để điều chỉnh `speedScale`, `sinr`, `cosr`, `rampStep`,...
- Tích hợp cảm biến khoảng cách để tránh va chạm khi điều khiển từ xa

---

> *Developed with 💙 by [YourName]*
