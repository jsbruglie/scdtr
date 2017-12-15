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
#include "pi.hpp"
#include "communication.hpp"
#include "calibration.hpp"
#include "consensus.hpp"

/* Global variables */

/** Device id */
uint8_t id{0};

/* System calibration */

/** Row i of K parameter matrix */
float k_i[N]    {0.0};
/** LUX value for complete darkness */
float ext       {0.0};

/* System I/O */

/** LDR input ADC value */
volatile float ldr_in       {0.0};
/** LDR input in Volt */
volatile float v_in         {0.0};
/** LDR input converted to LUX units */
volatile float lux          {0.0};
/** LED duty cycle */
volatile float out          {0.0};

/* Real-time control */

/** Lux lower bound */
volatile float lower_bound  {LOW_LUX};
/** Lux reference */
volatile float ref          {0.0};
/** Occupancy */
volatile bool occupancy     {false};

/** Proportional Integrator controller */
PIController::Controller controller(
  &lux, &out, &ref, K_P, K_I, T);

/** Latest elapsed milliseconds since startup */
unsigned long current_millis    {0};
/** Previously recorded elapsed milliseconds since startup */
unsigned long last_millis       {0};

/* Simple state machine */

/** Current node state */
volatile int state      {CONTROL};
/** Reset trigger */
volatile bool reset     {false};
/** Calibration trigger */
volatile bool calibrate {false};
/** Consensus trigger */
volatile bool consensus {false};

/** Command line buffer for Serial input */
char cmd_buffer[BUFFER_SIZE] {0};

/**
 * @brief      Arduino setup
 */
void setup() {

    // Setup serial communication
    Serial.begin(BAUDRATE);

    // Read device ID from EEPROM
    id = EEPROM.read(ID_ADDR);
    Serial.print("[I2C] Registered with id ");
    Serial.println((int) id);

    // Setup I2C communication
    Communication::setup(id, &reset, &consensus, &lower_bound, &occupancy);
    Wire.begin(id);
    Wire.onReceive(Communication::onReceive);

    // Setup timer interrupt
    setupTimerInt();

    // Configure controller features
    controller.configureFeatures(true, true, true);
    /*
    // Determine K matrix and external illuminance
    Wire.onReceive(Calibration::onReceive);
    Wire.onRequest(Calibration::onRequest);
    Calibration::execute(k_i, &ext, id);
    // Setup I2C communication for main loop
    Wire.onReceive(Communication::onReceive);
    */

}

/**
 * @brief      Arduino loop
 */
void loop() {

    // Master has a Serial connection with an external server
    if (id == MASTER) {
        if (readLine()){
            processCommand();
        }
    }

    // Update current state
    updateState();

    // Broadcast information to I2C bus periodically
    current_millis = millis();
    if (current_millis - last_millis >= STATUS_DELAY){
        last_millis = current_millis;

        // Packets have to be acknowledged in order to be sniffed:
        // Each device acks a single packet
        Communication::sendInfo((id + 1) % N,
            lux, out / 255.0, lower_bound, ext, ref, occupancy);

        // DEBUG
        printState();

    }

    // if start consensus

    /*
    Wire.onReceive(Communication::nop);

    float k_i_tmp[N][N] = {{2.0, 1.0}, {1.0, 2.0}};
    int d_best;
    if (id == 0){
        d_best = Consensus::solve(id, 150.0, k_i_tmp[0], 30.0);
    }else{
        d_best = Consensus::solve(id, 80.0,  k_i_tmp[1],  0.0);
    }
    Serial.println(d_best);
    */
}

// DEBUG
void printState(){
    Serial.print(lux);
    Serial.print("\t");
    Serial.print((floor(out)) / 255.0 );
    Serial.print("\t");
    Serial.print(lower_bound);
    Serial.print("\t");
    Serial.print(ext);
    Serial.print("\t");
    Serial.print(ref);
    Serial.print("\t");
    Serial.print(occupancy);
    Serial.print("\t");
    Serial.println(current_millis);
}

/**
 * @brief      Updates the current state.
 */
void updateState(){

    if (reset) {
        reset = false;
        state = CALIBRATION;
        // TODO - Calibrate
        Serial.println("[Calibration]");
    } else if (state == CONTROL) {
        if (consensus){
            consensus = false;
            state = CONSENSUS;
            // TODO - Consensus
            Serial.println("[Consensus]");
        }
    }
}

/**
 * @brief      Reads a line from Serial.
 *
 * @return     Whether a line was read
 */
bool readLine(){

    static uint8_t offset = 0;

    while (Serial.available()){
        char cur = Serial.read();
        switch (cur) {
            // Carriage return
            case '\r':
            // Line feed
            case '\n':
                // Terminate string
                cmd_buffer[offset] = '\0';
                if (offset > 0){
                    offset = 0;
                    //printCommand();
                    return true;
                }
                break;
            default:
                if (offset < BUFFER_SIZE - 1){
                    cmd_buffer[offset] = cur;
                    offset++;
                    cmd_buffer[offset] = '\0';
                }
        }
    }
    return false;
}

/**
 * @brief      Processes a command read from Serial.
 */
void processCommand(){

    char *command = strtok(cmd_buffer, " \n");
    char *param = strtok(NULL, " \n");
    char *value = strtok(NULL, " \n");

    int i = (param)? atoi(param) : ND;
    float f = (value)? atof(value) : ND;

    if (!strcmp(command, CMD_RESET)) {
        
        for (int j = 0; j < N; j++) {
            if (j != MASTER) {
                Communication::sendPacket(j, RES);
            }
        }
        reset = true;

    } else {

        if ( (!strcmp(command, CMD_OCCUPANCY) || !strcmp(command, CMD_LOWER_BOUND) )
            && i >= 0 && i < N && f != ND) {
            
            float tmp_lower_bound[N] = {ND};
            
            // Occupancy argument is binary, convert to float lux value
            if (!strcmp(command, CMD_OCCUPANCY)) {
                f = (atoi(value) == 0)? LOW_LUX : HIGH_LUX;
            }
            // Master can update its local estimate
            if (i == MASTER) {
                lower_bound = f;
                occupancy = (lower_bound <= LOW_LUX)? false : true;
            } else {
                tmp_lower_bound[i] = f;
            }
            // Broadcast estimate to other nodes and initiate consensus
            for (int j = 0; j < N; j++) {
                if (j != MASTER) {
                    Communication::sendConsensus(j, true, tmp_lower_bound);
                    // DEBUG
                    Serial.println(tmp_lower_bound[1]);
                }
            }
            consensus = true;
        }
    }
}

/**
 * @brief      Sets up timer interrupts
 *
 * @note       The body of this funtion was generated automatically at
 *             <a href="http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html">
 *             Arduino Timer Interrupt Calculator</a>
 */
void setupTimerInt(){
    // TIMER 1 for interrupt frequency 100/3 (33.(3)) Hz:
    cli(); // stop interrupts
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1  = 0; // initialize counter value to 0
    // set compare match register for 100 / 3 Hz increments
    OCR1A = 60000; // = 16000000 / (8 * (100 / 3)) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12, CS11 and CS10 bits for 8 prescaler
    TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei(); // allow interrupts
}

/**
 * @brief      Writes to led.
 */
void writeToLed(){
    /* Constrain output to 8 bits */
    out = (out >= 0)?   out : 255;
    out = (out <= 255)? out : 0;
    analogWrite(pin_led, (int) out);
}

/**
 * @brief      Timer1 interrupt callback function
 *
 * @param[in]  <unnamed>    Timer/Counter1 Compare Match A
 */
ISR(TIMER1_COMPA_vect){

    if (state == CONTROL){

        // Sample input
        ldr_in =  analogRead(pin_ldr);
        v_in = ldr_in * (VCC / 1023.0);
        lux = Utils::convertToLux(v_in, LUX_A[id], LUX_B[id]);

        // Control system
        controller.update(writeToLed);

    } else if (state == CALIBRATION) {
        // Ignore interrupt
    }
}
