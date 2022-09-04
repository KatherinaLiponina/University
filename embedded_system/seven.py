import numpy as np
import cv2

def draw_rectangles(frame, amount=1, active=1):
    if amount > 4:
        amount = 3
    width, height, channels = frame.shape
    rect_size = 100
    color = (255, 0, 0)
    thickness = 2
    
    width_gap = (width - amount * rect_size) / (amount + 1)
    height_gap = (height - amount * rect_size) / (amount + 1)
    for i in range(amount):
        start_point = (int(height_gap * (i + 1) + rect_size * i), int(width_gap * (i + 1) + rect_size * i))
        end_point = (int(height_gap * (i + 1) + rect_size * (i + 1)), int(width_gap * (i + 1) + rect_size * (i + 1)))
        rect = cv2.rectangle(frame, start_point, end_point, color, thickness)
    
    rectangle_point = [(int(height_gap * (active + 1) + rect_size * active), int(width_gap * (active + 1) + rect_size * active)), (int(height_gap * (active + 1) + rect_size * (active + 1)), int(width_gap * (active + 1) + rect_size * (active + 1)))]
    return rectangle_point



def process(frame, active):
    width, height, channels = frame.shape
    rect_size = 100
    h_sensivity = 15
    s_h = 255
    v_h = 255
    s_l = 50
    v_l = 50
    color = (255, 255, 255)
    thickness = 2
    rectangle_point = draw_rectangles(frame, 3, active)
    rect = cv2.rectangle(frame, rectangle_point[0], rectangle_point[1], (0, 255, 0), 3)
    
    color_amount = 8
    colors = [20, 30, 55, 90, 120, 150, 180, 0]

    hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask_frame = hsv_frame[rectangle_point[0][1]:rectangle_point[1][1] + 1, rectangle_point[0][0]:rectangle_point[1][0] + 1]


    color_upper = []
    color_lower = []
    mask_color = []
    color_rate = []
    for i in range(color_amount):
        color_upper.append(np.array([colors[i] + h_sensivity, s_h, v_h]))
        color_lower.append(np.array([colors[i] - h_sensivity, s_l, v_l]))
        mask_color.append(cv2.inRange(mask_frame, color_lower[i], color_upper[i]))
        color_rate.append(np.count_nonzero(mask_color[i])/(rect_size*rect_size))

    org = (height - 250, 100)
    font = cv2.FONT_HERSHEY_SIMPLEX
    fontScale = 0.7
	
    maximum = -1
    for i in range(color_amount):
        if color_rate[i] > maximum:
            maximum = color_rate[i]
            max_color = i

    if maximum > 0.9:
        if max_color == 0:
            founded_color = ' orange '
            printed_color = (0, 165, 255)
        elif max_color == 1:
            founded_color = ' yellow '
            printed_color = (0, 255, 255)
        elif max_color == 2:
            founded_color = ' green '
            printed_color = (0, 255, 0)
        elif max_color == 3:
            founded_color = ' light-blue '
            printed_color = (255, 166, 128)
        elif max_color == 4:
            founded_color = ' blue '
            printed_color = (255, 0, 0)
        elif max_color == 5:
            founded_color = ' purple '
            printed_color = (255, 0, 128)
        elif max_color == 6 or max_color == 7:
            founded_color = ' red '
            printed_color = (0, 0, 255)
        else:
            founded_color = ' nothing detected '
            printed_color = color
        text = cv2.putText(rect, founded_color, org, font, fontScale, printed_color, thickness, cv2.LINE_AA)
    else:
        text = cv2.putText(rect, ' nothing detected ', org, font, fontScale, color, thickness, cv2.LINE_AA)

    av_hue = np.average(mask_frame[:,:,0])
    av_sat = np.average(mask_frame[:,:,1])
    av_val = np.average(mask_frame[:,:,2])
    average = [int(av_hue),int(av_sat),int(av_val)]
    
    text = cv2.putText(rect, str(average) + " " + str(maximum), (height - 275, 50), font, fontScale, color, thickness, cv2.LINE_AA)
    frame = text

    return frame



print('Press 4 to Quit the Application\n')

#Open Default Camera
cap = cv2.VideoCapture(0)

active = 1
while(cap.isOpened()):
    #Take each Frame
    ret, frame = cap.read()
    
    #Flip Video vertically (180 Degrees)
    frame = cv2.flip(frame, 180)
    
    invert = process(frame, active)


    # Show video
    # cv2.imshow('Cam', frame)
    cv2.imshow('Inverted', invert)

    # Exit if "4" is pressed
    k = cv2.waitKey(1) & 0xFF
    if k == 52 : #ord 4
        #Quit
        print ('Good Bye!')
        break
    elif k == 51:
        active = 2
    elif k == 50:
        active = 1
    elif k == 49:
        active = 0


#Release the Cap and Video   
cap.release()
cv2.destroyAllWindows()
