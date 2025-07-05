

#include <Wire.h>
//#include "Server-adjustion.h"
#include "motorServoPs2.h"

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

// Define the variables (actual storage)
int motor1[2] = {8, 9};
int motor2[2] = {10, 11};
int motor3[2] = {12, 13};
int motor4[2] = {14, 15};
PS2X ps2x;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Control parameters
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
int lx = 0, ly = 0, rx = 0, slider = 0, servo1 =0, servo2 = 0;
void setup() {
  Serial.begin(115200);
  
  // Initialize components
  setupPS2x();
  setupPWMServoDriver();
  //setupWiFi();
  
  // Start server after short delay
  delay(1000);
  //setupAdjustmentServer();
  
  Serial.println("System initialized");
}

void loop() {
  //handleAdjustment();
  ps2x.read_gamepad();
  readGamePad();
  controlMotor();
  controlServo();
  //delay(50);
}