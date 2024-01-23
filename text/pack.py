import subprocess
import os, shutil
from tqdm import tqdm
from collections import OrderedDict


half='1234567890!@#$%^&*()_+-=,./?;:\'"`~qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM'
QUAN='１２３４５６７８９０！＠＃＄％＾＆＊（）＿＋－＝，．／？；：＇＂｀～ｑｗｅｒｔｙｕｉｏｐａｓｄｆｇｈｊｋｌｚｘｃｖｂｎｍＱＷＥＲＴＹＵＩＯＰＡＳＤＦＧＨＪＫＬＺＸＣＶＢＮＭ'

halfsolve=str.maketrans(half,QUAN)
# for f in os.listdir('SCRIPT_pre_unpack'):
#     fr=f[:-4]
#     # if os.path.exists(f'.\\SCRIPT.LPK_pack\\{fr}'):
#     #     continue 
#     xx=subprocess.run(f'.\\lucifen_pack.exe  .\\SCRIPT_pre\\{fr} .\\SCRIPT_pre_unpack\\{f} .\\SCRIPT_pre_pack\\{fr}' , stdout=subprocess.PIPE )
#     #print(xx.stdout)
def filtern(st):
    x=st.split('\n')
    x=[s for s in x if s]
    return '\n'.join(x)
def findx(s,string,start=0):
    for i in range(start,len(string)):
        if(string[i]==s and string[i+9]==s):return i
    return -1
def collectmap(file:str):
    import re   
    pair=OrderedDict()
    while True:
        _1=findx('○',file)
        if _1==-1:break
        
        _2=findx('●',file) 
        origin=file[_1:_2]
        _3=findx('○',file,_2)
        if _3==-1:_3=len(file)
        trans=file[_2:_3]
        
        addr1=origin[1:9]
        key=origin[10:]
        addr2=trans[1:9]
        v=trans[10:]
 
        kk='○'+addr1+'○'+key
        vv='●'+addr2+'●'+v
        pair[filtern(kk)]=filtern(vv)
        file=file[len(kk)+len(vv):]
        #print(kk,vv)
    return pair
offset=len('●00010C29●')
def tranparse(origin,dd):
    
    
    for k in dd:
        # if '\n' in k:
        #     print(k)
        if k not in origin:
            #print(k)
            addrts=k[:offset]
            usekey=None
            for kk in origin:
                addr=kk[:offset]
                if addr==addrts:
                    usekey=kk 
            
            if usekey is None:
                try:
                    k[offset:].replace('・','').encode('latin-1')
                    ok=True
                except:
                    ok=False
                if ok==False:
                    print(k[offset:].replace('・','').encode('utf8'))
                    print(1,k[offset:],2)
                    raise Exception()
                else:
                    continue
        else:
            usekey=k
        res=dd[k].strip()
        res=res[offset:]
        
        pre='' 
        
        while True:
            if res[-1] in '$ ':
                res=res[:-1]
            else:
                break
        if res[0]=='#': 
            _res=res.find('&')+1
            pre=res[:_res]
            res=res[_res:]
            
        if res[0]=='<':
            idx=res.index('>')+1
            pre+=res[:idx]
            res=res[idx:]

        _res=res.replace('%s','').replace('%d','')
        res1=str.translate(_res,halfsolve)
        if _res!=res1: 
            if set(_res)-set(half)==set():
                #raise Exception(_res)
                print(_res,res1)
            #需要处理半角
            res=res.translate(halfsolve)
        res=pre+res
        if k[-1]=='$' :
            res+='$'  
        res=dd[k][:offset]+res
        
        #dd[k]=res
        origin[usekey]=res

 
for current in ['CHS','CHT']:
    for f in os.listdir(f'./{current}/translatedok'):
        if f[:7]=='gallery':continue
        #if f!='s01_02e.sob.txt':continue
        #if f!='s02_01a.sob.txt':continue
        fr=f.split('.')[0]+'.sob.txt'
        try:
            with open('./SCRIPT.LPK_unpack/'+fr,'r',encoding='utf8') as _:
                origin=_.read()
            with open(f'./{current}/translatedok/'+f,'rb') as _:
                transed=_.read()
                if (transed[:3])==b'\xEF\xBB\xBF':
                    transed=transed[3:]
                    #print(f)
                transed=transed.decode('utf8').replace('\r\n','\n')
        except:
            #print(f,'!!!!!!!!!!!!!!!!!!!!!!')
            continue
        
        
        #print(f)
        originlines=origin.split('\n')
        translines=transed.split('\n')
        for line in originlines:
            if len(line) and line[0]=='○':
                if line not in translines:
                    print(f,line)
        origin=collectmap(origin)
        translated=collectmap(transed) 
        
        tranparse(origin,translated)
        
        
        with open(f'./{current}/merge/'+f.replace(' ',''),'w',encoding='utf8')as _:
            # for k in origin:
            #     _.write(k)
            #     _.write('\n')
            #     _.write(origin[k])
            #     _.write('\n\n')
            import json
            origin2={}
            

            for k in origin:
                # if k[offset:] in origin2 and origin2[k[offset:]]!=origin[k][offset:]:
                #     if k[offset:] not in ['　ぽっぽー、ぽっぽー、ぽっぽー……。$']:
                #         if Levenshtein.distance(origin2[k[offset:]],origin[k][offset:])>1:
                #             print(k)
                #             print(origin2[k[offset:]])
                #             print(origin[k][offset:])
                #             raise Exception()
                # origin2[k[offset:]]=origin[k][offset:]
                if len(origin[k][offset:])<3 and   f[:6]=='title2':
                    continue
                 
                origin2[k]=origin[k][offset:]
                origin2[k+'\n']=origin[k][offset:]+'\n'
            _.write(json.dumps(origin2,ensure_ascii=False))

    for f in tqdm(os.listdir(current+ '/merge')):
        if f[:7]=='gallery':continue
        #if f!='s02_01a.sob.txt':continue
        fr=f.split('.')[0]+'.sob'
        #print(f'.\\Project1.exe  .\\SCRIPT.LPK\\{fr} .\\{current}\\merge\\{f} .\\{current}\\SCRIPT.LPK_pack\\{fr}')
        #xx=subprocess.run(f'.\\lucifen_pack.exe  .\\SCRIPT.LPK\\{fr} .\\merge\\{f} .\\SCRIPT.LPK_pack\\{fr}' , stdout=subprocess.PIPE )
        xx=subprocess.run(f'.\\lucifen\\build\\Release\\pack.exe  .\\SCRIPT.LPK\\{fr} .\\{current}\\merge\\{f} .\\{current}\\SCRIPT.LPK_pack\\{fr}'   )
        shutil.copy(f'.\\{current}\\SCRIPT.LPK_pack\\{fr}',f'C:\\dataH\\俺たちに翼はない\\SCRIPT_{current}')
