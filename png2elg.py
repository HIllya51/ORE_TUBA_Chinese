import os
from PIL import Image
 
path ='SYS'
#直接用rgba的bmp就行，能识别
for file in os.listdir(path): 
        with Image.open(path+'/'+file) as img: 
            new_name =path+'/'+ os.path.splitext(file)[0] + '.elg' 
            img.save(new_name, 'bmp')
            