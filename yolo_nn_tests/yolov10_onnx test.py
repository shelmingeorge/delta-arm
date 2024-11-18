#ctrl+c in terminal for halt
import cv2
#from picamera2 import Picamera2
from ultralytics import YOLO

#model = YOLO("./yolov10n_ncnn_model")
#model = YOLO("./yolov10n.pt")
model = YOLO("./yolov10n.onnx")

cap = cv2.VideoCapture(0)
"""
picam2 = Picamera2()
camera_config = picam2.create_still_configuration(main={"size": (640, 640)}, 
                                                  lores={"size": (256, 256)}, display="lores")
picam2.configure(camera_config)
picam2.start()
"""

while (1):
    ret, im = cap.read()
    #im = picam2.capture_array()
    #cv2.imshow("Camera", im)
    #cv2.waitKey(1)

    #im = cv2.cvtColor(im, cv2.COLOR_BGR2RGB)

    results = model.predict(im, imgsz = 256 #, verbose = False, 
                            , conf = 0.45, max_det = 3)
    
    for r in results:
        annotated_frame = results[0].plot()
        cv2.imshow("YOLOv8 Inference", annotated_frame)
        cv2.waitKey(1)

    
