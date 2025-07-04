#ifndef MOTOR_SERVO_PS2_H  // Add this line at the top
#define MOTOR_SERVO_PS2_H
#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>

#define SERVO_CW   2400     // Quay thuận
#define SERVO_CCW  600     // Quay ngược

#define SERVO_1_CHANNEL 2
#define SERVO_2_CHANNEL 3
#define SERVO_3_CHANNEL 4
#define SERVO_4_CHANNEL 5
#define SERVO_5_CHANNEL 6
#define SERVO_6_CHANNEL 7

#define MAX_PWM 4059
#define pressures false
#define rumble false

#define PS2_DAT 12
#define PS2_CMD 13
#define PS2_SEL 15
#define PS2_CLK 14
#define MAX_PWM 4059

#define CCW 3045
#define CW 1015
// Declare variables as extern (they will be defined in a .cpp file)
extern int motor1[2];
extern int motor2[2]; 
extern int motor3[2];
extern int motor4[2];
extern PS2X ps2x;
extern Adafruit_PWMServoDriver pwm;

// Control parameters
extern float speedScale;
extern float maxSpeedScale;
extern float rampUpStep;
extern float rampDownStep;
extern float rampDownFastStep;
extern unsigned long rampInterval;
extern float sinr;
extern float cosr;
extern unsigned long lastSpeedUpdate;
extern float m1, m2, m3;
extern int lx, ly, rx, slider, servo1, servo2;


int flattenAnalog(int value, int threshold = 10) {
  if (abs(value) < threshold) return 0;
  return value;
}

int limitInput(int value, float scale) {
  int maxInput = 128 * scale;
  if (value > maxInput) return maxInput;
  if (value < -maxInput) return -maxInput;
  return value;
}

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

void readGamePad() {
  if (!ps2x.Button(PSB_PAD_UP) && !ps2x.Button(PSB_PAD_DOWN)){
    slider = 0;
    pwm.setPin(motor4[0], 0);
    pwm.setPin(motor4[1], 0);
  }
  else if (ps2x.Button(PSB_PAD_UP) || ps2x.Button(PSB_PAD_DOWN)){
    if (ps2x.ButtonPressed(PSB_PAD_UP) || ps2x.ButtonPressed(PSB_PAD_DOWN)) speedScale = 0;
    if (ps2x.Button(PSB_PAD_UP) && ps2x.Button(PSB_PAD_DOWN)) slider =2;
    if (ps2x.Button(PSB_PAD_UP)) slider = 1;
    if (ps2x.Button(PSB_PAD_DOWN)) slider = -1;
  }
  else {
    updateSpeedScale();
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
  if (ps2x.Button(PSB_PINK)){
    servo1 = 1;
  }
  if (ps2x.Button(PSB_GREEN)){
    servo1 = -1;
  }
  if (!ps2x.Button(PSB_PINK) && !ps2x.Button(PSB_GREEN)){
    servo1 = 0;
    pwm.setPin(SERVO_1_CHANNEL,MAX_PWM/2);
  }

  if (ps2x.Button(PSB_BLUE)){
    servo2 = 1;
  }
  if (ps2x.Button(PSB_RED)){
    servo2 = -1;
  }
  if (!ps2x.Button(PSB_BLUE) && !ps2x.Button(PSB_RED)){
    servo2 = 0;
    pwm.setPin(SERVO_2_CHANNEL,MAX_PWM/2);
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
  if (slider >= 1) {
    pwm.setPin(motor4[0],slider*MAX_PWM/2);
    pwm.setPin(motor4[1],0);
  }
  if (slider == -1){
    pwm.setPin(motor4[1],MAX_PWM/2);
    pwm.setPin(motor4[0],0);
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

void controlServo(){
  switch (servo1){
  case 1:
    pwm.setPin(SERVO_1_CHANNEL, CW);
    break;
  case -1:
    pwm.setPin(SERVO_1_CHANNEL, CCW);
    break;
  }
  switch (servo2){
  case 1:
    pwm.setPin(SERVO_2_CHANNEL, CW);
    break;
  case -1:
    pwm.setPin(SERVO_2_CHANNEL, CCW);
    break;
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
#endif