import os
from PIL import Image
import numpy as np

path ='SYS'
#直接用rgba的bmp就行，能识别
for file in os.listdir(path): 
    if file.endswith('elg'):continue
    os.system(f'python png2elg.py SYS/{file} SYS/{file.split(".")[0]}.elg')
            