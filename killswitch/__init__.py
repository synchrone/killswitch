import string
import serial
from serial.tools.list_ports import comports

from killswitch.keys import Key

CMD_STATUS = 0xFE


class Combination(object):
    def __init__(self, *args):
        self.buffer = list()
        for keypress in args:
            if keypress != 0x00:
                self.add(keypress)

    def add(self, buffer):
        lookup = Key.lookup(buffer)
        buffer = lookup if lookup else buffer

        if isinstance(buffer, Key) or (isinstance(buffer, int) and chr(buffer) in string.printable):
            return self.buffer.append(buffer)

        raise ValueError('Only a character or a one-byte integer is allowed as a Key, not %s' % type(buffer))

    def to_hex(self):
        return list(map(int, self.buffer)) + [Key.END_OF_SHORTCUT.value]

    def __str__(self):
        shortcut_str = ''
        for k in self.buffer:
            if k is Key.RELEASE_ALL_KEYS:
                shortcut_str += ', '
            elif k in [Key.END_OF_SHORTCUT, Key.PAUSE]:
                continue
            else:
                if len(shortcut_str) > 0:
                    shortcut_str += '+'

                if isinstance(k, Key):
                    shortcut_str += str(k)
                else:
                    shortcut_str += chr(k)
        return shortcut_str


class Device(object):
    def __init__(self, dev=None):
        self.conn = serial.Serial(dev or Device.default(), 9600,
                                  bytesize=serial.EIGHTBITS,
                                  parity=serial.PARITY_NONE,
                                  stopbits=serial.STOPBITS_ONE,
                                  timeout=2)

    @staticmethod
    def all():
        ports = comports()
        return [d for d in ports if d.vid == 0x2341 and d.pid == 0x8036]

    @staticmethod
    def default():
        devs = Device.all()
        if len(devs) == 0:
            raise LookupError('A Killswitch is probably not connected')
        return devs[0].device

    def status(self):
        self.send(bytearray([CMD_STATUS, keys.ASCII_ETX]))
        r = self.get_response()
        pincount, shortcut_size = map(int, r[:2])
        shortcuts = r[2:]

        combinations = []
        for i in range(0, pincount):
            buffer = shortcuts[i*shortcut_size: (i+1)*shortcut_size]
            combinations.append(Combination(*buffer))

        return combinations

    def program(self, pin: int, combination: Combination):
        buffer = bytearray([pin] + combination.to_hex())
        self.send(buffer)
        return str(self.get_response())

    def send(self, data):
        return self.conn.write(data)

    def get_response_line(self):
        return self.conn.read_until(b'\r\n')

    def get_response(self):
        line = self.get_response_line()
        while line[0] == ord('#'):
            # print(line)
            line = self.get_response_line()
        return line.strip(b'\r\n')

