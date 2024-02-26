# MDR-2 monochromator automatization

## What is it

Here you can find a code to control automated MDR-2 monochromator. I attached a stepper motor to the monochromator and now it can be positioned using PC instead of old-fashion manual control.

Here I have only the software without all the other stuff (circuitry and mechanic stuff). Arduino sketch and Free Pascal unit for MDR-2 monochromator.

The software has been tested for errors, stability and speed under Linux.

## Install

### Arduino

1. Make directory "MDR_2" in the sketchbook directory on your PC (it is "Arduino" by default, check the preferences in Arduino IDE).
2. Put the MDR_2.ino into "MDR_2" directory.
3. Open Arduino IDE and set your Arduino board
4. Compile the sketch and upload it to the board.

### Free Pascal

1. Download MDR.pas and MDR-2.ini
2. Get ArduinoDevice.pas from [my Arduino device repository](https://github.com/serhiykobyakov/Arduino_device_FPC) 
3. Put in your directory jedi.inc, synafpc.pas, synaser.pas, synautil.pas from [Synapse repository](http://synapse.ararat.cz/doku.php/download)
4. Use repository info and and see the comments in files to get it work

## Contact
For reporting [bugs, suggestions, patches](https://github.com/serhiykobyakov/MDR-2-monochromator/issues)

## License
The project is licensed under the [MIT license](https://github.com/serhiykobyakov/MDR-2-monochromator/blob/main/LICENSE)
