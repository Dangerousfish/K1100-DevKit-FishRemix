![](https://lh3.googleusercontent.com/EEbDC9FdasF8s3iLihuyjluvaMzFgwpuJ86ufD7u7NC-W0UEsnlTmOrpIezgrSYoG2-G4NJqgz9zuQHlUSEld7ig3VEhmmdE0VI2Mfp66e7jYSIhdlrNMviLHrBugtGA9HMuNPdkaE-i5aHahOkNifrUG6blBO_6ktR-7-ggCcWJCZH3KAJ475IYRw)

Credit:
-------

<https://arduino-pico.readthedocs.io/_/downloads/en/latest/pdf/>

<https://github.com/Seeed-Studio/SenseCraft>

<https://docs.platformio.org/>

Requirements:
-------------

### Hardware:

Raspberry Pi Pico (rp2040, xiao rp2040, pico-w)

<https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html>

<https://thepihut.com/collections/featured-products/products/raspberry-pi-pico>

Grove Wio-E5 - <https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/>

### Software

Visual Studio Code - <https://code.visualstudio.com/>

Guide:
------

Setting up your board and platform

1.  Install Visual Studio Code.

2.  Download the PlatformIO extension for Visual Studio Code.

3.  Restart Visual Studio Code.

4.  Connect your RP2040 via USB and ensure this is recognised within devices.![](https://lh3.googleusercontent.com/BKcTg5C8VYRCuXZbfPbYrbCyClI0kSfeosEB7qxXuWFFQZVH6qG0Ex5kuU1HrLA4WikjHt8ZVpG7jScCygmwqTQZWiFZ68WyKhYf5iyyRmvpmg-xMOM1MCPP4qDFMn7uzZJssYVhmIa-awkAJFDHr6Wp8lkBG8LN2l7rJxTESj9t_fKV1v9cyvwZvQ)

5.  Navigate to platforms > Advanced install > Install using:

https://github.com/maxgerhardt/platform-raspberrypi.git

6.  Still within platforms, install the "Linux x86_64" platform.

7.  Create a new project from within PlatformIO using the settings:

```
-  Platform: Raspberry Pi Pico

-  Board: pico

-  Framework: Arduino
```

8.  Save your edits and wait a few minutes for the project to load.

9.  Once created, the platformio.ini should automatically open. When it does, delete the contents and replace them with this:

```
[env:pico]

platform = https://github.com/maxgerhardt/platform-raspberrypi.git

platform_packages = https://github.com/maxgerhardt/platform-raspberrypi.git

board = seeed_xiao_rp2040

framework = arduino

board_build.core = earlephilhower

lib_deps =

   <https://github.com/Seeed-Studio/Seeed_Arduino_LoRaE5> #Wio-E5

   <https://github.com/knolleary/pubsubclient> #MQTT

   <https://github.com/Seeed-Studio/Arduino_Software_I2C> #Grove Port

   SPI

   Wire

   SoftwareSerial
```

10.  Save your edits and close the platformio.ini file

11.  Using the PlatformIO Tools, build your project (This will take some time, it's downloading files for your firmware)

THIS WILL TAKE SOME TIME - GO AND MAKE A COFFEE

12.  Once the build has completed, you can now upload your project by placing your rp2040 in boot mode and pressing the upload button in PlatformIO.
