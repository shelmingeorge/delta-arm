import serial
import time

num = "";

def write_read(x):
    #arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)
    data = arduino.readline()
    data = data.decode('utf-8')
    return data[:-2]

arduino = serial.Serial(port = 'COM3', baudrate = 115200, timeout = .1)


while 1:
    #num = input("Enter: ")
    value = write_read(num)
    if value == "ready":
        print("yay")
