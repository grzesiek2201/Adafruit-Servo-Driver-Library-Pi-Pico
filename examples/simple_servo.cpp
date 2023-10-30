#include "PCA9685_servo_driver.h"
#include "PCA9685_servo.h"
#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "pico/cyw43_arch.h"
#include <vector>


void StartMoveHandler(uint16_t Address);	// Servo callback
void StopMoveHandler(uint16_t Address);		// Servo callback
void init_servo(PCA9685_servo& servo, uint8_t mode, int16_t minRange, int16_t maxRange, int16_t position, uint8_t address, uint64_t TConstDur);

// create the controller and servos
PCA9685_servo_driver myController(i2c0, 0, 1, 0x40);
std::vector<PCA9685_servo> myServo = {PCA9685_servo(&myController, 0, 100, 540)}; // define a vector of servos, note this could extend to use multiple controller on the i2c bus
// necessary variables to drive the servo with constant speed or time
uint64_t TNow = 0;
uint64_t TPrevious = 0;
uint64_t TEllapsed = 0;


int main(void)
{
    // initialize Pi Pico peripherals
    stdio_init_all();

    // create connection to the PCA9685 and initialize it
    myController.begin(100000);
    // initialize the servos created
    int i{0};
    for(auto& servo : myServo){
        init_servo(servo, MODE_SCONSTANT, -90, 90, servo.getMinAngle(), i++, 1000000);
    }

    myServo[0].setAngularVelocity(40);

    bool changed = false;
    uint64_t start_t = time_us_64();
    sleep_ms(1000);
    while(1)
    {
        TNow = time_us_64();			// time now in microseconds
        TEllapsed = TNow - TPrevious;	// time, in microseconds, since the last loop
        TPrevious = TNow;				// store this ready for the next loop

        // loop through the servos calling their loop function so they can do their thing
        for(auto& servo : myServo){
            servo.loop(TEllapsed);
        }
        // after 6 seconds change position to relative and move
        if(TNow - start_t > 6 *1e6 && !changed)
        {
            myServo.at(0).setAngularVelocity(90);
            myServo.at(0).setRelativePosition(-90);
            changed = true;
        }
    }

    return 0;
}

void StartMoveHandler(uint16_t Address)
{
    // called when a servo starts to move
    return;
}

void StopMoveHandler(uint16_t Address)
{
    // called when a servo stops moving
	return;
}

void init_servo(PCA9685_servo& servo, uint8_t mode, int16_t minRange, int16_t maxRange, int16_t position, uint8_t address, uint64_t TConstDur)
{
    servo.setRange(minRange, maxRange);
    servo.setMode(mode);
    servo.setPosition(position); // move to mid point
    servo.setAddress(address);
    servo.setTConstantDuration(TConstDur);
}
