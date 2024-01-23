import os
import json
xxx={}
for f in os.listdir('备注汇总'):
    with open('备注汇总/'+f,'r',encoding='utf8') as ff: 
        file=ff.read()
    lines=file.split('\n')
    lines.append('○')
    collect=[]
    key=''
    for i in range(len(lines)):
        line=lines[i]
        if len(line) and line[0]=='○':
            
            filter(lambda x:len(x)==0,collect)
            if len(collect) and len(key):
                value='\n'.join(collect)
                 
                xxx[key]=value
            collect=[]
                
            key=line
            
        else:
            collect.append(line)

##check key can matched

collect={}
for f in os.listdir('./CHS/merge'):
    with open('./CHS/merge/'+f,'r',encoding='utf8') as ff: 
        file=ff.read()
    collect.update(json.loads(file))
newxxx={}
for key in xxx:
     
    if key not in collect:
        if (key+'$') in collect:
            key1=key+'$'
            newxxx[collect[key1]]=xxx[key]
        else:
            print(key)
    else: 
        newxxx[collect[key]]=xxx[key]
with open('../patch/tips.json','w',encoding='utf8') as ff:
    ff.write(json.dumps(newxxx,ensure_ascii=False,indent=4))