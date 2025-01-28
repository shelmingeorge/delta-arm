import cv2
import time
j = 0
i = 0;
video_capture = cv2.VideoCapture(0)

path = "your_path to folder"

if (video_capture.isOpened() == False):
  print("Error opening the video file")
else:
  fps = video_capture.get(20)

  while(video_capture.isOpened()):
    ret, frame = video_capture.read()
    #frame = cv2.resize(frame,(1080,720))
    cv2.imshow('frame', frame)
    cv2.waitKey(1)
    if j % 2 == 0 :
      cv2.imwrite(path + "nothing" + str(0 + i) + ".jpg", frame)
      i = i + 1
    j = j + 1
    print(j,i)
