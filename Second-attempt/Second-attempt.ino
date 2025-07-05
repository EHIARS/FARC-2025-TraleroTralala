#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>

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

#define CCW 2400
#define CW 600

int motor2[2]; 
int motor1[2];
int motor3[2];
int motor4[2];
PS2X ps2x;
Adafruit_PWMServoDriver pwm;

float sliderSpeed = 0;
float sliderMaxSpeed = 1.0;
float sliderRampUp = 0.05;
float sliderRampDown = 0.02;
unsigned long lastSliderUpdate = 0;
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
unsigned long sliderHoldStart = 0;
bool sliderHoldActive = false;
int rawLX = 0, rawLY = 0, rawRX = 0; // <-- Di chuyển khai báo này ra ngoài


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // Initialize components
  setupPS2x();
  setupPWMServoDriver();
  //setupWiFi();
  setServoAngle(SERVO_1_CHANNEL,100);
}

void loop() {
  // put your main code here, to run repeatedly:
  ps2x.read_gamepad();
  readGamePad();
  controlMotor();
  controlServo();
}

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
void updateSpeedScale(bool joystickActive) {
  unsigned long now = millis();
  if (now - lastSpeedUpdate >= rampInterval) {
    if (joystickActive) {
      speedScale += rampUpStep;
      if (speedScale > maxSpeedScale) speedScale = maxSpeedScale;
    } else {
      speedScale -= rampDownStep;
      if (speedScale < 0) speedScale = 0;
    }
    lastSpeedUpdate = now;
  }
}
void updateSliderSpeed(bool isSliderActive) {
  unsigned long now = millis();
  if (now - lastSliderUpdate >= rampInterval) {
    if (isSliderActive) {
      sliderSpeed += sliderRampUp;
      if (sliderSpeed > sliderMaxSpeed) sliderSpeed = sliderMaxSpeed;
    } else {
      sliderSpeed -= sliderRampDown;
      if (sliderSpeed < 0) sliderSpeed = 0;
    }
    lastSliderUpdate = now;
  }
}

void readGamePad() {
  bool joystickActive = false;
  if (!ps2x.Button(PSB_PAD_UP) && !ps2x.Button(PSB_PAD_DOWN)){
    slider = 0;
    rawLX = ps2x.Analog(PSS_LX) - 128;
    rawLY = 128 - ps2x.Analog(PSS_LY);
    rawRX = ps2x.Analog(PSS_RX) - 128;

    if (abs(rawLX) > 10 || abs(rawLY) > 10 || abs(rawRX) > 10) {
      joystickActive = true;
    }

    updateSpeedScale(joystickActive);  // 🚨 Gọi với trạng thái joystick

    rawLX = limitInput(rawLX, speedScale);
    rawLY = limitInput(rawLY, speedScale);
    rawRX = limitInput(rawRX, speedScale);

    lx = flattenAnalog(rawLX) * speedScale;
    ly = flattenAnalog(rawLY) * speedScale;
    rx = flattenAnalog(rawRX) * speedScale;
    }
    if ((lx * rawLX < 0) || (ly * rawLY < 0)) {
      speedScale -= 0.2 * ((millis() - lastSpeedUpdate) / 50.0);
;
      if (speedScale < 0) speedScale = 0;
  }

  // Các phần còn lại giữ nguyên:
  else //(ps2x.Button(PSB_PAD_UP) || ps2x.Button(PSB_PAD_DOWN)){
  { 
    lx=0;ly=0,rx=0;
    //if (ps2x.ButtonPressed(PSB_PAD_UP) || ps2x.ButtonPressed(PSB_PAD_DOWN)) speedScale = 0;
    if (ps2x.Button(PSB_PAD_UP) && ps2x.Button(PSB_PAD_DOWN)) slider =2;
    else{
      bool isSliderActive = false;
      if (ps2x.Button(PSB_PAD_UP)) {
        slider = 1;
        isSliderActive = true;
      }
      if (ps2x.Button(PSB_PAD_DOWN)) {
        slider = -1;
        isSliderActive = true;
      }
      updateSliderSpeed(isSliderActive);
    }
  }

  Serial.print("SpeedScale: "); Serial.println(speedScale);
  Serial.print("LX: "); Serial.println(lx);
  Serial.print("LY: "); Serial.println(ly);
  Serial.print("RX: "); Serial.println(rx);
}

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
  int sliderPWM = map(sliderSpeed * 100, 0, 100, 0, MAX_PWM);
  if (slider == 2) {
  if (!sliderHoldActive) {
    sliderHoldStart = millis();
    sliderHoldActive = true;
    pwm.setPin(motor4[1], MAX_PWM);
    pwm.setPin(motor4[0], 0);
  } else if (millis() - sliderHoldStart > 5000) {
    pwm.setPin(motor4[0], 0);
    pwm.setPin(motor4[1], 0);
    sliderHoldActive = false;
    slider = 0;
  }
  }
  else if (slider == 1 && !digitalRead(36)) {
    pwm.setPin(motor4[0], sliderPWM);
    pwm.setPin(motor4[1], 0);
  }
  else if (slider == -1) {
    pwm.setPin(motor4[1], sliderPWM);
    pwm.setPin(motor4[0], 0);
  }
  else {
    pwm.setPin(motor4[0], 0);
    pwm.setPin(motor4[1], 0);
  }

  if (slider != 2 && sliderHoldActive) {
    sliderHoldActive = false;
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
void setServoAngle(uint8_t channel, float angle) {
  // Giới hạn góc trong khoảng 0 - 180
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;

  // Chuyển đổi góc thành độ rộng xung (ms)
  float pulseMs = 0.5 + (angle / 180.0) * 2.0;  // từ 0.5ms đến 2.5ms

  // Chuyển sang giá trị PWM tương ứng
  int pwmVal = (int)(pulseMs * 1000.0 / (1000000.0 / 50.0 / 4096.0));  // hoặc pulseMs * 4096 / 20

  pwm.setPWM(channel, 0, pwmVal);
}
void controlServo(){
  if (ps2x.Button(PSB_PINK)){
    setServoAngle(SERVO_1_CHANNEL,70);
  }
  else setServoAngle(SERVO_1_CHANNEL,100);
}

void setupPWMServoDriver() {
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  Wire.setClock(400000);
  pinMode(36,INPUT_PULLDOWN);// encoder
}

// Kết nối tay cầm PS2
void setupPS2x() {
  Serial.print("Ket noi voi tay cam PS2:");
  int error = -1;

  for (int i = 0; i < 10; i++) {
    delay(100);
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
