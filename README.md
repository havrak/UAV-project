# UAV project

This is project for my (hopefully) final year of highschool.

Main purpose is to create UAV platform based on Raspberry Pi.
However as I have never done anything like this I'm not sure how far will I get.

Alongside code for the drone and software to control it this repo includes various stl files to be 3d printed.


# Components

| Component                                     | Description                                        |
|-----------------------------------------------|----------------------------------------------------|
| X-UAV Mini Talon (PNP)                        | Basic chassis of the UAV                           |
| 1200kv motor 					                        | Main motor of the drone                            |
| Beatles 40A ESC                               | Electronic speed control                           |
| 4 servo motor                                 | Servos to control all control surfaces             |
| Wifi Adapter                                  | 6Dbi Wi-Fi adapter to extend range of rpi0 Wi-Fi   |
| Raspberry Pi Zero 2                           | Main computing unit of the UAV                     |

* whole drone runs off from 4S 14.8V battery with XT60 plug, thus packet of XT60 F/M plugs is also necessary
* servos and Pi have seperete supply of power thus two step down converters and XT60 splitter
* On PC side I'm using Alfa Wi-Fi adapter with 25dBi directional antenna

# Libraries and toolkits
* [WT901B library](https://github.com/havrak/Raspberry-JY901-Serial-I2C)
* [INA226 library](https://github.com/havrak/raspberry-pi-ina226)
* [PCA9685 library](https://github.com/havrak/PCA9685-rpi)
* [inih (INI Not Invented Here)](https://github.com/benhoyt/inih)
* OpenCV
* Gtk3
* crp, fmt
* WiringPi (used by some libraries above) -- need to move to gpio library

# Photos

### UAV Photos

![whole plane](./photos/whole_plane.jpg)

![body detail](./photos/body.jpg)

### Controlling unit

![component 1](./photos/detail.jpg)

![component 2](./photos/detail_2.jpg)

### Circuit schema

![circuit](./photos/schema.png)


# Credits
* textures for desktop client originate from [marek-cel/QFlightinstruments](https://github.com/marek-cel/QFlightinstruments)


