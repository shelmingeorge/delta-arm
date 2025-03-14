import serial
import time

num = "";

def read():
    time.sleep(0.05)
    data = arduino.readline()
    data = data.decode('utf-8')
    return data[:-2]

def write(x):
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)


arduino = serial.Serial(port = 'COM3', baudrate = 115200, timeout = .1)


while 1:
    #num = input("Enter: ")
    value = read()
    if value == "ready":
        print("yay")
