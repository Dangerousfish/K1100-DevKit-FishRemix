/*
    SD card read/write

    This example shows how to read and write data to and from an SD card file
    The circuit:
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

    created   Nov 2010
    by David A. Mellis
    modified 9 Apr 2012
    by Tom Igoe

    This example code is in the public domain.

*/
#include <Seeed_Arduino_FS.h>
#include <Seeed_SFUD.h>

#define SERIAL Serial

#define DEV SFUD

void setup() {
    SERIAL.begin(115200);

    while (!SERIAL) {};

    while (!DEV.begin(104000000UL)) {
        SERIAL.println("Card Mount Failed");
        return;
    }

    SERIAL.println("initialization done.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.

    File RootWrite = DEV.open("/hello.txt", "w");
    // File RootWrite = DEV.open("/hello.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (RootWrite) {
        SERIAL.print("Writing to hello.txt...");
        RootWrite.println("hello 1, 2, 3.");
        // close the file:
        RootWrite.close();
        SERIAL.println("done.");
    } else {
        // if the file didn't open, print an error:
        SERIAL.println("error opening hello.txt");
    }
    
    // re-open the file for reading:
    File RootRead= DEV.open("/hello.txt");
    if (RootRead) {
        SERIAL.println("hello.txt:");

        // read from the file until there's nothing else in it:
        while (RootRead.available()) {
            SERIAL.write(RootRead.read());
        }
        // close the file:
        RootRead.close();
    } else {
        // if the file didn't open, print an error:
        SERIAL.println("error opening hello.txt");
    }
}

void loop() {
    // nothing happens after setup
}
