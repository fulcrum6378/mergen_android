import os
import socket
import sys
from datetime import datetime
from enum import Enum

print("""
USAGE: python android/debugger.py <ANDROID_SERVER_IP>

>> Command codes:
Open the app                  0 (requires ADB)
Start recording               1
Stop recording                2
Close the app                 3
Wipe visual STM               10

@ Note: if you use `open` or `close` actions, the power
optimisation mode is automatically turned on and closes
the app when not required for analysis to save power.
""")

if len(sys.argv) < 2:
    print('Error: please enter the IP address of the Android server as described above!')
    quit()


class Mode(Enum):
    # shell execution actions
    OPEN = 0

    # primary actions
    START = 1
    STOP = 2
    EXIT = 3


# Turns the app on or off.
def turn_app(on: bool):
    ret: int = os.system(
        'adb shell am start -n ir.mahdiparastesh.mergen/ir.mahdiparastesh.mergen.Main' if on else
        'adb shell input keyevent 4'
    )
    if ret != 0:
        global power_optimisation_mode
        power_optimisation_mode = False


# miscellaneous tweaks
power_optimisation_mode: bool = False  # when on, open the app only when needed to save battery of the phone

while True:
    try:
        mode: int = int(input('Debug mode: '))
    except ValueError:
        break

    path = ''
    if mode <= 0:
        match mode:
            case Mode.OPEN.value:
                turn_app(True)
                power_optimisation_mode = True
        continue
    elif mode == Mode.EXIT.value:
        power_optimisation_mode = True

    # connect to Debug.java
    connect_time = datetime.now()
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(10)
    try:
        s.connect((sys.argv[1], 3772))
    except ConnectionRefusedError as cre:
        print(cre)
        continue
    except TimeoutError as toe:
        print(toe)
        continue
    s.sendall(bytes([mode]))
    data: bytes = s.recv(1)
    print('Connection established...', datetime.now() - connect_time)
    if data[0] != 0:
        if data[0] == 255:
            print('Unknown (to server) debug mode', mode)
        elif mode == Mode.START.value and data[0] == 1:
            print('Already started!')
        elif mode == Mode.STOP.value and data[0] == 1:
            print('Already stopped!')
        elif mode == Mode.EXIT.value and data[0] == 1:
            print('Still got work to do!')
        else:
            print('Error code', int(data[0]))
        continue
    print('Done')
