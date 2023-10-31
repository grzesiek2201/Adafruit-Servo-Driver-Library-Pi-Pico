# Adafruit-Servo-Driver-Library-Pi-Pico

This is both a port of Adafruit's Arduino library for the PCA9685 (<a href=https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library/tree/master#adafruit-pca9685-pwm-servo-driver-library->Adafruit Library<a/>) for the Raspberry Pi Pico board and an improvement on the PCA9685 Servo Controller library for Pi Pico (<a href=https://github.com/aleopardstail/PCA9685_Servo_Controller>PCA9685 Servo Controller</a>).

Please note it is not a Micropython library (as those have been released by Adafruit <a href=https://github.com/adafruit/Adafruit_CircuitPython_PCA9685>Circuitpython library</a>), but a library meant to be used with Pi Pico C/C++ SDK.

Please find doxygen docs here <a href=https://grzesiek2201.github.io/Adafruit-Servo-Driver-Library-Pi-Pico>Doxygen docs</a>.

## Setup

### Include the headers
Copy the repository into your project. Include two files in your main cpp file:
- <code>#include <PCA9685_servo_driver.h></code>
- <code>#include <PCA9685_servo.h></code>

### Change your projects CMakeLists.txt
Add a subdirectory:

    add_subdirectory(Adafruit-Servo-Driver-Library-Pi-Pico)    # the argument represents a path to the library directory

Then include the library directory:

    target_include_directories(${PROJECT_NAME}
        PRIVATE Adafruit-Servo-Driver-Library-Pi-Pico    # the argument represents a path to the library directory
    )

Lastly, link the library:

    target_link_libraries(${PROJECT_NAME}
        Adafruit-Servo-Driver-Library-Pi-Pico    # the argument represents the name of the library, as specified in the CMakeLists.txt in the library directory
    )

### Create necessary objects
- Create an object of PCA9685_servo_driver type to serve as your interface with the PCA9685 board.
  <code>PCA9685_servo_driver myController(i2c0);</code>
- Create an object(s) (depending on the number of servos that are being driven) of PCA9685_servo type to keep track of the servo's parameters and movement.
  <code>std::vector<PCA9685_servo> myServo = {PCA9685_servo(&myController, 0, 100, 540)};</code>
- Initialize the servo:
  - set the angle range with <code>setRange()</code>,
  - set operation mode with <code>setMode()</code>,
  - set initial position with <code>setPosition()</code>,
  - set channel number on the board with <code>setAddress()</code>,
  - set duration of movement in constant time mode with <code>setTConstantDuration()</code>.
- In the main() loop, initialize the connection to the PCA9685 with <code>myController.begin();</code>.

### Create control loop
Create a loop in which PCA9685_servo.loop() method will be called for each of the controlled servos. Into the loop() method pass an argument with time elapsed since last loop iteration (to keep track of when to move the servo).

    while(1)
    {
        TEllapsed = TNow - TPrevious;
        for(auto& servo : myServo) servo.loop(TEllapsed);
    }

## How to control the servo
### Movement modes
There are 3 modes:
- <code>MODE_FAST</code> - moves to the set position immediately, with maximum speed (part of the PCA9685_servo class as _TFastDuration),
- <code>MODE_TCONSTANT</code> - the movement takes exactly _TConstantDuration microseconds, independent of the length of the movement,
- <code>MODE_SCONSTANT</code> - the servo moves with a set angular velocity (set with setAngularVelocity()), which means the servo moves one degree after _SConstantPeriod microseconds.

Select a mode with <code>setMode()</code> method.

### Set servo position
In order to move the servo, it first needs to get its position changed with <code>setPosition()</code> or <code>setRelativePosition()</code> methods. The argument passed is the position to move to in degrees. It takes effect immediately.

### Set servo speed or movement time
In order to change the speed, use the <code>setAngularVelocity()</code> method, providing velocity in deg/s. It changes the speed in both MODE_SCONSTANT and MODE_TCONSTANT modes.

In order to change the movement time, use <code>changeSpeedConstT()</code>.

### Loop through the movement
After setting the position, it is necessary to call the <code>loop()</code> method repeatedly in order to execute the movement. It needs an argument that counts the time between the last call to the <code>loop()</code> and now in microseconds.

## Other functions
### Servo status
Each servo keeps track of itself, it's possible to retrieve these states through e.g.:
- <code>getPosition()</code> retrieves current position in degrees,
- <code>isMoving()</code> is the servo moving or not.

### Callbacks
Each servo can call a function on start or end of movement through binding to function pointer. E.g.

    void StopMoveHandler(uint16_t Address) { INTERNAL_LED(0); }
    servo.onStopMove = StopMoveHandler;

## PWM Pulse length count
In order to properly control the servo, its PWM pulse length count has to be determined. The usual range lies within (100-600), but please find the one matching your particular servo. The best way to do this is by manually changing the lower and upper range by a little bit until it matches the (-90, 90) degree range.

### Examples
A simple example can be found in the <code>examples</code> directory alongside a sample CMakeLists.txt file.

### Problems and bugs
At the moment there are not any known problems or bugs, please feel free to report them.
