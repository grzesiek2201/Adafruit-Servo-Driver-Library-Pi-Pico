#include "PCA9685_servo.h"

/// @brief PCA9685_servo constructor.
/// @param controller PCA9685_servo_controller pointer
/// @param channel channel number of the servo on the PCA9685
/// @param PWMmin minimal PWM signal for the servo. This is not the minimal pulse width of the servo, but rather the pulse length count. Min and max values usually within (150-600) range.
/// @param PWMmax maximal PWM signal for the servo. Just like the PWMmin, to be determined experimentally, by slowly raising the value and checking the motion range.
PCA9685_servo::PCA9685_servo(PCA9685_servo_driver *controller, uint8_t channel, uint16_t PWMmin, uint16_t PWMmax) :
	_controller{controller}, _channel{channel}, _PWMmin{PWMmin}, _PWMmax{PWMmax}
{}

/// @brief Routine to call every loop iteration. 
/// @note Enclose this in a for()/while() loop and call every time, passing
/// 		TEllapsed argument containing the time passed since loop() was last called. 
/// @note 	E.g.
///	@note		TNow = time_us_64();			// time now in microseconds
///	@note		TEllapsed = TNow - TPrevious;	// time, in microseconds, since the last loop
///	@note		TPrevious = TNow;				// store this ready for the next loop
/// @note		servo.loop(TEllapsed);
/// @param TEllapsed time since last function call in microseconds
void PCA9685_servo::loop(uint64_t TEllapsed)
{
	switch (_mode)
	{
		case MODE_FAST:
		{
			if (_isMoving)
			{
				// has enough time ellasped to allow us to have moved into position?
				_Tcntr += TEllapsed;
				
				if (_Tcntr >= _TFastDuration)
				{
					// we have finished moving
					_isMoving = 0;
					if (onStopMove != NULL) onStopMove(0);
				}
			}
			
			break;
		}
		
		case MODE_TCONSTANT:
		case MODE_SCONSTANT:
		{
			if (_isMoving)
			{
				// has enough time ellasped that we are due to move again?
				_Tcntr += TEllapsed;
				if (_Tcntr >= _TConstantPeriod)
				{
					_Tcntr = 0;
					// we need to move by one degree
					if (_currentAngle < _targetAngle)
					{
						// increase angle
						_currentAngle++;

						uint16_t PWM = (uint16_t) _controller->map(_currentAngle, -90, 90, _PWMmin, _PWMmax);
						
						_controller->setPWM(_channel, PWM);
					}
					else
					{
						// decrease angle
						_currentAngle--;
						
						uint16_t PWM = (uint16_t) _controller->map(_currentAngle, -90, 90, _PWMmin, _PWMmax);
						
						_controller->setPWM(_channel, PWM);
					}
				}
				
				// have we finished moving?
				if (_currentAngle == _targetAngle)
				{
					_isMoving = 0;
					if (onStopMove != NULL) onStopMove(0);
				}
			}
			break;
		}
	}
	
	return;
}

/// @brief Set target position for the servo.
/// @param Angle target position in degrees
void PCA9685_servo::setPosition(int8_t angle)
{
	_startAngle = _currentAngle;
	if (angle < _minAngle) angle = _minAngle;
	if (angle > _maxAngle) angle = _maxAngle;

	switch (_mode)
	{
		// move to the set point as quickly as possible
		case MODE_FAST:
		{
			_currentAngle = angle;

			uint16_t PWM = (uint16_t) _controller->map(angle, -90, 90, _PWMmin, _PWMmax);
			
			_controller->setPWM(_channel, PWM);
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
		
		// move to the set point gradually over a constant time
		case MODE_TCONSTANT:
		{
			_targetAngle = angle;	// where we wish to end up
			uint64_t AngleDelta = abs(angle - _currentAngle);		// how many degrees we need to move
			
			// how long between 1 degree movements?
			_TConstantPeriod = _TConstantDuration / AngleDelta;	
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
		
		// move to the set point gradually at a set angular velocity
		case MODE_SCONSTANT:
		{
			_targetAngle = angle;	// where we wish to end up
			_TConstantPeriod = _SConstantPeriod;
			
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
	}

	return;
}

/// @brief Set relative target position for the servo (in realtion to current angle).
/// @param angle relative angle in degrees
/// @note Does not work in MODE_FAST mode.
void PCA9685_servo::setRelativePosition(int8_t angle)
{
	int16_t newAngle = _currentAngle + angle;
	if (newAngle < _minAngle) newAngle = _minAngle;
	if (newAngle > _maxAngle) newAngle = _maxAngle;
	setPosition(newAngle);
}

/// @brief Get the current angle of the servo.
/// @param  
/// @return current angle of the servo.
int8_t PCA9685_servo::getPosition(void)
{
	return _currentAngle;
}

void PCA9685_servo::setRange(int8_t minAngle, int8_t maxAngle)
{
	_minAngle = minAngle;
	_maxAngle = maxAngle;
	_midAngle = ((_maxAngle - _minAngle) / 2) + _minAngle;
	return;
}

void PCA9685_servo::setRange(int8_t minAngle, int8_t midAngle, int8_t maxAngle)
{
	_minAngle = minAngle;
	_maxAngle = maxAngle;
	_midAngle = midAngle;
	return;
}

uint8_t PCA9685_servo::getInvertMode(void)
{
	return _invert;
}

void PCA9685_servo::setInvertMode(uint8_t invert)
{
	_invert = invert;
}

/// @brief Set servo position to its max or min angle value.
/// @param value 0 if movement to max, -1 if movement to min.
void PCA9685_servo::throwServo(uint8_t value)
{
	if (_invert)
	{
		// inverted operation
		if (value == 0)
		{
			setPosition(_maxAngle);
		}
		else
		{
			setPosition(_minAngle);
		}
	}
	else
	{
		// normal operation
		if (value == 0)
		{
			setPosition(_minAngle);
		}
		else
		{
			setPosition(_maxAngle);
		}
	}
	return;
}

int8_t PCA9685_servo::getMinAngle(void)
{
	return _minAngle;
}

int8_t PCA9685_servo::getMaxAngle(void)
{
	return _maxAngle;
}

int8_t PCA9685_servo::getMidAngle(void)
{
	return _midAngle;
}

uint8_t PCA9685_servo::getMode(void)
{
	return _mode;
}

uint16_t PCA9685_servo::getAddress(void)
{
	return _address;
}

/// @brief  Check if servo is in motion.
/// @param  
/// @return  true if servo in motion, false otherwise.
uint8_t PCA9685_servo::isMoving(void)
{
	return _isMoving;
}

uint64_t PCA9685_servo::getTConstantDuration(void)
{
	return _TConstantDuration;
}

uint64_t PCA9685_servo::getSConstantPeriod(void)
{
	return _SConstantPeriod;
}

void PCA9685_servo::setMinAngle(int8_t angle)
{
	if (_currentAngle < angle)
	{
		_currentAngle = angle;
		setPosition(_currentAngle);
	}
	
	_minAngle = angle;
	return;
}

void PCA9685_servo::setMidAngle(int8_t angle)
{
	if ((angle >= _minAngle) && (angle <= _maxAngle))
	{
		_midAngle = angle;
	}
	
	return;
}

void PCA9685_servo::setMaxAngle(int8_t angle)
{
	if (_currentAngle > angle)
	{
		_currentAngle = angle;
		setPosition(_currentAngle);
	}
	_maxAngle = angle;
	return;
}

/// @brief Set movement type.
/// @param mode mode type {MODE_FAST, MODE_TCONSTANT, MODE_SCONSTANT} 
void PCA9685_servo::setMode(uint8_t mode)
{
	_mode = mode;
	return;
}

/// @brief Set duration of movement in MODE_TCONSTANT mode, takes effect only after setPosition() is called.
/// @param TConstantDuration duration of movement in microseconds
void PCA9685_servo::setTConstantDuration(uint64_t TConstantDuration)
{
	_TConstantDuration = TConstantDuration;
	return;
}

/// @brief Set interval time between each step (changing servo angle) in MODE_SCONSTANT mode, takes effect only after setPosition() is called.
/// @param SConstantPeriod interval time in microseconds
void PCA9685_servo::setSConstantPeriod(uint64_t SConstantPeriod)	
{
	_SConstantPeriod = SConstantPeriod;
	return;
}

/// @brief Set the pin number on the PCA9685 the servo is connected to. 
/// @param address pin number (0-15)
void PCA9685_servo::setAddress(uint16_t address)
{
	_address = address;
	return;
}

/// @brief Changes the speed of current movement, only use if you want to change the speed in MODE_TCONSTANT during the movement.
///			Otherwise use setTConstantDuration
/// @param TConstantDuration is the duration of the movement in microseconds
void PCA9685_servo::changeSpeedConstT(uint64_t TConstantDuration)
{
	// _targetAngle = Angle;	// where we wish to end up
	uint64_t AngleDelta = abs(_targetAngle - _currentAngle);		// how many degrees we need to move
	_TConstantDuration = TConstantDuration;
	// how long between 1 degree movements?
	_TConstantPeriod = _TConstantDuration / AngleDelta;
}

/// @brief Changes the angular velocity used in MODE_SCONSTANT and MODE_TCONSTANT.
/// @param ang_vel is the angular velocity in degrees/s
void PCA9685_servo::setAngularVelocity(double angVel)
{
	double time_interval_s = 1 / angVel;
	_SConstantPeriod = time_interval_s * 1e6;
	_TConstantPeriod = _SConstantPeriod;
}

/// @brief Stop the servo.
void PCA9685_servo::stop()
{
	_targetAngle = _currentAngle;
}
