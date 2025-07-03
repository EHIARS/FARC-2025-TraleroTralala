/*
#include <Wire.h> //thư viện I2c của Arduino, do PCA9685 sử dụng chuẩn giao tiếp i2c nên thư viện này bắt buộc phải khai báo 
#include <Adafruit_PWMServoDriver.h> // thư viện PCA9685
#include <PS2X_lib.h>

#define PS2_DAT 12 // MISO 
#define PS2_CMD 13 // MOSI 
#define PS2_SEL 15 // SS 
#define PS2_CLK 14 // SLK
#define MAX_PWM 4059
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false
//motor pin define
int motor1[2] = {8,9};
int motor2[2] = {10,11};
int motor3[2] = {12,13};
int motor4[2] = {14,15};

PS2X ps2x; // khởi tạo class PS2x

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const float sinr = 0.86602540378;

// Xử lý di chuyển
float speedScale = 0.0;
const float maxSpeedScale = 2.0;
const float rampUpStep = 0.05;
const float rampDownStep = 0.03;
const float rampDownFastStep = 0.1;
unsigned long lastSpeedUpdate = 0;
const unsigned long rampInterval = 50;
int limitInput(int value, float scale) {
  int maxInput = 32 * scale;
  if (value > maxInput) return maxInput;
  if (value < -maxInput) return -maxInput;
  return value;
}

int flattenAnalog(int value, int threshold = 10) {
  if (abs(value) < threshold) return 0;  // Trong vùng chết ⇒ bỏ qua
  return value;
}
void updateSpeedScale() {
  bool r1 = ps2x.Button(PSB_R1);
  bool l1 = ps2x.Button(PSB_L1);
  unsigned long now = millis();

  if (now - lastSpeedUpdate >= rampInterval) {
    if (r1) {
      // Tăng dần khi giữ R1
      speedScale += rampUpStep;
      if (speedScale > maxSpeedScale) speedScale = maxSpeedScale;
    }
    else if (l1) {
      // Giảm nhanh khi giữ L1
      speedScale -= rampDownFastStep;
      if (speedScale < 0) speedScale = 0;
    }
    else {
      // Giảm chậm khi thả R1
      speedScale -= rampDownStep;
      if (speedScale < 0) speedScale = 0;
    }

    lastSpeedUpdate = now;
  }
}



//setup library
void setupPWMServoDriver(){
  pwm.begin(); //khởi tạo PCA9685 
  // cài đặt tần số dao động 
  pwm.setOscillatorFrequency(27000000); 
  // cài đặt tần số PWM. Tần số PWM có thể được cài đặt trong khoảng 24-1600 HZ, tần số này được cài đặt tùy thuộc vào nhu cầu xử dụng. Để điều khiển được cả servo và động cơ DC cùng nhau, tần số PWM điều khiển được cài đặt trong khoảng 50-60Hz.
  pwm.setPWMFreq(50);
  // cài đặt tốc độ giao tiếp i2c ở tốc độ cao nhất(400 Mhz). Hàm này có thể bỏ qua nếu gặp lỗi hoặc không có nhu cầu tử dụng I2c tốc độ cao
  Wire.setClock(400000); 
}

void setupPS2x(){
  Serial.print("Ket noi voi tay cam PS2:");

  int error = -1;
  for (int i = 0; i < 10; i++) // thử kết nối với tay cầm ps2 trong 10 lần
  {
    delay(1000); // đợi 1 giây
    // cài đặt chân và các chế độ: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    Serial.print(".");
  }
  Serial.println(error);
  switch (error) // kiểm tra lỗi nếu sau 10 lần không kết nối được
  {
  case 0:
    Serial.println(" Ket noi tay cam PS2 thanh cong");
    break;
  case 1:
    Serial.println(" LOI: Khong tim thay tay cam, hay kiem tra day ket noi vơi tay cam ");
    break;
  case 2:
    Serial.println(" LOI: khong gui duoc lenh");
    break;
  case 3:
    Serial.println(" LOI: Khong vao duoc Pressures mode ");
    break;
  }
}
int lx=0, ly=0, rx=0;
void readGamePad(){
  updateSpeedScale();  // xử lý tốc độ mềm

  int rawLX = ps2x.Analog(PSS_LX) - 128;
  int rawLY = 128 - ps2x.Analog(PSS_LY);
  int rawRX = ps2x.Analog(PSS_RX) - 128;

  // Giới hạn biên độ theo speedScale
  rawLX = limitInput(rawLX, speedScale);
  rawLY = limitInput(rawLY, speedScale);
  rawRX = limitInput(rawRX, speedScale);

  // Áp dụng scale
  lx = flattenAnalog(rawLX) * speedScale;
  ly = flattenAnalog(rawLY) * speedScale;
  rx = flattenAnalog(rawRX) * speedScale;
  if ((lx * rawLX < 0) || (ly * rawLY < 0)) {
  // Nếu đổi hướng (nhân 2 số trái dấu)
  speedScale -= 0.2;  // giảm nhẹ tốc độ
  if (speedScale < 0) speedScale = 0;
  }
  Serial.print("SpeedScale: "); Serial.println(speedScale);
  Serial.print("LX: "); Serial.println(lx);
  Serial.print("LY: "); Serial.println(ly);
  Serial.print("RX: "); Serial.println(rx);
}  
unsigned long Ptime = millis();
void controlMotor(){
  // Tính tốc độ động cơ theo hướng
  float m1 = lx + rx;
  float m2 = -0.5 * lx + sinr * ly + rx;
  float m3 = -0.5 * lx - sinr * ly + rx;

  // ✅ Chuẩn hóa nếu vượt quá ±255
  float maxVal = max(max(abs(m1), abs(m2)), abs(m3));
  if (maxVal > 255) {
    m1 *= 255.0 / maxVal;
    m2 *= 255.0 / maxVal;
    m3 *= 255.0 / maxVal;
  }

  // Điều khiển động cơ 1
  if (m1 > 0) {
    pwm.setPin(motor1[1], map(abs(m1), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor1[0], 0);
  } else {
    pwm.setPin(motor1[0], map(abs(m1), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor1[1], 0);
  }

  // Điều khiển động cơ 2
  if (m2 > 0) {
    pwm.setPin(motor2[1], map(abs(m2), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor2[0], 0);
  } else {
    pwm.setPin(motor2[0], map(abs(m2), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor2[1], 0);
  }

  // Điều khiển động cơ 3
  if (m3 > 0) {
    pwm.setPin(motor3[1], map(abs(m3), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor3[0], 0);
  } else {
    pwm.setPin(motor3[0], map(abs(m3), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor3[1], 0);
  }
}
void setup(){
  Serial.begin(115200);
  setupPS2x();
  setupPWMServoDriver();
}
void loop(){
  ps2x.read_gamepad();
  readGamePad();
  controlMotor();
  delay(50);
}
*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>
#include <Server-adjustion.h>

#define PS2_DAT 12
#define PS2_CMD 13
#define PS2_SEL 15
#define PS2_CLK 14

#define SERVO_1_CHANNEL 2
#define SERVO_2_CHANNEL 3
#define SERVO_3_CHANNEL 4
#define SERVO_4_CHANNEL 5
#define SERVO_5_CHANNEL 6
#define SERVO_6_CHANNEL 7

#define MAX_PWM 4059
#define pressures false
#define rumble false

int motor1[2] = {8, 9};
int motor2[2] = {10, 11};
int motor3[2] = {12, 13};
int motor4[2] = {14,15};

PS2X ps2x;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Biến xử lý tốc độ
float speedScale = 1.0;
float maxSpeedScale = 2.0;
float rampUpStep = 0.05;
float rampDownStep = 0.03;
float rampDownFastStep = 0.1;
unsigned long rampInterval = 50;

float sinr = 0.86602540378;
float cosr = 0.5;
unsigned long lastSpeedUpdate = 0;
float m1 = 0.0, m2 = 0.0, m3 = 0.0;

int lx = 0, ly = 0, rx = 0, slider = 0;

// Hàm làm phẳng đầu vào analog
int flattenAnalog(int value, int threshold = 10) {
  if (abs(value) < threshold) return 0;
  return value;
}

// Giới hạn biên độ theo scale
int limitInput(int value, float scale) {
  int maxInput = 128 * scale;
  if (value > maxInput) return maxInput;
  if (value < -maxInput) return -maxInput;
  return value;
}

// Cập nhật tốc độ mềm
void updateSpeedScale() {
  bool r1 = ps2x.Button(PSB_R1);
  bool l1 = ps2x.Button(PSB_L1);
  unsigned long now = millis();

  if (now - lastSpeedUpdate >= rampInterval) {
    if (r1) {
      speedScale += rampUpStep;
      if (speedScale > maxSpeedScale) speedScale = maxSpeedScale;
    }
    else if (l1) {
      speedScale -= rampDownFastStep;
      if (speedScale < 0) speedScale = 0;
    }
    else {
      speedScale -= rampDownStep;
      if (speedScale < 0) speedScale = 0;
    }

    lastSpeedUpdate = now;
  }
}

// Đọc và xử lý tay cầm
void readGamePad() {
  updateSpeedScale();  
  if (ps2x.ButtonReleased(PSB_PAD_UP) || ps2x.ButtonReleased(PSB_PAD_DOWN)){
    slider = 0;
    speedScale = 0;
  }
  if (ps2x.Button(PSB_PAD_UP) || ps2x.Button(PSB_PAD_DOWN)){
    slider = 128 * speedScale;
    slider = limitInput(slider,speedScale);
    lx = 0;
    ly = 0;
    rx = 0;
  }
  else{
  int rawLX = ps2x.Analog(PSS_LX) - 128;
  int rawLY = 128 - ps2x.Analog(PSS_LY);
  int rawRX = ps2x.Analog(PSS_RX) - 128;

  rawLX = limitInput(rawLX, speedScale);
  rawLY = limitInput(rawLY, speedScale);
  rawRX = limitInput(rawRX, speedScale);

  lx = flattenAnalog(rawLX) * speedScale;
  ly = flattenAnalog(rawLY) * speedScale;
  rx = flattenAnalog(rawRX) * speedScale;

  // Giảm tốc nếu đổi hướng đột ngột
  if ((lx * rawLX < 0) || (ly * rawLY < 0)) {
    speedScale -= 0.2;
    if (speedScale < 0) speedScale = 0;
  }
  }
  Serial.print("SpeedScale: "); Serial.println(speedScale);
  Serial.print("LX: "); Serial.println(lx);
  Serial.print("LY: "); Serial.println(ly);
  Serial.print("RX: "); Serial.println(rx);
}

// Điều khiển động cơ
void controlMotor() {
  m1 = lx + rx;
  m2 = -cosr * lx + sinr * ly + rx;
  m3 = -cosr * lx - sinr * ly + rx;

  // Chuẩn hóa m1~m3 về ±255
  float maxVal = max(max(abs(m1), abs(m2)), abs(m3));
  if (maxVal > 255) {
    m1 *= 255.0 / maxVal;
    m2 *= 255.0 / maxVal;
    m3 *= 255.0 / maxVal;
  }
  Serial.print("m1: "); Serial.println(m1);
  Serial.print("m2: "); Serial.println(m2);
  Serial.print("m3: "); Serial.println(m3);
  // Motor 1
  if (ps2x.Button(PSB_PAD_UP)){
    pwm.setPin(motor4[0], map(slider, 0, 255, 0, MAX_PWM));
    pwm.setPin(motor4[1], 0);
  } else if (ps2x.Button(PSB_PAD_DOWN)){
    pwm.setPin(motor4[1], map(slider, 0, 255, 0, MAX_PWM));
    pwm.setPin(motor4[0], 0);
  }
  if (m1 > 0) {
    pwm.setPin(motor1[1], map(abs(m1), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor1[0], 0);
  } else {
    pwm.setPin(motor1[0], map(abs(m1), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor1[1], 0);
  }

  // Motor 2
  if (m2 > 0) {
    pwm.setPin(motor2[1], map(abs(m2), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor2[0], 0);
  } else {
    pwm.setPin(motor2[0], map(abs(m2), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor2[1], 0);
  }

  // Motor 3
  if (m3 > 0) {
    pwm.setPin(motor3[1], map(abs(m3), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor3[0], 0);
  } else {
    pwm.setPin(motor3[0], map(abs(m3), 0, 255, 0, MAX_PWM));
    pwm.setPin(motor3[1], 0);
  }
}

// Khởi tạo PCA9685
void setupPWMServoDriver() {
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  Wire.setClock(400000);
}

// Kết nối tay cầm PS2
void setupPS2x() {
  Serial.print("Ket noi voi tay cam PS2:");
  int error = -1;

  for (int i = 0; i < 10; i++) {
    delay(1000);
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    Serial.print(".");
  }

  Serial.println(error);
  switch (error) {
    case 0: Serial.println(" Ket noi tay cam PS2 thanh cong"); break;
    case 1: Serial.println(" LOI: Khong tim thay tay cam"); break;
    case 2: Serial.println(" LOI: Khong gui duoc lenh"); break;
    case 3: Serial.println(" LOI: Khong vao duoc Pressure mode"); break;
  }
}

void setup() {
  Serial.begin(115200);
  setupPS2x();
  setupPWMServoDriver();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("✅ WiFi connected");
  Serial.println(WiFi.localIP());
  setupAdjustmentServer();
}

void loop() {
  handleAdjustment();
  ps2x.read_gamepad();
  readGamePad();
  controlMotor();
  delay(50);
}
