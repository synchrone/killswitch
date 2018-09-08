import click

from killswitch import Device, Combination
from killswitch.keys import Key


@click.command()
@click.option('--dev', help='Virtual COM port name for communication')
def status(dev):
    for n, c in enumerate(Device(dev).status()):
        print('Button #{}: {}'.format(n, str(c)))


@click.command()
@click.option('--dev', help='Virtual COM port name for communication')
@click.option('--pin', default=0, help='Button number to program')
@click.argument('keys', nargs=-1)
def program(dev, pin, keys):
    d = Device(dev)
    cmb = Combination(*keys)

    print('Configuring the keyboard combination: %s for button %d' % (cmb, pin))
    response = d.program(pin, cmb)
    print('Device response: '+response)


@click.command()
def keys():
    print('All ASCII characters, plus special keys:')
    for k in Key:
        print(k.name)


@click.group()
def main():
    pass


main.add_command(status)
main.add_command(program)
main.add_command(keys)

if __name__ == '__main__':
    main()
