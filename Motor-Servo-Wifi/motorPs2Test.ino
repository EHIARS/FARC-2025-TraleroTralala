#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>

// PS2 controller SPI pins
#define PS2_DAT 12
#define PS2_CMD 13
#define PS2_SEL 15
#define PS2_CLK 14

#define MAX_PWM 4095

// Motor pin mapping (each motor: DIR pin + PWM channel on PCA9685)
int motor1[2] = {8, 9};   // DIR pin 8, PWM channel 9
int motor2[2] = {10, 11}; // DIR pin 10, PWM channel 11
int motor3[2] = {12, 13}; // DIR pin 12, PWM channel 13

PS2X ps2x;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setupPWMServoDriver() {
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  Wire.setClock(400000);
}

uint16_t mapSpeedToPWM(int speed) {
  speed = constrain(abs(speed), 0, 255);
  return map(speed, 0, 255, 0, MAX_PWM);
}

void motorControl(int motor[2], int speed) {
  bool direction = (speed >= 0);
  int pwmVal = mapSpeedToPWM(speed);
  if (direction) {
    pwm.setPin(motor[1], pwmVal); // forward
    pwm.setPin(motor[0], 0);
  } else {
    pwm.setPin(motor[0], pwmVal); // reverse
    pwm.setPin(motor[1], 0);
  }
}

void setup() {
  Serial.begin(115200);
  setupPWMServoDriver();

  // Set DIR pins as OUTPUT
  pinMode(motor1[0], OUTPUT);
  pinMode(motor2[0], OUTPUT);
  pinMode(motor3[0], OUTPUT);

  int error;
  bool pressures = false;
  bool rumble = false;

  while (true) {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    if (error == 0) {
      Serial.println("PS2 controller connected");
      break;
    } else {
      Serial.println("Failed to connect to PS2 controller. Retrying...");
      delay(1000);
    }
  }

  byte type = ps2x.readType();
  switch (type) {
    case 0: Serial.println("Unknown controller type"); break;
    case 1: Serial.println("DualShock controller detected"); break;
    case 2: Serial.println("GuitarHero controller detected"); break;
    case 3: Serial.println("Wireless DualShock controller detected"); break;
  }
}

void loop() {
  ps2x.read_gamepad();

  // Joystick axes
  float vx = ps2x.Analog(PSS_LX) - 128;
  float vy = 128 - ps2x.Analog(PSS_LY);  // Inverted
  float rotation = ps2x.Analog(PSS_RX) - 128;

  // Omniwheel kinematics (Motor1 at 90°, Motor2 at -30°, Motor3 at 150°)
  float m1 = vy + rotation;
  float m2 = 0.866 * vx - 0.5 * vy + rotation;
  float m3 = -0.866 * vx - 0.5 * vy + rotation;

  // Normalize if needed
  float maxVal = max(max(abs(m1), abs(m2)), abs(m3));
  if (maxVal > 127) {
    m1 *= 127.0 / maxVal;
    m2 *= 127.0 / maxVal;
    m3 *= 127.0 / maxVal;
  }

  // Apply motor commands
  motorControl(motor1, m1 * 2);  // scale to -255..255
  motorControl(motor2, m2 * 2);
  motorControl(motor3, m3 * 2);

  delay(20);
}
