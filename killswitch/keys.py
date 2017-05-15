from enum import Enum

ASCII_EOT = 0x03
ASCII_GS = 0x29
ASCII_RS = 0x30


class Key(Enum):
    # Arduino Keyboard
    LEFT_CTRL = 0x80
    LEFT_SHIFT = 0x81
    LEFT_ALT = 0x82
    LEFT_GUI = 0x83
    RIGHT_CTRL = 0x84
    RIGHT_SHIFT = 0x85
    RIGHT_ALT = 0x86
    RIGHT_GUI = 0x87
    UP_ARROW = 0xDA
    DOWN_ARROW = 0xD9
    LEFT_ARROW = 0xD8
    RIGHT_ARROW = 0xD7
    BACKSPACE = 0xB2
    TAB = 0xB3
    RETURN = 0xB0
    ESC = 0xB1
    INSERT = 0xD1
    DELETE = 0xD4
    PAGE_UP = 0xD3
    PAGE_DOWN = 0xD6
    HOME = 0xD2
    END = 0xD5
    CAPS_LOCK = 0xC1
    F1 = 0xC2
    F2 = 0xC3
    F3 = 0xC4
    F4 = 0xC5
    F5 = 0xC6
    F6 = 0xC7
    F7 = 0xC8
    F8 = 0xC9
    F9 = 0xCA
    F10 = 0xCB
    F11 = 0xCC
    F12 = 0xCD

    # Killswitch Control Chars
    PAUSE = ASCII_RS
    END_OF_SHORTCUT = ASCII_EOT
    RELEASE_ALL_KEYS = ASCII_GS

    def __int__(self):
        return self.value

    def __str__(self):
        return self.name.replace('_', ' ').title()

    @staticmethod
    def lookup(key):
        try:
            return Key[key]
        except KeyError:
            return None
