/** 
 * @file main.ino
 * @brief Multiple board system main application
 * @author António Almeida
 * @author João Borrego
 */

#include <EEPROM.h>
#include <Wire.h>

#include "constants.hpp"
#include "utils.hpp"

using namespace Utils;

/* Pinout */

/** LDR analog in pin */
const int pin_ldr = A0;
/** LED PWM out pin */
const int pin_led = 11;

/* Global variables */

/** Device id */
uint8_t id{0};

/** K parameter matrix */
float k[N*N]{0};

/** LDR input ADC value */
volatile float ldr_in {0};
/** LDR input in Volt */
volatile float v_in {0};
/** LDR input converted to LUX units */
volatile float input {0};

/** Latest elapsed milliseconds since startup */
unsigned long current_millis {0};
/** Previously recorded elapsed milliseconds since startup */
unsigned long last_millis {0};

void calibrateOnReceive(int bytes);
void calibrateOnRequest();
void calibrate(float *k_matrix, int devices, uint8_t id);

/**
 * @brief      Arduino setup
 */
void setup() {
    /* Read device ID from EEPROM */
    id = EEPROM.read(ID_ADDR);
    /* Setup serial communication */
    Serial.begin(BAUDRATE);
    /* Setup I2C communication */
    if (id == MASTER){
        Wire.begin();
        Serial.print("[I2C] Registered as Master - ");
    } else {
        Wire.begin(id);
        Wire.onReceive(calibrateOnReceive);
        Wire.onRequest(calibrateOnRequest);
        Serial.print("[I2C] Registered as Slave - ");
    }
    Serial.println(id);
}

/**
 * @brief      Arduino loop
 */
void loop() {
    
    /* Print status every STATUS_DELAY milliseconds */
    current_millis = millis();
    if (current_millis - last_millis >=  STATUS_DELAY){
        last_millis = current_millis;

        calibrate(k, N, id);

        /* Optional: lower CPU usage */
        delay(0.7 * STATUS_DELAY);
    }
}

volatile bool calibration_ready{false};

void writeFloat(float var){
    int size = sizeof(float);
    byte packet[size];
    float2bytes_t f2b;
    f2b.f = var;
    for (int i = 0; i < size; i++)
        packet[i] = f2b.b[i];
    Wire.write(packet, size);
}

float readFloat(){
    float2bytes_t b2f;
    bool received = false;
    int i = 0;

    while (Wire.available()) { 
        b2f.b[i++] = Wire.read();
        received = true;
    }
    return (received)? b2f.f : -1;
}

void inline sendReady(int destination){
    byte value = 0;
    Wire.beginTransmission(destination);
    Wire.write(value);
    Wire.endTransmission();
}

void inline waitReady(){
    while (calibration_ready == false){}
    calibration_ready = false;
}

void calibrateOnReceive(int bytes){

    if (Wire.available() != 0){
        // Command
        if (bytes == 1){
            byte value = Wire.read();
            if (value == 0){
                calibration_ready = true;
            }
        // Data
        } else {
            for (int i = 0; i < bytes; i++){
                byte value = Wire.read();
                Serial.print("[I2C] Received: ");
                Serial.println(value, HEX);
            }
        }
    }
}

float getLDRValue(){
    ldr_in = analogRead(pin_ldr);
    v_in = ldr_in * (VCC / 1023.0);
    input = Utils::convertToLux(v_in, LUX_A[id], LUX_B[id]);
    return input;
}

void calibrateOnRequest(){
    float lux = getLDRValue();
    writeFloat(lux);
}

void calibrate(float *k_matrix, int devices, uint8_t id){

    float value;

    // Master 
    if (id == MASTER){
        for (int i = 0; i < devices; i++){
            if (i != MASTER){
                sendReady(i);
            }
        }
    } else {
        waitReady();
    }

    Serial.println("[Calibration] Started");

    for (int i = 0; i < devices; i++){
        // Turn system i led on
        if (i == id){
            analogWrite(pin_led, 255);
        }
        delay(100);

        if (id == MASTER){
            for (int j = 0; j < devices; j++){
                if (j == MASTER){
                    value = getLDRValue();
                } else {
                    Wire.requestFrom(j, 4);
                    while ((value = readFloat()) < 0){}
                }
                
                // DEBUG
                Serial.print(value);
                if (i == j && j == devices - 1){
                    Serial.print("\n");
                } else {
                    Serial.print(",");
                }

                k[i*N + j] = value; 
            }
            for (int j = 0; j < devices; j++){
                if (j != MASTER){
                    sendReady(j);
                }
            }
        } else {
            waitReady();
        }

        // Turn system i led off
        if (i == id){
            analogWrite(pin_led, 0);
        }
        delay(100);
    }

    // TODO - Send matrix to each arduino?

    Serial.println("[Calibration] Done");
}

/**
 * @brief      Lists important variables to Serial
 */
void listVariables(){

    ldr_in = analogRead(pin_ldr);
    v_in = ldr_in * (VCC / 1023.0);
    input = Utils::convertToLux(v_in, LUX_A[id], LUX_B[id]);

    Serial.print(id, DEC);
    Serial.print("\t");
    Serial.print(ldr_in);
    Serial.print("\t");
    Serial.println(input);
}