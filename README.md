# killswitch
Arduino Leonardo custom shortcut-keyboard project

# Serial comm format:
`<pin-number><keys><ASCII_EOT>`, where:

`<pin-number>` is a (char) later casted to (int), so 0x00 for an 0-index element of the *pins* array

`<keys>` are straight ASCII bytes, plus [Arduino KeyboardModifiers](https://www.arduino.cc/en/Reference/KeyboardModifiers), and ASCII Record Separator (0x30) indicates a 200ms pause in the combination

`<ASCII_EOT>` is 0x03 for End Of Transmission

# Example key combinations for serial re-programming: (moserial HEX format)

Ctrl+Alt+l (hard coded as default): 008082306C03
