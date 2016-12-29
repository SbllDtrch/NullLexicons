from nltk import *
import random, sys, re, os
import nltk
import argparse
import itertools
import Levenshtein
import time, datetime
import pickle
t = time.time()
import collections
from calc_stats import *
import copy
from lm import *
from nphone import NgramModel
from nsyll import NsyllModel
from pcfg import PCFG
from evaluation import *
from generation import *
#from perplexity import *
from subprocess import call



try:
    os.mkdir('Lexicons')
except OSError:
    pass

try:
    os.mkdir('Graph')
except OSError:
    pass

try:
    os.mkdir('evaluation')
except OSError:
    pass



""" --------------main------------"""

parser = argparse.ArgumentParser()
parser.add_argument('--n', metavar='--n', type=int, nargs='?',
                    help='n for ngram', default=3)
parser.add_argument('--homo', metavar='h', type=int, nargs='?',
                    help='0 to exclude homophones being generated, 1 for homophones allowed', default=0)
parser.add_argument('--model', metavar='--m', type=str, nargs='?',
                    help='should be nphone for ngram model', default="nphone")
parser.add_argument('--iter', metavar='--i', type=int, nargs='?',
                    help='number of lexicons to generate', default=2)
parser.add_argument('--lex', metavar='--l', type=str, nargs='?', 
                    help='lexicon file - list of words, one on each line')
parser.add_argument('--cv', metavar='--cv', type=int, nargs='?',
                    help='put 1 here to match for CV pattern', default=0)
parser.add_argument('--grammar', metavar='--g', type=str, nargs='?',
                        help='grammar file for pcfg', default="grammars/grammar_german.wlt")
parser.add_argument('--fnc', metavar='--f', type=str, nargs='?',
                     help='evaluate/generate corpus', default="generate")
parser.add_argument('--train', metavar='--t', type=float, nargs='?',
                      help='fraction of corpus use for training', default=0.75)
parser.add_argument('--freq', metavar='--freq', type=int, nargs='?',
                       help='use frequency based lm', default=0)
parser.add_argument('--graph', metavar='--g', type=int, nargs='?',
                        help='output graph', default=0)
parser.add_argument('--smoothing', metavar='--s', type=float, nargs='?',
                        help='discount for add-smoothing', default=0.01)
parser.add_argument('--lang', metavar='--lang', type=str, nargs='?', help='language english/dutch/german/french', default="english")


args = parser.parse_args()
corpus = [i.strip() for i in open(args.lex, "r").readlines()]

if args.fnc == "generate":
    print "### GENERATING WORDS ###"
    print "model:", args.model, args.n 
    print "language:", args.lang
    if args.model == "nphone":
        lm = NgramModel(args.n, corpus, 1)
    elif args.model == "nsyll":
        if args.lex.startswith("celexes/syll"):
            lm = NsyllModel(args.n, corpus, 1)
        else:
            print "Use syll__ file for this model"
            sys.exit()
    elif args.model == "pcfg":
        if args.lex.startswith("celexes/pcfg"):
            print call(["./pcfg/io","-d","1","-g", args.grammar, args.lex],stdout = open('grammars/gram_pcfg.wlt', 'w'))
            lm = PCFG('grammars/gram_pcfg.wlt')
            corpus = [re.sub(" ","",x) for x in corpus]
        else:
            print "Use pcfg__ file for this model"
            sys.exit()
    lm.create_model(corpus, args.smoothing) 
    o = "Lexicons/lex_" + args.lex.split("/")[-1][:-4] + "_cv" +  str(args.cv) + "_iter" + str(args.iter) + "_m" + args.model + "_n" + str(args.n) + "_smoothing" + str(args.smoothing) + ".txt"
    lexfile = write_lex_file(o, corpus, args.cv, args.iter, lm, args.homo)
    print "null lexicons wrote on", lexfile
    print "### WRITING RESULTS ###"
    write_all(lexfile, args.graph, args.lang)


else: 
    o = "evaluation/eval_" + args.lex.split("/")[-1][:-4] + "_cv" +  str(args.cv) + "_iter" + str(args.iter) + "_m" + args.model  + "_n" + str(args.n) + "_smoothing" +  str(args.smoothing)+ ".txt"
    out = open(o, 'w')
    out.write("i,smoothing,model,ppl,logprob\n")
    lengths = nltk.defaultdict(int)
    for c in corpus:
        lengths[len(c)] += 1
    for it in range(args.iter):
        train = random.sample(corpus, int(args.train*len(corpus)))
        test_temp = random.sample(corpus, int((1-args.train)*len(corpus)))
        test = []
        for i in test_temp:
            i = i.split("\t")
            test += [i[0]]#*int(i[2]) 
        if args.model == "nphone":
            lm= NgramModel(args.n, train)
        elif args.model == "nsyll":
            lm = NsyllModel(args.n, train)
        elif args.model == "pcfg":
            train_pcfg = []
            for i in train:
                i = i.split("\t")
                train_pcfg += [i[0]]#*int(i[2])
            temp = 'temp_file'
            f = open(temp, 'w')
            print >> f, "\n".join(str(i) for i in train_pcfg)
            call(["./pcfg/io","-d","1","-p","0","-g", args.grammar, temp],stdout = open('grammars/gram_pcfg.wlt', 'w'))
            lm = PCFG('grammars/gram_pcfg.wlt')
        else:
            print "not yet implemented"
            sys.exit()
        lm.create_model(train, args.smoothing)
    
        if args.model != 'pcfg':
    #This bunch of commented code was to compare with the results obtained using srilm
    #        if args.model == 'nphone':
   #             srilm_train = [" ".join(list(w)) for w in train] 
   #             srilm_test = [" ".join(list(w)) for w in test]
     #       if args.model == 'nsyll':
  #              srilm_train = [re.sub("-", " ", w) for w in train] 
  #              srilm_test = [re.sub("-", " ", w) for w in test]
 #           wb = compute(srilm_train, srilm_test, args.n, 'wbdiscount')
 #           add = compute(srilm_train, srilm_test, args.n, 'addsmooth', args.smoothing)
#            out.write(str(i)+","+"srilm_wb"+","+args.model+str(args.n)+str(args.smoothing)+","+str(wb[0])+","+str(-wb[1])+"\n")
#            out.write(str(i)+","+"srilm-add"+","+args.model+str(args.n)+str(args.smoothing)+","+str(add[0])+","+str(-add[1])+"\n")
#            print "wb", args.model, args.n, wb[0], -wb[1]
#            print "add", args.model, args.n, add[0], -add[1] 
            print it, "add-smoothing", args.model, args.n, perplexity(cross_entropy(lm, test)), logprob(lm, test)
        else:
            test = [re.sub(" ","",x) for x in test]
            print it, "add-smoothing", args.model, args.n, perplexity(cross_entropy(lm, test)), logprob(lm, test)
            os.remove('temp_file')
        out.write(str(it)+","+"add-smoothing"+","+args.model+","+str(args.n)+","+str(args.smoothing)+","+str(perplexity(cross_entropy(lm, test)))+","+str(logprob(lm,test))+"\n")
#    os.system('Rscript eval.r')
    #evaluate_model(args.lex, args.iter, args.model, args.n, args.homo, args.train, args.freq)

