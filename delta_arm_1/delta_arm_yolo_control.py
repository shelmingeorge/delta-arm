from ultralytics import YOLO
import cv2
import tensorflow as tf
import numpy as np
import serial
import time

classes = ["pawn", "cylinder", "cube"]
place_coords = [[0, 0, 0], 
                   [0, 0, 0], 
                   [0, 0, 0]] # fi, dist, height

commands = {"endl" : 'e', "angles" : 'a',
  "pause" : 'p', "play" : 'c', "grab" : 'g', "default_pos" : ' ', "write_pos" : 'w',
  "up" : 'u', "down" : 'd', "left" : 'l', "right" : 'r', "forward" : 'f', "backward" : 'b'}

def send_to_arduino(command):
    #arduino.write(bytes(command, 'utf-8'))
    time.sleep(0.05)

def get_from_arduino():
    time.sleep(0.05)
    #data = arduino.readline()
    #return data

def search():
    send_to_arduino(commands["right"])
    time.sleep(0.30)

def move_to_obj(coords):
    x = coords[0].numpy()
    y = coords[1].numpy()
    w = coords[2].numpy()
    h = coords[3].numpy()

def place_object(obj_number):
    if obj_number == 0:
        #move in position 0
        pass
    if obj_number == 1:
        #move in position 1
        pass
    if obj_number == 2:
        #move in position 2
        pass
    #place oblect
    #move back a little
    
    send_to_arduino(commands["default"])
    time.sleep(1.0)

# main
#arduino = serial.Serial(port = 'COM3', baudrate = 115200, timeout = 0.1)
#defolt positioning - corner left

model = YOLO("<your_location>")
cap = cv2.VideoCapture(0)


while(1):
    success, frame = cap.read()
    if not success:
        continue
    
    results = model.predict(frame, conf = 0.5, max_det = 1, verbose = False) #1 object per robot's cycle
    for r in results:
        cls = tf.squeeze(r.boxes.cls)
        if tf.rank(cls) != 0: #search if 0 objects in frame
            search()
            continue
        
        move_to_obj(tf.squeeze(r.boxes.xywhn))
        place_object(cls)

        annotated_frame = results[0].plot()
        cv2.imshow("YOLOv10", annotated_frame)
        cv2.waitKey(1)
