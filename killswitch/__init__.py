import string
import serial
from serial.tools.list_ports import comports

from killswitch.keys import Key

CMD_STATUS = 0xFF


class Combination(object):
    def __init__(self, *args):
        self.buffer = list()
        for keypress in args:
            self.add(keypress)

    def add(self, buffer):
        if type(buffer) == str and Key.lookup(buffer):
            buffer = Key[buffer]

        if type(buffer) == Key or (type(buffer) == int and chr(buffer) in string.printable):
            return self.buffer.append(buffer)
        if type(buffer) == str and len(buffer) == 1:
            return self.buffer.append(ord(buffer))

        raise ValueError('Only a character or a one-byte integer is allowed as a Key, not %s' % type(buffer))

    def to_hex(self):
        return list(map(int, self.buffer)) + [keys.ASCII_EOT]

    def __str__(self):
        shortcut_str = ''
        for k in self.buffer:
            if k is Key.RELEASE_ALL_KEYS:
                shortcut_str += ', '
            elif k in [Key.END_OF_SHORTCUT, Key.PAUSE]:
                continue
            else:
                shortcut_str += (len(shortcut_str) > 0 and '+' or '')
                if type(k) is Key:
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
        self.send(CMD_STATUS)
        r = self.get_response()
        pincount, shortcut_size = map(int, r[:2])
        shortcuts = r[2:]

        combinations = []
        for i in range(1, pincount+1):
            buffer = shortcuts[:i*shortcut_size]
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
            line = self.get_response_line()
        return line.strip(b'\r\n')

