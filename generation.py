from evaluation import *
import random, sys, re
import nltk

vowels = {}
for v in "IE{VQU@i#$u312456789cq0~iI1!eE2@aAoO3#4$6^uU7&5+8*9(0)<>[]{}":
    vowels[v] = 0

def get_cv(word, cv = 0):
    x = ""
    for letter in word:
        if letter in vowels: 
            x += "V"
        else: x += "C"
    if cv == 1: return x
    return len(re.sub("-","",word))
    
    
def generate_correct_number(corpus, homo, lm):
    """Generate number of words to match length, handle homophones being generated"""
    lengths = nltk.defaultdict(lambda: nltk.defaultdict(int))
    lengths_needed = nltk.defaultdict(int)

    for item in corpus:
        lengths[get_cv(item)][len(item.split("-"))] += 1
        lengths_needed[get_cv(item)] += 1
    newwords = []
    newwords2 = []
    exist = nltk.defaultdict(int)
    while True:
        words = lm.generate()
        for w in words:
            if lengths_needed[get_cv(w)] > 0:
                if homo == 1 or (w not in newwords and re.sub("-","",w) not in newwords2):
                   lengths_needed[get_cv(w)] += -1
                   newwords += [w]
                   newwords2 += [re.sub("-", "", w)]
                   if w in corpus:
                        exist[len(w)] +=1
            elif sum([lengths_needed[j] for j in lengths_needed.keys()]) == 0: 
                print "nb of real words", sum(exist.values())
                return newwords



def write_lex_file(o, corpus, cv, iter, lm, homo):
    outfile  = open(o, "w")
    for c in corpus:
        outfile.write("-1," + re.sub("-","",c) + "\n")
    for i in range(iter):
        print "generating null lex", str(i)
        gen_lex= generate_correct_number(corpus, homo, lm)
        for w in gen_lex:
            outfile.write(str(i) + "," +  re.sub("-","",w) + "\n")
    outfile.close()
    return o


