add_library(pca9685
			PCA9685_servo_driver.cpp
			PCA9685_servo.cpp
			)

target_link_libraries(pca9685 PUBLIC
		pico_stdlib
		hardware_i2c
		)
