#include <Wire.h> //thư viện I2c của Arduino, do PCA9685 sử dụng chuẩn giao tiếp i2c nên thư viện này bắt buộc phải khai báo 
#include <Adafruit_PWMServoDriver.h> // thư viện PCA9685
//server
#include <WiFi.h>
#include <WebServer.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
const char* ssid = "FARC Tralalero Tralala";
const char* password = "12345678";


//motor pin define
int motor1[2] = {8,9};
int motor2[2] = {10,11};
int motor3[2] = {12,13};
int motor4[2] = {14,15};

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

//control motor
void motorControl(int motor[2], int speed, bool direction){
 if (direction=1) {
 pwm.setPin(motor[1], speed);
 pwm.setPin(motor[2], 0);
 } else {
 pwm.setPin(motor[2], speed);
 pwm.setPin(motor[1], 0);
 }
 
}

void setup() {
  setupPWMServoDriver();
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  motorControl(motor1, 4000, 1);
  /*
   if (Serial.available()){
      int speed = int(Serial.read());
      motorControl(motor1, speed, 1);
      
  }*/
 
}
