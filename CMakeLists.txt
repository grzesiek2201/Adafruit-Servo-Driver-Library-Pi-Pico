add_library(Adafruit-Servo-Driver-Library-Pi-Pico
			PCA9685_servo_driver.cpp
			PCA9685_servo.cpp
			)

target_link_libraries(Adafruit-Servo-Driver-Library-Pi-Pico PUBLIC
		pico_stdlib
		hardware_i2c
		)
