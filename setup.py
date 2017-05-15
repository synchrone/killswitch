from setuptools import setup

setup(
    name='killswitch',
    version='0.1',
    py_modules=['killswitch'],
    install_requires=[
        'Click',
        'pyserial'
    ],
    entry_points='''
        [console_scripts]
        killswitch=killswitch.cli:main
    ''',
)