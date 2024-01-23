with open('config.dat','rb') as ff:
    bs=ff.read()
import ctypes,json
save=[]

for i in range(len(bs)-4):
    strlen=(ctypes.c_uint.from_buffer_copy(bs[i:]).value)
    if(strlen>1000):continue
    s=bs[i+4:i+4+strlen-1]
    
    if b'\x00' in s:
        continue
    
    try:
        sjis=s.decode('shiftjis')
        
    except:
        continue
    try:
        sjis.encode('ASCII')
        continue
    except:
        pass

    print(strlen,sjis)
    save.append([i,sjis])

with open('transconfig.json','w',encoding='utf8') as ff:
    ff.write(json.dumps(save,ensure_ascii=False,indent=4))