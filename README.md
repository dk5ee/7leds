
## LEDs with Software PWM

X-axis: Time; Y-axis: PWM-value

#### Normal Hardware PWM
![Normal PWM](/images/distribution_standardPWM.png)
#### SoftwarePWM, with prime distribution
![Normal PWM](/images/distribution_prime107.png)
#### Delta Modulation/DPCM distribution
![Normal PWM](/images/distribution_DPCM.png)

### Content of Repository

This repository comprises of this parts: 
1. Code for AVR Microcontroller to light up attached LEDs
2. Code for sending commands to the Microcontroller
3. Extra design files

### Microcontroller Code

Atmega328 modules with optiboot/arduino bootlader and USB serial are used.

Not quite PWM, as the uptime of the outputs are distributed over time.

the main.c in "code" uses 8 bit generation with "prime"-algorithm for seven LEDs/outputs

for comparison:

the main.c in "code2" uses 8 bit generation with "synthetic division"-algorithm for seven LEDs/outputs, which is equivalent to Delta Modulation / Differential pulse-code modulation (DPCM)

for further experiments:

the main.c in "code3" uses 16 bit generation with  "prime"-algorithm for just one LED/output

### Command Code

A Python Project in "webserver" allows commanding the controller over webinterface