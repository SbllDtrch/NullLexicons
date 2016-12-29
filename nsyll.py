from lm import *
from math import log
import nltk
import time
import random
import collections
import itertools

""" --------------nsyll model------------"""

def multichooser(context,pd,n):
    """ Return random choice from multinomial cfd. """
    context = "".join(context)
    cumprob = 0
    rand = random.random()
    possibles = pd[n][context].keys()
    possibles.sort()
    for possible in possibles:
        cumprob += pd[n][context][possible]
        if cumprob > rand: return possible
    return
    
    
    
class NsyllModel(LM):
    def __init__(self, n, corpus, gen = 0):
        self.__dict__.update(locals())
        self.n = n
        self.cfd = collections.defaultdict(lambda: collections.defaultdict(lambda: collections.defaultdict(int)))
        self.cpd = collections.defaultdict(lambda: collections.defaultdict(lambda: collections.defaultdict(int)))
        self.alpha = collections.defaultdict(lambda: collections.defaultdict(int))
        self.smoothing = 0
        self.generation = gen
        self.mini=1
        self.minalpha = 1
        LM.__init__(self)

    def create_model(self, corpus, smoothing = 0):
        """Update cfd using ngrams for syllables"""
        unigrams = []
        for k in range(1,self.n+1):
            for item in corpus:
                 item, ortho, f = item.split(" ")
                 item_ngrams = nltk.ngrams(["["]*(k-1) + [i for i in item.split("-")] + ["]"], k)
                 for ng in item_ngrams:
                    self.cfd[k]["-".join(ng[:-1])][ng[-1]] += 1 * float(f)#.inc(ng[-1])
                    unigrams += [ng[-1]]
        U = len(set(unigrams))
        units = set(unigrams)
        self.smoothing = smoothing
        for k in self.cfd.keys():
            for i in self.cfd[k].keys():
                pbak = 0
                for j in self.cfd[k][i].keys():
            	    self.cpd[k][i][j] = (self.cfd[k][i][j] + smoothing) / float(sum(self.cfd[k][i].values()) + smoothing*U)
                    if self.cpd[k][i][j] < self.mini:
                        self.mini = self.cpd[k][i][j]
                    pbak += self.cpd[k-1]["-".join(i.split("-")[1:])][j]
                if self.smoothing:
                    self.alpha[k][i] = (1 - sum(self.cpd[k][i].values())) / float(1 - pbak)
                    if self.alpha[k][i] < self.minalpha and self.alpha[k][i] > 0:
                        self.minalpha = self.alpha[k][i]
        print "mini", self.mini

        #for generation with smoothing 
        if self.smoothing and self.generation:
            grams = [i for i in itertools.product(units, repeat = self.n)]
            for i, g in enumerate(grams):
                self.cpd[self.n]['-'.join(g[:-1])][g[-1]] = self.backoff(self.n, '-'.join(g[:-1]), g[-1])
            for k in range(1,self.n):
                grams_border = [i for i in itertools.product(units, repeat = k)]
                for g in grams_border:
                    s = ["["]*(self.n - k) + [g]
                    self.cpd[self.n]['-'.join(s[:-1])][s[-1]] = self.backoff(self.n, '-'.join(s[:-1]), s[-1])
 

        LM.create_model(self, corpus, smoothing)

    def generate_one(self, n):
        """Generate one word from ngram model."""
        word = ["["]*(self.n - 1)
        while True:
            context = "-".join(word[(len(word) - (self.n -1)):len(word)])
            word = word + ["-"] + [multichooser(context, self.cpd, self.n)]
            if word[-1] == "]":
                break
            if word[-1] == "[" or word[-1] == None:
                word = ["("]*(self.n - 1)
        return "".join(word[(self.n):-2])

    def generate(self, ngen = 1):
        """Generate as many words as specified by ngen"""
        LM.generate(self, ngen)
        words = [self.generate_one(self.n) for xx in range(ngen)]
        return words
    
                                                           
    def evaluate(self, word):
        """ get the log probability of generating a given word under the language model """
        LM.evaluate(self, word)
        p=0
        oov =0
        word = [i for i in word.split("-")] + [")"]
        l = len(word)
        fifo = ["["]*(self.n -1)
        for ch in word:
            context = "-".join(fifo[(len(fifo) - (self.n - 1)):len(fifo)])
            pbak =  self.backoff(self.n, fifo, ch)
            if pbak != 0:
                p += log(pbak,10)
            else:
                p += log(self.mini, 10)
                oov +=1
            fifo.append(ch)
        return l,oov,p

    def backoff(self, n, f, c):
        h = "-".join(f[(len(f) - (n - 1)):len(f)])
        h_ = "-".join(f[(len(f) - (n - 2)):len(f)])
        if c in self.cpd[n][h].keys():#seen ngram
            return self.cpd[n][h][c]
        elif h not in self.alpha[n].keys() and n > 0:#context never been observed
            return self.backoff(n-1, f[1:], c)
        elif c not in self.cpd[n-1]["-".join(f[1:])].keys() and n > 0:#unseen phone in previous n
            return self.alpha[n][h] * self.backoff(n-1, f[1:], c)
        elif n > 0:
            return self.alpha[n][h] * self.cpd[n-1][h_][c]
        else:
            return 0
       
