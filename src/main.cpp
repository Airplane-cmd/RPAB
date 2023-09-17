#include "HardwareSerial.h"
#include <Arduino.h>
#include <string.h>
#define DIR_R 30
#define DIR_R_INV 31
#define SPEED_R 8

#define DIR_L 4
#define DIR_L_INV 5
#define SPEED_L 9

#define YAW_COEFF_P 0.1
#define YAW_COEFF_I 0.1
#define YAW_COEFF_D 0.1

#define SPEED_COEFF_P 0.1
#define SPEED_COEFF_I 0.1
#define SPEED_COEFF_D 0.1

#define HEADER_SPEED 0xaa

#define HEADER_YAW 0xba
#define HEADER_YAW_TARGET 0xbb

static String buffer = "";

int16_t speed, speed_target, yaw, yaw_target;
bool data_updated = 0;

void write_right(int power) {
  digitalWrite(DIR_R, power > 0);
  digitalWrite(DIR_R_INV, power < 0);
  analogWrite(SPEED_R, abs(power));
}

void write_left(int power) {
  digitalWrite(DIR_L, power > 0);
  digitalWrite(DIR_L_INV, power < 0);
  analogWrite(SPEED_L, abs(power));
}

int16_t u_yaw, u_p_yaw, u_i_yaw, u_d_yaw, last_err_yaw, last_time_yaw;
void evaluate_yaw() {
  int16_t err = yaw_target - yaw;
  u_p_yaw = YAW_COEFF_P * err;
  u_i_yaw += YAW_COEFF_I * err;
  u_d_yaw = YAW_COEFF_D *
            ((err - last_err_yaw) / (millis() - last_time_yaw)); // kD(de/dt)

  if (abs(err) < 256) // we don't need I component when error is small
    u_i_yaw = YAW_COEFF_I * err;

  last_err_yaw = err;
  u_yaw = u_p_yaw + u_i_yaw + u_d_yaw;
}

// int16_t u_speed, u_p_speed, u_i_speed, u_d_speed, last_err_speed,
//     last_time_speed;
// void evaluate_speed() {
//   int16_t err = speed_target - speed;
//   u_p_speed = SPEED_COEFF_P * err;
//   u_i_speed += SPEED_COEFF_I * err;
//   u_d_speed = SPEED_COEFF_D * ((err - last_err_speed) /
//                                (millis() - last_time_speed)); // kD(de/dt)

//   if (abs(err) < 256) // we don't need I component when error is small
//     u_i_speed = SPEED_COEFF_I * err;

//   last_err_speed = err;
//   u_speed = u_p_speed + u_i_speed + u_d_speed;
// }

void setup() {
  Serial.begin(2000000);
  pinMode(13, 0x1);

  pinMode(DIR_R, 0x1);
  pinMode(DIR_R_INV, 0x1);
  pinMode(SPEED_R, 0x1);

  pinMode(DIR_L, 0x1);
  pinMode(DIR_L_INV, 0x1);
  pinMode(SPEED_L, 0x1);
}

void loop() {
  if (Serial.available()) {
    data_updated = 1;
    while (Serial.available()) {
      switch (Serial.read()) {
      case HEADER_SPEED:
        Serial.readBytes((char *)&speed, 2);
      case HEADER_YAW:
        while (Serial.available() < 4)
          asm("nop");
        Serial.readBytes((char *)&yaw, 2);
        Serial.readBytes((char *)&yaw_target, 2);
        evaluate_yaw();
      }
    }
  }
  // CHANGE SIGNS IF BOAT DESTABILISES
  write_left(map(speed + u_yaw, -32768, 32767, -255, 255));
  write_right(map(speed + u_yaw, -32768, 32767, -255, 255));
}

// void moveX(float duration, bool direction, uint8_t speed) {
//   digitalWrite(DIR_R, direction);
//   digitalWrite(DIR_L, direction);
//   digitalWrite(DIR_R_INV, !direction);
//   digitalWrite(DIR_L_INV, !direction);
//   analogWrite(SPEED_R, speed);
//   analogWrite(SPEED_L, speed);
//   delay(duration * 1000);
//   digitalWrite(SPEED_R, 0);
//   digitalWrite(SPEED_L, 0);
// }
// void rotateW(float duration, bool direction, uint8_t speed) {
//   digitalWrite(DIR_R_INV, !direction);
//   digitalWrite(DIR_R, direction);
//   digitalWrite(DIR_L_INV, direction);
//   digitalWrite(DIR_L, !direction);
//   analogWrite(SPEED_R, speed);
//   analogWrite(SPEED_L, speed);
//   delay(duration * 1000);
//   digitalWrite(SPEED_R, 0);
//   digitalWrite(SPEED_L, 0);
// }