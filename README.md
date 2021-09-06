

# UAV project

This is project for my (hopefully) final year of highschool.

Main purpose is to create UAV platform based on Raspberry Pi.
However as I have never done anything like this I'm not sure how far will I get.

NOTE: Currently I'm awaiting shipment of all the different components.

# Proposed stages of UAV development

I'm too lazy to update this file regularly so it may be out of date from time to time.
Also more detailed roadmap is in my vimwiki, which is is private repo (might move it here and symlink it to vimwiki later).

1. getting the thing to fly
2. prettier client on control PC (map etc., integration on something like CivTAK)
3. basic autopilot (PID controller, capable of flying in straight line)
4. movable camera
5. fully autonomous flying (with exception of landing and taking off)
6. tracking antenna (might just used what was made for ardupilot)
7. more robust control solution -- RC backup, analog video etc.

### Bonus:

1. DIY catapult (looks pretty cool but not mission critical)

# Components

| Component                                     | Description                                                 | Price  |
|-----------------------------------------------|-------------------------------------------------------------|--------|
| X-UAV Mini Talon (PNP)                        | Basic chassis of the UAV along with ESC, Motor and 4 servos | 145$   |
| ublox NEO-7M                                  | GPS module                                                  | 22$    |
| PCA9685                                       | 16 Channel driver for servos                                | 4$     |
| Wifi Adapter                                  | 6Dbi wifi adapter to extend range of rpi0 wifi              | 10$    |
| Raspberry Pi Zero HW                          | Main computing unit of the UAV                              | 40$    |
| DC-DC 24V 12V 9V to 5V (does not have a name) | Step down convertor to lower battery's 14,8V to 5V          | 5$     |
| INA226                                        | Volt/Ampere meter to measure draw from the battery          | 5$     |
| WitMotion WT901B                              | Acc, Gyro, Angle, Mag and Baro meter                        | 25$    |
| Pi Camera (chinese knock-off)                 | camera for rpi0 to get video from the drone                 | 10$    |
| FPC cable (12 inch)                           | longer cable for Pi camera                                  | 2$     |


* whole drone runs off from 4S 14.8V battery with XT60 plug, thus packet of XT60 F/M plugs is also necessary
* servos and Pi have seperete supply of power thus two step down converters and XT60 splitter
* On PC side I'm using Alfa Wi-Fi adapter with 25dBi directional antenna
* STM32 programmer is necessary to programm the chip on WT901B

