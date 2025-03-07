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

obj_is_tracking = False
ready_to_grab = False

commands = {"endl" : 'e', "angles" : 'a',
  "pause" : 'p', "play" : 'c', "grab" : 'g', "default_pos" : ' ', "write_pos" : 'w',
  "up" : 'u', "down" : 'd', "left" : 'l', "right" : 'r', "forward" : 'f', "backward" : 'b',
  "ard_ready" : "ready"}

def send_to_arduino(command):
    #arduino.write(bytes(command, 'utf-8'))
    time.sleep(0.05)

def get_from_arduino():
    time.sleep(0.05)
    #data = arduino.readline()
    #return data

def search():
    if obj_is_tracking:
        return
    if get_from_arduino != commands["ard_ready"]:
        return
    
    send_to_arduino(commands["right"])
    time.sleep(0.05)

def move_to_object(coords):
    if get_from_arduino != commands["ard_ready"]:
        return
    if ready_to_grab:
        return
    
    x = coords[0].numpy()
    y = coords[1].numpy()
    w = coords[2].numpy()
    h = coords[3].numpy()

    # if obj is close and centered start grabbing
    if (h >= 0.8) and (abs(x - 0.5) <= 0.1):
        ready_to_grab = True
        return

    # centering
    if (x - 0.5) > 0.1:
        send_to_arduino(commands["right"])
        return
    if (x - 0.5) < 0.1:
        send_to_arduino(commands["left"])
        return
    if (y - 0.5) > 0.1:
        send_to_arduino(commands["down"])
        return
    if (y - 0.5) < 0.1:
        send_to_arduino(commands["up"])
        return

    # moving closer
    if h < 0.8:
        send_to_arduino(commands["forward"])
        return

def grab_and_place_object(obj_number):
    if get_from_arduino != commands["ard_ready"]:
        return
    if not ready_to_grab:
        return

    send_to_arduino(commands["grab"]) # open grabber
    time.sleep(1.0)
    
    # move closer blindly
    wait_arduino_answer()
    
    send_to_arduino(commands["grab"]) # close grabber
    time.sleep(1.0)

    if obj_number == 0:
        #move to position 0
        pass
    if obj_number == 1:
        #move to position 1
        pass
    if obj_number == 2:
        #move to position 2
        pass 
    wait_arduino_answer()

    send_to_arduino(commands["grab"]) # open grabber
    time.sleep(1.0)
    
    # move back a little
    wait_arduino_answer()

    send_to_arduino(commands["grab"]) # close grabber
    time.sleep(1.0)
    
    send_to_arduino(commands["default"])
    time.sleep(0.05)
    obj_is_tracking = False

# обязательно проверить
def wait_arduino_answer():
    while(get_from_arduino != commands["ard_ready"]):
        time.sleep(0.05)

# main
#arduino = serial.Serial(port = 'COM3', baudrate = 115200, timeout = 0.1)
# default positioning - corner left

model = YOLO("<your_location>")
cap = cv2.VideoCapture(0)


while(1): # one move per iteration for correct AI working
    success, frame = cap.read()
    if not success:
        continue
    
    results = model.predict(frame, conf = 0.5, max_det = 1, verbose = False) #1 object per robot's cycle

    cls = tf.squeeze(results.boxes.cls)
    if tf.rank(cls) != 0: # search if 0 objects in frame
        search()
        continue
        
    obj_is_tracking = True
    move_to_object(tf.squeeze(results.boxes.xywhn))
    grab_and_place_object(cls) # full operation here, no AI


    # for human
    annotated_frame = results[0].plot()
    cv2.imshow("YOLOv10", annotated_frame)
    cv2.waitKey(1)
