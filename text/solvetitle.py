import os,shutil

mps=[
    'ゲームを終了します','结束游戏','結束游戲',
    'ゲームシステムの設定を行います','打开游戏系统设置','打開遊戲系統設置',
    '音楽鑑賞画面へ移行します','打开音乐鉴赏界面','打開音樂鑑賞界面',
    'ギャラリー画面へ移行します','Gallery界面','Gallery界面',
    'セーブした場面から再開します','从保存的场景重新开始','從保存的場景重新開始',
    '最初から始めます','从最初开始','從最初開始',
    #func_lib
    'クイックセーブを実行しました','快速存档完毕','快速存檔完畢'  ,
    r'''ゲームを終了します。
よろしいですか？''',
    '确定要结束游戏吗？','確定要結束遊戲嗎？',
    r'''タイトルに戻ります。
よろしいですか？''',
    '确定要返回到标题界面吗？','確定要返回到標題界面嗎？',
    r'''前の選択肢に戻ります。
よろしいですか？''',
    '确定要返回到前一个选择支吗？','確定要返回到前一個選擇支嗎？'
]

for current in ['CHS','CHT']:
    shutil.copy('SCRIPT.LPK/func_lib.sob',current+ '\\SCRIPT.LPK_pack\\func_lib.sob')
    for f in ['title.sob','title2.sob','func_lib.sob']:

        try:
            with open(current+ '\\SCRIPT.LPK_pack\\'+f,'rb') as ff:
                bs=ff.read()
        except:
            continue
        
        bs=bytearray(bs)
        for i in range(len(mps)):
            if i%3!=0:continue
            sjis=mps[i].encode('cp932')
            gbk=mps[i+(1 if(current=='CHS') else 2)].encode('gbk')
            
            idx=bs.find(b'\x00'+sjis+b'\x00')
            if idx<0:continue
            idx+=1
            for j in range(len(sjis)):
                bs[idx+j]=0
            for j in range(len(gbk)):
                bs[idx+j]=gbk[j]

        with open(r'C:\dataH\俺たちに翼はない\SCRIPT_'+current+'\\'+f,'wb') as ff:
            ff.write(bs)