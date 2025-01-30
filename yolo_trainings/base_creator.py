import glob, os
import cv2
import random

object_names = ["pawn", "cylinder", "cube"]

path = "C:\\worktable_copy\\robo-arm\\diploma\\new\\data\\prep_images"
path_to_obj = path + "\\objects"
path_to_back = path + "\\backgrounds"

path_to_base = "C:\\worktable_copy\\robo-arm\\diploma\\new\\data\\base\\"
path_to_img = path_to_base + "train\\images\\"

val_path_to_base = path_to_base + "val\\"
val_path_to_img = val_path_to_base + "images\\"

pattern = '*.jpg'
glob_path = os.path.join(path_to_obj, pattern)
list_files = glob.glob(glob_path)

class object:

    def __init__(self, obj_number):
        self.number = obj_number
    
    def line(self):
        self.w = w / new_img.shape[1]
        self.h = h / new_img.shape[0]

        self.y_c = random.randint(int(h / 2) + 1, new_img.shape[0] - int(h / 2) - 1) #  координаты центра в пикселях
        self.x_c = random.randint(int( w / 2) + 1, new_img.shape[1] - int(w / 2) - 1)

        y_c_n = self.y_c / new_img.shape[0]
        x_c_n = self.x_c / new_img.shape[1]

        line1 = str(x_c_n) + ' ' + str(y_c_n) + ' ' + str(self.w) + ' ' + str(self.h)
        line1 = str(int(self.number)) + ' ' + line1
        return line1

    def imwrite_object_to_back(self):

        new_up = self.y_c - int(h / 2)
        new_down = self.y_c + int(h / 2)
        new_left = self.x_c - int(w / 2)
        new_right = self.x_c + int(w / 2)
        
        if img.shape[1] > new_right-new_left:
            new_right += 1
        if img.shape[0] > new_down-new_up:
            new_down += 1

        new_img[new_up : new_down, new_left : new_right] = img


if list_files:
    file_number = 0

    for file_name in list_files:
        filename = os.path.basename(file_name)
        
        filename_txt = path_to_base + "train\\labels\\" + filename[:-5] + ".txt"
        val_filename_txt = val_path_to_base + "labels\\" + filename[:-5] + ".txt"
        
        img=cv2.imread(file_name)
        
        w = img.shape[1]
        h = img.shape[0]

        file_number += 1
        i = 0
    
        new_img = cv2.imread(os.path.join(path_to_back, random.choice(os.listdir(path_to_back))))

        for line in object_names:   
            if filename[:len(object_names[i])] == object_names[i]:
                can = object(i)

                if file_number% 6 == 0 :
                    with open(val_filename_txt, 'w') as fw:
                        fw.write(f'{can.line()}\n')
                        fw.close()
                        can.imwrite_object_to_back()
                        cv2.imwrite(val_path_to_img + filename, new_img)
                        print(filename, "validation") 

                else:
                    with open(filename_txt, 'w') as fw:
                        fw.write(f'{can.line()}\n')
                        fw.close()
                        can.imwrite_object_to_back()
                        cv2.imwrite(path_to_img + filename, new_img)
                        print(filename) 
            i += 1
        
