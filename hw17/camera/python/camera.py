import pgzrun # pip install pgzero

import serial
ser = serial.Serial('/dev/tty.usbmodem1101') # the name of your port here

import numpy as np

# Set the window size
WIDTH = 400
HEIGHT = 400
def update():
    selection_endline = 'c'+'\n'
     
    # send the command 
    ser.write(selection_endline.encode())
def draw():
    reds = np.zeros((60, 80), dtype=np.uint8)
    greens = np.zeros((60, 80), dtype=np.uint8)
    blues = np.zeros((60, 80), dtype=np.uint8)
    
    
    for t in range(4800):
            dat_str = ser.read_until(b'\n'); 
            dat_str = dat_str.decode('utf-8').strip()
            print(dat_str)
            i,r,g,b = list(map(int,dat_str.split())) 
            row = i//80
            col = i%80
            reds[row][col] = min(255, int(r * 255 / 31))
            greens[row][col] = min(255, int(g * 255 / 31))
            blues[row][col] = min(255, int(b * 255 / 31))

    screen.fill((0, 0, 0))  # Fill the background with black
    for x in range(60):
         for y in range(80):
              screen.draw.filled_rect(Rect((x * 3, (60-y) * 3), (3, 3)), (reds[x][y], greens[x][y], blues[x][y]))

pgzrun.go()