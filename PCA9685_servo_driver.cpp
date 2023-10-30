#include "PCA9685_servo_driver.h"

/// @brief PCA9685_servo_driver constructor.
/// @param i2c i2c_inst_t pointer, for Pi Pico usually just 'i2c0' or 'i2c1', depending on the i2c interface used
/// @param SDA pin number of the SDA connection on the microcontroller 
/// @param SCL pin number of the SCL connection on the microcontroller
/// @param i2c_address I2c address of the PCA9685, default is 0x40
PCA9685_servo_driver::PCA9685_servo_driver(i2c_inst_t *i2c, uint SDA, uint SCL, uint8_t i2c_address) :
	_i2c{i2c}, _SDA{SDA}, _SCL{SCL}, _i2c_address{i2c_address}
{}

/// @brief Setups the I2C interface and hardware
/// @param baudrate baudrate for the I2C interface
void PCA9685_servo_driver::begin(uint baudrate)
{
	// initialise I2C
	i2c_init(_i2c, baudrate);
	gpio_set_function(_SDA, GPIO_FUNC_I2C);
	gpio_set_function(_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(_SDA);
	gpio_pull_up(_SCL);
	
	// configure the PCA9685 for driving servos ()
	writeRegister(PCA9685_MODE1, 0b10100000);	// enable restart, use internal clock, enable auto-increment, normal mode (not sleeping)
	writeRegister(PCA9685_MODE2, 0b00000100);	// (these are defaults) output logic state not inverted, outputs change on STOP, output are totem-pole
	
	// set the default internal frequency
	setOscillatorFrequency(FREQUENCY_OSCILLATOR);

	setPWMFreq(50);

	return;
}

/// @brief Write to a value to a specified register.
/// @param reg where to write
/// @param value what to write
void PCA9685_servo_driver::writeRegister(uint8_t reg, uint8_t value)
{
	uint8_t D[2];
	
	D[0] = reg;
	D[1] = value;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 2, false);	// write register then value
	
	return;
}

/// @brief Read one byte from a register.
/// @param reg where to read from
/// @return uint8_t return value from register
uint8_t PCA9685_servo_driver::readRegister(uint8_t reg)
{
	uint8_t D[1] = {reg};
	
	i2c_write_blocking(_i2c, _i2c_address, D, 1, false);	// write register
	i2c_read_blocking(_i2c, _i2c_address, D, 1, false);		// read value
	
	return D[0];
}

// set a position based on a supplied PWM value (assumes "on" is at zero, offset applied here)

/// @brief Set a positon based on a supplied PWM value (assumes "on" is at zero, offset applied here)
/// @param channel servo channel on the PCA9685
/// @param PWM PWM value reflecting the angle
void PCA9685_servo_driver::setPWM(uint8_t channel, int16_t off, int16_t on)
{
	uint8_t D[5];
	
	uint16_t ChannelOffset = channel * 10;		// adds 0-160 to the counter values

	uint16_t ChannelOn = 0 + ChannelOffset;
	uint16_t ChannelOff = off + ChannelOffset;
	
	// divide the PWM into 2, 8bit words
	D[0] = 0x06 + (4 * channel);
	D[1] = (0x00FF & ChannelOn);
	D[2] = (0xFF00 & ChannelOn) >> 8;
	D[3] = (0x00FF & ChannelOff);
	D[4] = (0xFF00 & ChannelOff) >> 8;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 5, false);
	
	return;
}

/// @brief Utility function
/// @param x 
/// @param in_min 
/// @param in_max 
/// @param out_min 
/// @param out_max 
/// @return 
long PCA9685_servo_driver::map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min)/(in_max - in_min) + out_min;
}

/// @brief Sends a reset command to the PCA9685 chip over I2C
void PCA9685_servo_driver::reset() 
{
	writeRegister(PCA9685_MODE1, MODE1_RESTART);
	sleep_ms(10);
}

/// @brief Puts board into sleep mode
void PCA9685_servo_driver::sleep() 
{
	uint8_t awake = readRegister(PCA9685_MODE1);
	uint8_t sleep = awake | MODE1_SLEEP; // set sleep bit high
	writeRegister(PCA9685_MODE1, sleep);
	sleep_ms(5); // wait until cycle ends for sleep to be active
}

/// @brief Wakes board from sleep
void PCA9685_servo_driver::wakeup() 
{
	uint8_t sleep = readRegister(PCA9685_MODE1);
	uint8_t wakeup = sleep & ~MODE1_SLEEP; // set sleep bit low
	writeRegister(PCA9685_MODE1, wakeup);
}

/// @brief  Sets EXTCLK pin to use the external clock
/// @param  prescale Configures the prescale value to be used by the external clock
void PCA9685_servo_driver::setExtClk(uint8_t prescale) 
{
  uint8_t oldmode = readRegister(PCA9685_MODE1);
  uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
  writeRegister(PCA9685_MODE1, newmode); // go to sleep, turn off internal oscillator

  // This sets both the SLEEP and EXTCLK bits of the MODE1 register to switch to
  // use the external clock.
  writeRegister(PCA9685_MODE1, (newmode |= MODE1_EXTCLK));

  writeRegister(PCA9685_PRESCALE, prescale); // set the prescaler

  sleep_ms(5);
  // clear the SLEEP bit to start
  writeRegister(PCA9685_MODE1, (newmode & ~MODE1_SLEEP) | MODE1_RESTART | MODE1_AI);
}

/// @brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
/// @param  freq Floating point frequency that we will attempt to match
void PCA9685_servo_driver::setPWMFreq(float freq) 
{
	// Range output modulation frequency is dependant on oscillator
	if (freq < 1)
		freq = 1;
	if (freq > 3500)
		freq = 3500; // Datasheet limit is 3052=50MHz/(4*4096)

	float prescaleval = ((_oscillator_freq / (freq * 4096.0)) + 0.5) - 1;
	if (prescaleval < PCA9685_PRESCALE_MIN)
		prescaleval = PCA9685_PRESCALE_MIN;
	if (prescaleval > PCA9685_PRESCALE_MAX)
		prescaleval = PCA9685_PRESCALE_MAX;
	uint8_t prescale = (uint8_t)prescaleval;

	uint8_t oldmode = readRegister(PCA9685_MODE1);
	uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
	writeRegister(PCA9685_MODE1, newmode);                             // go to sleep
	writeRegister(PCA9685_PRESCALE, prescale); // set the prescaler
	writeRegister(PCA9685_MODE1, oldmode);
	sleep_us(500);
	// This sets the MODE1 register to turn on auto increment.
	writeRegister(PCA9685_MODE1, oldmode | MODE1_RESTART | MODE1_AI);
}

/// @brief  Sets the output mode of the PCA9685 to either
/// 	open drain or push pull / totempole.
/// 	Warning: LEDs with integrated zener diodes should
/// 	only be driven in open drain mode.
/// @param  totempole Totempole if true, open drain if false.
void PCA9685_servo_driver::setOutputMode(bool totempole)
{
  uint8_t oldmode = readRegister(PCA9685_MODE2);
  uint8_t newmode;
  if (totempole) {
    newmode = oldmode | MODE2_OUTDRV;
  } else {
    newmode = oldmode & ~MODE2_OUTDRV;
  }
  writeRegister(PCA9685_MODE2, newmode);
}

/// @brief  Reads set Prescale from PCA9685
/// @return prescale value
uint8_t PCA9685_servo_driver::readPrescale(void) 
{
	return readRegister(PCA9685_PRESCALE);
}

/// @brief  Gets the PWM output of one of the PCA9685 pins
/// @param  num One of the PWM output pins, from 0 to 15
/// @param  off If true, returns PWM OFF value, otherwise PWM ON
/// @return requested PWM output value
uint16_t PCA9685_servo_driver::getPWM(uint8_t num, bool off) 
{
	uint8_t buffer[2] = {uint8_t(PCA9685_LED0_ON_L + 4 * num), 0};
	if(off)
		buffer[0] += 2;
	i2c_write_blocking(_i2c, _i2c_address, buffer, 1, false);
	i2c_read_blocking(_i2c, _i2c_address, buffer, 2, false);		// read value
	return uint16_t(buffer[0]) | (uint16_t(buffer[1]) << 8);
}

///   @brief  Helper to set pin PWM output. Sets pin without having to deal with
/// 			on/off tick placement and properly handles a zero value as completely off and
/// 			4095 as completely on.  Optional invert parameter supports inverting the
/// 			pulse for sinking to ground.
///   @param  num One of the PWM output pins, from 0 to 15
///   @param  val The number of ticks out of 4096 to be active, should be a value
/// 				from 0 to 4095 inclusive.
///   @param  invert If true, inverts the output, defaults to 'false'
void PCA9685_servo_driver::setPin(uint8_t num, uint16_t val, bool invert) 
{
	// Clamp value between 0 and 4095 inclusive.
	val = std::min(val, (uint16_t)4095);
	if (invert) {
		if (val == 0) {
			// Special value for signal fully on.
			setPWM(num, 4096, 0);
	} else if (val == 4095) {
		// Special value for signal fully off.
		setPWM(num, 0, 4096);
	} else {
		setPWM(num, 0, 4095 - val);
	}
	} else {
	if (val == 4095) {
		// Special value for signal fully on.
		setPWM(num, 4096, 0);
	} else if (val == 0) {
		// Special value for signal fully off.
		setPWM(num, 0, 4096);
	} else {
		setPWM(num, 0, val);
	}
	}
}

/// @brief Getter for the internally tracked oscillator used for freq calculations
/// @return The frequency the PCA9685 thinks it is running at (it cannot introspect)
uint32_t PCA9685_servo_driver::getOscillatorFrequency(void) 
{
	return _oscillator_freq;
}

/// @brief Setter for the internally tracked oscillator used for freq calculations
/// @param freq The frequency the PCA9685 should use for frequency calculations

void PCA9685_servo_driver::setOscillatorFrequency(uint32_t freq) 
{
	_oscillator_freq = freq;
}