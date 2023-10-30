#ifndef PCA9685_servo_h
#define PCA9685_servo_h

#include "PCA9685_servo_driver.h"
#include <stdlib.h>
#include <iostream>

#define MODE_FAST		0			// move to the set point as quickly as the servo can manage
#define MODE_TCONSTANT	1			// move to the set point over a constant time period
#define MODE_SCONSTANT	2			// move to the set point at a constant angular velocity

class PCA9685_servo
{
public:
	PCA9685_servo(PCA9685_servo_driver *controller, uint8_t channel, uint16_t PWMmin = 104, uint16_t PWMmax = 508);
	void setPosition(int8_t angle);						// set the desired position, in degrees
	void setRelativePosition(int8_t angle);				// set the desired position relative to the current angle, in degrees
	int8_t getPosition(void);
	int8_t getMinAngle(void);							// return minimum set angle
	int8_t getMaxAngle(void);							// return mid set point angle
	int8_t getMidAngle(void);							// return maximum set angle
	uint8_t getMode(void);
	uint8_t isMoving(void);								// returns 1 if the servo is moving, 0 otherwise
	uint64_t getTConstantDuration(void);				// returns the number of microseconds to use as the constant movement duration
	uint64_t getSConstantPeriod(void);
	uint16_t getAddress(void);
	uint8_t getInvertMode(void);
	
	void setMinAngle(int8_t angle);
	void setMidAngle(int8_t angle);
	void setMaxAngle(int8_t angle);
	void setRange(int8_t minAngle, int8_t maxAngle);	// set the allowable range of operation, in degrees
	void setRange(int8_t minAngle, int8_t midAngle, int8_t maxAngle);	// set the allowable range of operation, in degrees
	void setMode(uint8_t Mode);
	void setTConstantDuration(uint64_t TConstantDuration);	// sets the time constant for the movement duration, in microseconds
	void setSConstantPeriod(uint64_t SConstantPeriod);		// sets the number of microseconds to move 1 degree
	void setAddress(uint16_t address);
	void setInvertMode(uint8_t invert);
	
	void throwServo(uint8_t value);								// 0 = unthrow, 1 = throw
	
	void loop(uint64_t TEllapsed);						// call every loop iteration, fEllapsedTime 

	void changeSpeedConstT(uint64_t TConstantDuration);		// change speed of the servo in MODE_TCONSTANT mode 
	void setAngularVelocity(double angVel);					// set the speed of the servo in MODE_SCONSTANT mode deg/s
	void stop();
	
	// callback function pointers

	/// @brief Function pointer to a function that is to be called when the movement starts.
	/// @note E.g.
	/// @note void StartMoveHandler(uint16_t Address) { INTERNAL_LED(1); }
	/// @note servo.onStartMove = StartMoveHandler;
	void (*onStartMove)(uint16_t address) = NULL;
	/// @brief Function pointer to a function that is to be called when the movement stops.
	/// @note E.g.
	/// @note void StopMoveHandler(uint16_t Address) { INTERNAL_LED(0); }
	/// @note servo.onStopMove = StopMoveHandler;
	void (*onStopMove)(uint16_t address) = NULL;
	
private:
	PCA9685_servo_driver *_controller;
	uint8_t _channel;
	uint16_t _PWMmin;
	uint16_t _PWMmax;
	
	uint8_t _invert = 0;				// 0 = normal, 1 = invert
	uint16_t	_address = 0;			// the servos address, basically a user variable that can be set as desired
	int8_t	_minAngle = -90;
	int8_t	_maxAngle = 90;
	int8_t	_midAngle = 0;
	int8_t 	_currentAngle = 0;			// the angle the servo is currently set to be at
	int8_t 	_targetAngle = 0;
	int8_t 	_startAngle = 0;			// the angle that the movement starts from

	uint64_t _Tcntr = 0;				// an ellapsed time counter
	uint64_t _TFastDuration = 500000;	// time in microseconds to allow for a "fast" move
	uint64_t _TConstantDuration = 2000000;	// time in microseconds to allow for a constant time move
	uint64_t _TConstantPeriod = 0;
	uint64_t _SConstantPeriod = 100000;		// number of microseconds between each angle for a constant speed
	
	uint8_t	_isMoving = 0;				// a flag to say if we are currently moving

	uint8_t	_mode = 0;
};

#endif

