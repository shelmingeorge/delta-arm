import serial
import time

speed = 9600
num = "some_data"

def write_serial(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)

def read_serial():
    data = arduino.readline()
    return data

def write_read_serial(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)
    data = arduino.readline()
    return data

arduino = serial.Serial(port = '/dev/ttyACM0', baudrate = speed, timeout = .1)

while 1:
    num = input("Enter: ")
    value = write_read_serial(num)
    print(value)
