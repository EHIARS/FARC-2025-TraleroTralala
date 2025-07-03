//Khai báo thư viện cho tay cầm
#include <PS2X_lib.h> // Khai báo thư viện

//Định nghĩa các chân điều khiển 
#define PS2_DAT 12 // MISO 
#define PS2_CMD 13 // MOSI 
#define PS2_SEL 15 // SS 
#define PS2_CLK 14 // SLK

//Khởi tạo class của thư viện
PS2X ps2x; // khởi tạo class PS2x

void setup(){
//Khởi tạo Serial monitor với tốc độ 115200
Serial.begin(115200);

//Kết nối với tay cầm bằng hàm ps2x.config_gamepad, thử kết nối lại trong vòng 10 lần nếu quá 10 lần không kết nối được với tay cầm thì sẽ dừng lại

int error = -1; 
for (int i = 0; i < 10; i++) // thử kết nối với tay cầm ps2 trong 10 lần 
{
  delay(1000); // đợi 1 giây 
  // cài đặt chân và các chế độ: GamePad
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble); 
  Serial.print("."); 
  if(!error) //kiểm tra nếu tay cầm đã kết nối thành công 
	break; // thoát khỏi vòng lặp 
} 
}

void loop(){

}