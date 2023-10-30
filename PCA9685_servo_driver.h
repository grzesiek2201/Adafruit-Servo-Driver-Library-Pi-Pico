#ifndef PCA9685_servo_driver_h
#define PCA9685_servo_driver_h

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <algorithm>
#include <iostream>

// REGISTER ADDRESSES
#define PCA9685_MODE1 0x00      /**< Mode Register 1 */
#define PCA9685_MODE2 0x01      /**< Mode Register 2 */
#define PCA9685_SUBADR1 0x02    /**< I2C-bus subaddress 1 */
#define PCA9685_SUBADR2 0x03    /**< I2C-bus subaddress 2 */
#define PCA9685_SUBADR3 0x04    /**< I2C-bus subaddress 3 */
#define PCA9685_ALLCALLADR 0x05 /**< LED All Call I2C-bus address */
#define PCA9685_LED0_ON_L 0x06  /**< LED0 on tick, low byte*/
#define PCA9685_LED0_ON_H 0x07  /**< LED0 on tick, high byte*/
#define PCA9685_LED0_OFF_L 0x08 /**< LED0 off tick, low byte */
#define PCA9685_LED0_OFF_H 0x09 /**< LED0 off tick, high byte */
// etc all 16:  LED15_OFF_H 0x45
#define PCA9685_ALLLED_ON_L 0xFA  /**< load all the LEDn_ON registers, low */
#define PCA9685_ALLLED_ON_H 0xFB  /**< load all the LEDn_ON registers, high */
#define PCA9685_ALLLED_OFF_L 0xFC /**< load all the LEDn_OFF registers, low */
#define PCA9685_ALLLED_OFF_H 0xFD /**< load all the LEDn_OFF registers,high */
#define PCA9685_PRESCALE 0xFE     /**< Prescaler for PWM output frequency */
#define PCA9685_TESTMODE 0xFF     /**< defines the test mode to be entered */

// MODE1 bits
#define MODE1_ALLCAL 0x01  /**< respond to LED All Call I2C-bus address */
#define MODE1_SUB3 0x02    /**< respond to I2C-bus subaddress 3 */
#define MODE1_SUB2 0x04    /**< respond to I2C-bus subaddress 2 */
#define MODE1_SUB1 0x08    /**< respond to I2C-bus subaddress 1 */
#define MODE1_SLEEP 0x10   /**< Low power mode. Oscillator off */
#define MODE1_AI 0x20      /**< Auto-Increment enabled */
#define MODE1_EXTCLK 0x40  /**< Use EXTCLK pin clock */
#define MODE1_RESTART 0x80 /**< Restart enabled */
// MODE2 bits
#define MODE2_OUTNE_0 0x01 /**< Active LOW output enable input */
#define MODE2_OUTNE_1 0x02 /**< Active LOW output enable input - high impedience */
#define MODE2_OUTDRV 0x04 /**< totem pole structure vs open-drain */
#define MODE2_OCH 0x08    /**< Outputs change on ACK vs STOP */
#define MODE2_INVRT 0x10  /**< Output logic state inverted */

#define PCA9685_I2C_ADDRESS 0x40      /**< Default PCA9685 I2C Slave Address */
#define FREQUENCY_OSCILLATOR 25000000 /**< Int. osc. frequency in datasheet */

#define PCA9685_PRESCALE_MIN 3   /**< minimum prescale value */
#define PCA9685_PRESCALE_MAX 255 /**< maximum prescale value */


class PCA9685_servo_driver
{
public:
	PCA9685_servo_driver(i2c_inst_t *i2c=i2c0, uint SDA=0, uint SCL=1, uint8_t i2c_address=PCA9685_I2C_ADDRESS);	
	// PCA9685 config functions
	void begin(uint baudrate = 100000);
	void reset();
	void sleep();
	void wakeup();
	void setExtClk(uint8_t prescale);
	void setPWMFreq(float freq);
	void setOutputMode(bool totempole);
	uint16_t getPWM(uint8_t num, bool off = false);
	void setPWM(uint8_t channel, int16_t off, int16_t on=0);
	void setPin(uint8_t num, uint16_t val, bool invert=false);
	uint8_t readPrescale(void);

	void setOscillatorFrequency(uint32_t freq);
	uint32_t getOscillatorFrequency(void);

	long map(long x, long in_min, long in_max, long out_min, long out_max);

private:
	i2c_inst_t *_i2c;
	uint _SDA;
	uint _SCL;
	uint8_t _i2c_address;
	uint32_t _oscillator_freq;

	void writeRegister(uint8_t reg, uint8_t value);
	uint8_t readRegister(uint8_t reg);
};

#endif
