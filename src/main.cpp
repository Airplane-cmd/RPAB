#include <Arduino.h>
#include <string.h>
#define DIR_L 30
#define NOT_DIR_L 31
#define DIR_R 4
#define NOT_DIR_R 5
#define SPEED_L 8
#define SPEED_R 9
#define ONE_METER_T 3.5

static bool followStream_f = 0;
 
static int16_t pwm_l;
static int16_t pwm_r;

static float cmd_vel_linear;
static float cmd_vel_angular;

static String command = "";
static bool EOL = 0;

static float duration = 0;
static bool direction_f = 0;
static bool rotation_f = 0;
static uint8_t PWM = 0;
static bool stop_f = 0;
static float yaw;
static float pos[3];
static float yaw_goal;
static float pos_goal[3];

static uint8_t minAutoPWM = 40;
static uint8_t maxAutoPWM = 60;

uint64_t start_tp;
uint64_t cmd_vel_tp;
uint32_t streamThreshold = 100;

void serialEvent();
void moveX(float duration, bool direction, uint8_t speed = 254);
void rotateW(float duration, bool direction, uint8_t speed = 254);
float getFloatFromString(String &command, uint8_t start);
void cmd_velSpin(float angular_vel, float linear_vel);
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
  // digitalWrite(DIR_R, direction_f);//fix rotation 
  // digitalWrite(DIR_L, rotation_f || !direction_f);
  // digitalWrite(NOT_DIR_R, !direction_f);
  // digitalWrite(NOT_DIR_L, !(rotation_f || !direction_f));//fancy shit
  if(!followStream_f)
  {
    if(!rotation_f)
    {
      digitalWrite(DIR_R, direction_f);//fix rotation 
      digitalWrite(DIR_L, !direction_f);
      digitalWrite(NOT_DIR_R, !direction_f);
      digitalWrite(NOT_DIR_L, direction_f);//not fancy shit
    }
    else
    {
      digitalWrite(DIR_R, direction_f);//fix rotation 
      digitalWrite(DIR_L, direction_f);
      digitalWrite(NOT_DIR_R, !direction_f);
      digitalWrite(NOT_DIR_L, !direction_f);//not fancy shit
    }
    analogWrite(SPEED_R, PWM);
    analogWrite(SPEED_L, PWM);
  }
  if(millis() - start_tp >= duration)
  {
   if(!followStream_f) PWM = 0;
  }
  if(followStream_f)
  {
    if(millis() - cmd_vel_tp >= streamThreshold)  
      {
        analogWrite(SPEED_R, 0);
        analogWrite(SPEED_L, 0);
      }
  }
  if(EOL)
  {
    if(uint8_t(command[0]) == 45 && uint8_t(command[1]) == 230 && !followStream_f)
    {
      Serial.println("move command received");
      PWM = (uint8_t(command[2]) == 0) ? 0 : uint8_t(command[5]);
      if(uint8_t(command[2]) == 1) rotation_f = 1;
      else rotation_f = 0;
      duration = getFloatFromString(command, 6);//to do: 4 bytes for duration
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
          buff[(j - 2) % 4] = command[j]; //command.size() nice
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
    if(uint8_t(command[0]) == 45 && uint8_t(command[1]) == 232)
    {

      Serial.println("Received cmd_vel");
      cmd_vel_linear = getFloatFromString(command, 2);
      cmd_vel_angular = getFloatFromString(command, 6);
      Serial.print("Linear cmd_vel: ");
      Serial.println(cmd_vel_linear);
      Serial.print("Angular cmd_vel: ");
      Serial.println(cmd_vel_angular);
      if(followStream_f)  cmd_velSpin(cmd_vel_angular, cmd_vel_linear);
      
    }
    if(uint8_t(command[0]) == 45 && uint8_t(command[1]) == 233 && command.length() == 4)//stop
    {
      followStream_f = command[2];
      Serial.print("Follow: ");
      Serial.println(followStream_f);
      if(!followStream_f) 
      {
        duration = 0; 
        pwm_l = 0;
        pwm_r = 0;
        PWM = 0;
      }
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
float getFloatFromString(String &str, uint8_t start)
{
  char buff[4];
  float data;
  for(uint8_t i = start; i < start + 4; ++i)
  {
    buff[i - start] = command[i]; //command.size() nice
    // Serial.println((j - 2) % 4);//db
  }
  // Serial.println("");//db
  memcpy(&data, buff, 4);
  return data;
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
void cmd_velSpin(float angular_vel, float linear_vel)
{
  Serial.println("cmd_velSpin called");
  if(abs(linear_vel) <= 0.01f && abs(angular_vel) <= 0.1f)
  {
    pwm_l = 0;
    pwm_r = 0;
  }
  if(abs(linear_vel) > 0.01f)
  {
    pwm_l = (abs(linear_vel) * (maxAutoPWM - minAutoPWM)) + minAutoPWM;
    pwm_r = (abs(linear_vel) * (maxAutoPWM - minAutoPWM)) + minAutoPWM;
    direction_f = (linear_vel > 0.01f) ? 0 : 1;//todo: as params

    if(angular_vel > 0.1f)
    { 
      if(!direction_f) pwm_l = 0;
      else            pwm_r = 0;
    }
    if(angular_vel < -0.1f)
    {
      
      if(!direction_f) pwm_r = 0;//hope, it works
      else            pwm_l = 0;
    } 
  }
  if(abs(linear_vel) <= 0.01f && abs(angular_vel) > 0.1f)
  {
    direction_f = 0;
    if(angular_vel > 0.1f)
    { 
      pwm_r = 50;//minAutoPWM;//hope, it works
      pwm_l = 0; 
    }
    if(angular_vel < -0.1f)
    {
      pwm_r = 0;//hope, it works
      pwm_l = 50;//minAutoPWM;
    }    
  }
  Serial.print("Left, right pwm: ");
  Serial.print(pwm_l);
  Serial.print(" ");
  Serial.println(pwm_r);
  digitalWrite(DIR_R, direction_f);
  digitalWrite(DIR_L, !direction_f);
  digitalWrite(NOT_DIR_R, !direction_f);
  digitalWrite(NOT_DIR_L, direction_f); 
  analogWrite(SPEED_R, pwm_r);
  analogWrite(SPEED_L, pwm_l);
  cmd_vel_tp = millis();
}
