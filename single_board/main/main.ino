/** 
 * @main.ino
 * @brief Generic utilities
 * @author António Almeida
 * @author João Borrego
 */

#include "utils.hpp"

float lux_value = 0.0;
float ldr_in = 0.0; 
float v_in = 0.0;
float lookup[256] = LOOKUP_TABLE;
float ref = 0;
int pwm_value = 0;

int pin_ldr = A0;

void setup() {
    /* */
    Serial.begin(9600);
}

void loop() {

    //stair(0, 255, 1, 100, 1, false);
    stair(0, 255, 1, 100, 1, true);
    //feedForward();
}

void feedForward(){
  
      while(Serial.available() == 0){}

      ref = Serial.parseFloat();
      if(ref > 100 || ref < 0){
          Serial.println("please choose a positive number < 100");
      } else {
          for(int i = 1; i < 256; i++){
              if(ref < lookup[i-1]){
                  analogWrite(11, i-1);
                  Serial.print("Target led value is ");
                  Serial.println(lookup[i-1]);
                  break;
              }
          }
          delay(500);
          ldr_in =  analogRead(A0);
          v_in = ldr_in * (5.0 / 1023.0);
          lux_value = Utils::convertToLux(v_in);
          Serial.println(lux_value);
      }
}


void stair(
    int min,
    int max,
    int step,
    int sample_time,
    int samples,
    bool convert){

    float ldr_in;
    float v_in;
    float lux_value;

    for (int out = min; out <= max; out += step){
        analogWrite(11, out);
        for(int wait = 0; wait < samples; wait++){
            ldr_in =  analogRead(A0);
            if (convert){
                v_in = ldr_in * (5.0 / 1023.0);
                lux_value = Utils::convertToLux(v_in);
                Serial.print(lux_value);
            } else {
                Serial.print(ldr_in);
            }
            Serial.print(", ");
            delay(sample_time);
        }
    }
    Serial.println();
}

