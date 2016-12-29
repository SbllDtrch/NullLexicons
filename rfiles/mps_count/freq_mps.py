import argparse
import nltk

vowels = {}
for v in "IE{VQU@i#$u312456789cq0~iI1!eE2@aAoO3#4$6^uU7&5+8*9(0)<>[]{}":
    vowels[v] = 0

parser = argparse.ArgumentParser()
parser.add_argument('--f', metavar='--f', type=str, nargs='?',
                    help='input file', default="")
parser.add_argument('--c', metavar='--c', type=str, nargs='?',
                    help='contrast file', default="")
parser.add_argument('--lang', metavar='--l', type=str, nargs='?',
                    help='language', default="")
parser.add_argument('--p', metavar='--p', type=float, nargs='?',
                    help='language', default="5.0")
parser.add_argument('--pos', metavar='--pos', type=float, nargs='?',
                    help='language', default="-1.0")




args = parser.parse_args()
real = [i.strip().split(",") for i in open(args.f, "r").readlines() if i.strip().split(",")[0] == "real"]
sim = [i.strip().split(",") for i in open(args.f, "r").readlines()[1:] if i.strip().split(",")[0] != "real"]

nb = int(len(real)*args.p/100)
real_s = sorted(real, key = lambda x: args.pos*int(x[2]))[:nb]
f_contrasts = [i.strip() for i in open(args.c, "r").readlines()]

contr= nltk.defaultdict(list)
for c in f_contrasts:
    a = c.split(",")
    contr[a[0]] = [a[1], a[2], a[3]]


diff = 0
n = 0
set_contr = []
for s in real_s:
    a = s[1].split("_")
    set_contr += [s[1]] 
    if a[0] in contr.keys() and a[1] in contr.keys():
        diff += len(list(set(contr[a[0]]) - set(contr[a[1]])))
        n += 1

print args.lang, "real", 1.0*diff/n

diff= nltk.defaultdict(int)
n= nltk.defaultdict(int)
for j in range(0,29):
    sim_j = [i for i in sim if int(i[0]) == j]
    sim_s = sorted(sim_j, key = lambda x: args.pos*int(x[2]))[:nb]
    for i in sim_s:
        a = i[1].split("_")
        if a[0] in contr.keys() and a[1] in contr.keys():
            diff[i[0]] += len(list(set(contr[a[0]]) - set(contr[a[1]])))
            n[i[0]] += 1
        else: print a

for i in diff.keys():
    print args.lang, i, 1.0*diff[i]/n[i], n[i]


            
f = [i.strip().split(",") for i in open(args.f, "r").readlines()[1:]]
mps_contr = nltk.defaultdict(lambda: nltk.defaultdict(int))

for i in f:
    a = i[1].split("_")
    if a[0] in contr.keys() and a[1] in contr.keys():
        diff = len(list(set(contr[a[0]]) - set(contr[a[1]])))
        mps_contr[i[0]][diff] += float(i[2])


#for i in mps_contr.keys():
#    for j in mps_contr[i].keys():
#        print args.lang, i, j, mps_contr[i][j]




mps_vowels = nltk.defaultdict(int)
mps_consonants = nltk.defaultdict(int)
for i in f:
    a = i[1].split("_")
    if a[0] in vowels.keys() and a[1] in vowels.keys():
        mps_vowels[i[0]] += float(i[2])
    elif a[0] not in vowels.keys() and a[1] not in vowels.keys():
        mps_consonants[i[0]] += float(i[2])


#for i in mps_vowels.keys():
#    print args.lang, i, "vowel", mps_vowels[i]

#for i in mps_consonants.keys():
#    print args.lang, i, "consonant", mps_consonants[i]



