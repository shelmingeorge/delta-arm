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
search_movements = 0
max_search_mov = 25 # проверить

error_margin = 0.1
max_height = 0.8

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
    #data = data.decode('utf-8')
    #return data[:-2]

def search():
    if search_movements > max_search_mov:
        send_to_arduino(commands["default_pos"])
        search_movements = 0
        time.sleep(1)
        return
    if obj_is_tracking:
        return
    if get_from_arduino != commands["ard_ready"]:
        return
    
    send_to_arduino(commands["right"])
    search_movements += 1
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
    if (h >= max_height) and (abs(x - 0.5) <= error_margin):
        ready_to_grab = True
        return

    # centering
    if (x - 0.5) > error_margin:
        send_to_arduino(commands["right"])
        return
    if (x - 0.5) < -error_margin:
        send_to_arduino(commands["left"])
        return
    if (y - 0.5) > error_margin:
        send_to_arduino(commands["down"])
        return
    if (y - 0.5) < -error_margin:
        send_to_arduino(commands["up"])
        return

    # moving closer
    if h < max_height:
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

    go_to_coords(obj_number)
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

def wait_arduino_answer():
    while(get_from_arduino != commands["ard_ready"]):
        time.sleep(0.05)

def go_to_coords(obj_number):
    #send_to arduino()
    # тут обращение к place_coords[obj_number]
    pass

# main
#arduino = serial.Serial(port = 'COM3', baudrate = 115200, timeout = 0.1)
# default positioning - corner left

model = YOLO("<>ur_loc")
cap = cv2.VideoCapture(1)


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
