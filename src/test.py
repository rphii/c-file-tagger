import os

files = {}
tags = {}
n = 0

for root, dirs, osfiles in os.walk('/home/rphii/dev/c'):
    #print(f'Current Path: {root}')
    #print(f'Subdirectories: {dirs}')
    #print(f'Files: {osfiles}')
    #print('--------------')
    for file in osfiles:
        use = False
        use |= file.endswith('.csv')
        use |= file.endswith('.txt')
        use |= file.endswith('.cft')
        use |= file.endswith('.md')
        use |= file.endswith('.pdf')
        if not use: continue
        #print(n,root+'/'+file)#,use)
        n += 1
        with open(root+'/'+file, mode='rb') as f:
            #content = f.read().decode('utf-8')
            content = f.readlines() #f.read().decode('utf-8')
            for line in content:
                #print(content)
                #print(line)
                #chunks = line.split(b',')
                chunks = [x.strip() for x in line.split(b',')]
                #print(chunks)
                filen = None
                for chunk in chunks:
                    #print(f'CHUNK:[{chunk}]')
                    if filen is None:
                        filen = chunk
                        #print(filen)
                        if filen not in files:
                            files[filen] = set()
                    else:
                        if chunk not in tags:
                            tags[chunk] = set()
                        tags[chunk].add(filen)
                        files[filen].add(chunk)

#for i,file in enumerate(sorted(files)):
#    print(f'[{i}] {file}')

#s = ''
for i,tag in enumerate(sorted(tags)):
    #s += f'[{i}] {tag}\n'
    print(f'[{i}] {tag}')
    #print(f'{tag}')
    #pass
#print(s)

