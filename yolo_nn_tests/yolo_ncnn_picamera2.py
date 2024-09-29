#ctrl+c in terminal for halt
import cv2
from picamera2 import Picamera2
#import tensorflow as tf
from ultralytics import YOLO

#model = YOLO("./yolov8n.pt")
#model.export(format="ncnn")
model = YOLO("./yolov8n_ncnn_model")

picam2 = Picamera2()
camera_config = picam2.create_still_configuration(main={"size": (640, 480)}, 
                                                  lores={"size": (640, 480)}, display="lores")
picam2.configure(camera_config)
picam2.start()

while (1):
    im = picam2.capture_array()
    #cv2.imshow("Camera", im)
    #cv2.waitKey(1)

    im = cv2.cvtColor(im, cv2.COLOR_BGR2RGB)

    results = model.predict(im, verbose = False)
    for r in results:
        annotated_frame = results[0].plot()
        cv2.imshow("YOLOv8 Inference", annotated_frame)
        cv2.waitKey(1)
