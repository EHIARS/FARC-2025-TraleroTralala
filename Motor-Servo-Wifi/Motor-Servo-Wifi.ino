#include <Wire.h> //thư viện I2c của Arduino, do PCA9685 sử dụng chuẩn giao tiếp i2c nên thư viện này bắt buộc phải khai báo 
#include <Adafruit_PWMServoDriver.h> // thư viện PCA9685
#include <PS2X_lib.h>
//server
#include <WiFi.h>
#include <WebServer.h>


#define PS2_DAT 12 // MISO 
#define PS2_CMD 13 // MOSI 
#define PS2_SEL 15 // SS 
#define PS2_CLK 14 // SLK
#define MAX_PWM 4059
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false
PS2X ps2x; // khởi tạo class PS2x

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
uint16_t mapSpeedToPWM(int speed) {
  speed = constrain(abs(speed), 0, 255);
  return map(speed, 0, 255, 0, MAX_PWM);
}
void motorControl(int motor[2], int speed){
 if (speed > 0){
  bool direction = 1;
 } else bool direction = 0;
 if (direction==1) {
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
  Serial.begin(115200);

//Kết nối với tay cầm bằng hàm ps2x.config_gamepad, thử kết nối lại trong vòng 10 lần nếu quá 10 lần không kết nối được với tay cầm thì sẽ dừng lại
int error = 0
unsigned long t_start = millis();
Serial.println("Initializing PS2 controller.");
while(1) {
    
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble); // software SPI initialization
    
    if(error == 0){
      Serial.print("Found Controller, configured successful after ");
      Serial.print(millis() - t_start, DEC);
      Serial.print("ms (pressures = ");
    if (pressures)
      Serial.print("true");
    else
      Serial.print("false");
    Serial.print(", rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
    break;
    }  
    else if(error == 1) {
      Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
      continue;
    }
    
    else if(error == 2) {
      Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
      continue;
    }

    else if(error == 3) {
      Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
      break; // non-fatal error
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  int lx = ps2x.Analog(PSS_LX) - 128;        // X-axis
  int ly = 128 - ps2x.Analog(PSS_LY);        // Y-axis (inverted)
  int rx = ps2x.Analog(PSS_RX) - 128;        // Rotation
  float m1 = ly + rx;
  float m2 = 0.866 * lx - 0.5 * ly + rx;
  float m3 = -0.866 * lx - 0.5 * ly + rx;

  delay(20);



}
