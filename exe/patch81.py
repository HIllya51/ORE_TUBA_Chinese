with open('asm.txt','r',encoding='utf8') as ff:
    asm=ff.read()
asmx={}
for line in asm.split('\n'):
    idx=6
    if line[idx]==':':
        idx+=1
    
    addr=int(line[idx:idx+8],16)
    asmx[addr]=line[idx+8:]


for f in ['我们没有翅膀','我們沒有翅膀']:
    #使用CFF explorer添加dll
    with open(f+'+dll.exe','rb') as ff:
        bs=bytearray(ff.read())

    alignoff=0x401036-bs.index(b'\x8A\x86\x2D\x0E\x00\x00')

    i=0
    cnt=0
    while True:
        i+=1
        if i>len(bs)-1:break
        if bs[i] not  in [0x3c,0x80]:continue
        
        addr=i+alignoff

        if addr not in asmx:continue
        
        if 'cmp' not in asmx[addr] :continue
        if 'l, ' not in asmx[addr]:continue
        skip=1 + (bs[i]==0x80)
        mp={
            0x9f:0xfe,
            0xfc:0xfe
        }
        if bs[i+skip] in mp:
            bs[i+skip]=mp[bs[i+skip]] 
            cnt+=1
            print(hex(addr),asmx[addr])
    print(cnt)
    with open(f+'.exe','wb')as ff:
        ff.write(bs)
    