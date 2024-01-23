import subprocess
import os, shutil,tqdm


# for f in tqdm.tqdm(os.listdir('SCRIPT.LPK')):
#     # if(os.path.exists('SCRIPT.LPK_unpack\\'+f+'.txt') ):
#     #     continue

    
#     xx=subprocess.run('lucifen.exe SCRIPT.LPK\\'+f, stdout=subprocess.PIPE )
#     #print(xx.stdout)
#     if xx.stdout==b'':
#         continue
#     with open('SCRIPT.LPK_unpack\\'+f+'.txt','wb'  ) as ff:
#         ff.write(xx.stdout )




for f in tqdm.tqdm(os.listdir('SCRIPT.LPK')):
    # if(os.path.exists('SCRIPT.LPK_unpack\\'+f+'.txt') ):
    #     continue

    xx=subprocess.run('lucifen\\build\\Release\\unpack.exe SCRIPT.LPK\\'+f, stdout=subprocess.PIPE )
    #print(xx.stdout)
    if xx.stdout==b'':
        continue
    with open('SCRIPT.LPK_unpack\\'+f+'.txt','wb'  ) as ff:
        ff.write(xx.stdout )



 