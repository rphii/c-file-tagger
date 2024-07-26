import csv

files = {}
tags = {}

infile = '/home/hp/.config/cft/tags.cft'
#infile = '/home/hp/Downloads/Firefox/customers-2000000.csv'
#infile = '/home/hp/Downloads/Firefox/customers-100000.csv'
with open(infile, newline='') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=',')
    for row in spamreader:
        filename = None
        for x in row:
            if filename is None:
                filename = x
            else:
                if filename not in files:
                    files[filename] = set()
                files[filename].add(x)
                if x not in tags:
                    tags[x] = set()
                    tags[x].add(filename)


print(f'Files: {len(files)}')
for i, file in enumerate(sorted(files)):
    print(f'  [{i}] {file} ({len(files[file])})', end='')
    for j, tag in enumerate(files[file]):
        print(f' {tag}', end='')
    print("")

#print(f'Tags: {len(tags)}')
#for i, tag in enumerate(sorted(tags)):
#    print(f'  [{i}] {tag} ({len(tags[tag])})', end='')
#    for j, file in enumerate(tags[tag]):
#        print(f' {file}', end='')
#    print("")

