# killswitch
Arduino Leonardo custom shortcut-keyboard project.

## Hardware
I am using [DF Robot Beetle](https://www.dx.com/p/jtron-mini-controller-module-black-works-with-official-arduino-board-326521) aka [Jtron Beetle](https://www.dx.com/p/jtron-mini-controller-module-black-works-with-official-arduino-board-326521).

Simply put a non-locking switch between the GPIOs (A0, A1, A2) and GND. 

The device emulates HID Keyboard in hardware.

Flash it with killswitch.ino using [Arduino IDE](https://www.arduino.cc/en/Main/Software)  

## Configuration
 
```
pip install -e . # in a virtualenv, to avoid system bloat
killswitch keys # get information about available keys
killswitch program --pin <0-based pin number> <space-separated key combination>
killswitch status # check device memory to verify
```

## Serial comm format:
`<pin-number><keys><ASCII_EOT>`, where:

`<pin-number>` is a (char) later casted to (int), so 0x00 for an 0-index element of the *pins* array

`<keys>` are straight ASCII bytes, plus [Arduino KeyboardModifiers](https://www.arduino.cc/en/Reference/KeyboardModifiers), and ASCII Record Separator (0x30) indicates a 200ms pause in the combination

`<ASCII_EOT>` is 0x03 for End Of Transmission

#### Example key combinations for serial re-programming: (moserial HEX format)

Ctrl+Alt+l (hard coded as default): 008082306C03
