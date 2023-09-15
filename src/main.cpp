#include <Arduino.h>
#include <string.h>
#define DIR_R 30
#define NOT_DIR_R 31
#define DIR_L 4
#define NOT_DIR_L 5
#define SPEED_R 8
#define SPEED_L 9
#define FULL_CIRCLE_T 2
#define ONE_METER_T 3.5

static String command = "";
static bool EOL = 0;

static float duration = 0;
static bool direction_f = 0;
static bool rotation_f = 0;
static uint8_t PWM = 0;

static float yaw;
static float pos[3];
static float yaw_goal;
static float pos_goal[3];

uint64_t start_tp;

void serialEvent();
void moveX(float duration, bool direction, uint8_t speed = 254);
void rotateW(float duration, bool direction, uint8_t speed = 254);
void setup() 
{
  Serial.begin(9600);
//  Serial.println("wtf?");
  pinMode(13, 0x1);
  pinMode(DIR_R, 0x1);
  pinMode(DIR_L, 0x1);
  pinMode(NOT_DIR_L, 0x1);
  pinMode(NOT_DIR_R, 0x1);
  pinMode(SPEED_R, 0x1);
  pinMode(SPEED_L, 0x1);
  command.reserve(200);
}

void loop() 
{
  // digitalWrite(DIR_R, rotation_f & direction_f);//fix rotation
  // digitalWrite(DIR_L, !rotation_f & direction_f);
  // digitalWrite(NOT_DIR_R, rotation_f & !direction_f);
  // digitalWrite(NOT_DIR_L, !rotation_f & !direction_f);
  digitalWrite(DIR_R, rotation_f & direction_f);//fix rotation
  digitalWrite(DIR_L, !rotation_f & direction_f);
  digitalWrite(NOT_DIR_R, !(rotation_f & direction_f));
  digitalWrite(NOT_DIR_L, !(!rotation_f & direction_f));
  analogWrite(SPEED_R, PWM);
  analogWrite(SPEED_L, PWM);
  if(millis() - start_tp >= duration * 1000)
  {
    PWM = 0;
  }
  if(EOL)
  {

    // Serial.println("fine");
    // Serial.println(uint8_t(command[2]));
    // Serial.println(uint8_t(command[3]));
    // Serial.println(uint8_t(command[4]));
    // Serial.println(uint8_t(command[5]));
    //   duration = 10.0;
    //   rotation_f = 0;
    //   direction_f = 1;
    //   PWM = 50;
    //   start_tp = millis();
      
    // }
    if(uint8_t(command[0]) == 45 && uint8_t(command[1]) == 230)
    {
      Serial.println("move command received");
      PWM = (uint8_t(command[2]) == 0) ? 0 : uint8_t(command[5]);
      if(uint8_t(command[2]) == 1) rotation_f = 1;
      else rotation_f = 0;
      duration = float(command[3]);//to do: 4 bytes for duration
      direction_f = bool(command[4]);
      Serial.println(command);
      Serial.println(duration);
      Serial.println(direction_f);
      Serial.println(rotation_f);
      Serial.println(PWM);
      start_tp = millis();
    }
    if(uint8_t(command[0]) == 45 && uint8_t(command[1]) == 231 && command.length() == 19)
    {
      Serial.print("position data received");
      //char bytes[4][4];
      Serial.println(command);
      float data[4];
      for(uint8_t i = 0; i < 4; ++i)
      {
        char buff[4];
        Serial.println("retardness check:");//db
        for(uint8_t j = 4 * i + 2; j < 4 * (i + 1) + 2; ++j)
        {
          buff[(j - 2) % 4] = command[j]; //command.size()
          Serial.println((j - 2) % 4);//db
        }
        Serial.println("");//db
        memcpy(&(data[i]), buff, 4);
      }
      
      yaw = data[0];
      Serial.print("Pos: ");
      for(uint8_t i = 0; i < 3; ++i)
      {
        pos[i] = data[i + 1];
        Serial.print(" ");
        Serial.print(pos[i]);
      }
      Serial.println("");
      Serial.print("Yaw: ");
      Serial.println(yaw);

    }
    command = "";
    EOL = 0;
  
  }
//  Serial.println("tick");
//  delay(1000);
}
void serialEvent()
{
  while(Serial.available())
  {
    char symbol = char(Serial.read());
    command += symbol;
    if(symbol == '\n')  EOL = 1;
  }

}
void moveX(float duration, bool direction, uint8_t speed)
{
  digitalWrite(DIR_R, direction);
  digitalWrite(DIR_L, direction);
  digitalWrite(NOT_DIR_R, !direction);
  digitalWrite(NOT_DIR_L, !direction);
  analogWrite(SPEED_R, speed);
  analogWrite(SPEED_L, speed);
  delay(duration * 1000);
  digitalWrite(SPEED_R, 0);
  digitalWrite(SPEED_L, 0);
}
void rotateW(float duration, bool direction, uint8_t speed)
{
  digitalWrite(NOT_DIR_R, !direction);
  digitalWrite(DIR_R, direction);
  digitalWrite(NOT_DIR_L, direction);
  digitalWrite(DIR_L, !direction);
  analogWrite(SPEED_R, speed);
  analogWrite(SPEED_L, speed);
  delay(duration * 1000);
  digitalWrite(SPEED_R, 0);
  digitalWrite(SPEED_L, 0);
}