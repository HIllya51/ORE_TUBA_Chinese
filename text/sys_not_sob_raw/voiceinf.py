# coding=utf8
 

import tqdm ,os,shutil
for current in ['CHS','CHT']: 
    with open('voiceinf.dat','rb') as ff:
        bs=ff.read()
    names=[
        '渡来 明日香','渡来 明日香','渡來 明日香',
        '山科 京','山科 京','山科 京',
        '羽田 小鳩','羽田 小鸠','羽田 小鳩',
        '玉泉 日和子','玉泉 日和子','玉泉 日和子',
        '望月 紀奈子','望月 纪奈子','望月 紀奈子',
        '日野 英里子','日野 英里子','日野 英裡子',
        '米田 優','米田 优','米田 優',
        '鳳 鳴','凤 鸣','鳳 鳴',
        '香田 亜衣','香田 亚衣','香田 亞衣',
        '春日 春恵','春日 春恵','春日 春恵',
        '高内 昌子','高内 昌子','高內 昌子',
        '羽田 鷹志','羽田 鹰志','羽田 鷹志',
        '千歳 鷲介','千岁 鹫介','千歲 鷲介',
        '成田 隼人','成田 隼人','成田 隼人',
        'アリス','爱丽丝','愛麗絲',
        '伊丹 伽楼羅','伊丹 伽楼罗','伊丹 伽樓羅',
        '少年ヨージ','少年鹰志','少年鷹志',
        '青年ヨージ','青年鹰志','青年鷹志',
        'DJコンドル','DJ秃鹫','DJ禿鷲',
        '森里 和馬','森里 和马','森裡 和馬',
        '針生 蔵人','针生 藏人','針生 藏人',
        '鳳 翔','凤 翔','鳳 翔',
        '軽部 狩男','轻部 狩男','輕部 狩男',
        'マルチネス','马汀内斯','馬汀內斯',
        'プラチナ','普纳奇纳','普納奇納',
        'メンマ','面魔','面魔',
        'チケドン','奇肯多','奇肯多',
        'ＬＲ２００１','ＬＲ２００１','ＬＲ２００１',
        'バニィＤ','巴尼Ｄ','巴尼Ｄ',
        '咲夜','咲夜','咲夜',
        '狂夜','狂夜','狂夜',
        '満夜','满夜','滿夜',
        'その他・女','其他·女','其他·女',
        'その他・男','其他·男','其他·男',
    ]
    
    bs=bytearray(bs)
    for i in range(len(names)):
        if i%3!=0:
            continue
        #print(names[i])
        sjis=names[i].encode('shiftjis')
        if current=='CHS':
            gbk=names[i+1].encode('gbk')
        else:
            gbk=names[i+2].encode('gbk')
        idx=bs.index(sjis)
        
        if len(sjis)>len(gbk):
            print(len(sjis)-len(gbk))
            
            gbk+=('　'.encode('gbk')) *((len(sjis)-len(gbk))//2)
        for j in range(len(sjis)):
            bs[idx+j]=0
        for j in range(len(gbk)):
            bs[idx+j]=gbk[j]
    with open('../sys_not_sob_solved/voiceinf.dat','wb') as ff:
        ff.write(bs)


    with open('ll_app.inf','rb') as ff:
        bs=ff.read()
    if current=='CHS':
        names= '我们没有翅膀'.encode('gbk')
    else:
        names= '我們沒有翅膀'.encode('gbk')
    solved=b''
    solved+=b'\x01\x00\x00' 
    import ctypes
    solved+=bytes(ctypes.c_char(len(names)+1))
    print(solved)
    solved+=names
    print(solved)
    solved+=b'\x00'

    solved+=bytes(ctypes.c_char(len(names)+1))
    solved+=names
    solved+=b'\x00'

    solved+=bs[0x27:]

    with open('../sys_not_sob_solved/ll_app.inf','wb') as ff:
        ff.write(solved)
    for f in tqdm.tqdm(os.listdir('..\\sys_not_sob_solved')):
        shutil.copy(f'..\\sys_not_sob_solved\\{f}',f'C:\\dataH\\俺たちに翼はない\\SCRIPT_{current}')

 