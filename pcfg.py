from lm import *
import nltk
from nltk import *
import json
import time, datetime
from math import log, sqrt, exp 
import random


""" --------------pcfg model------------"""



class PCFG(LM):
    def __init__(self, grammar, ngram = None):
        self.__dict__.update(locals())
        self.n = 0
        self.ngram = ngram
        self.grammar_file = grammar
        self.nonterminal_counts = nltk.defaultdict(int)
        self.binary_rule_counts = nltk.defaultdict(int)
        self.unary_rule_counts = nltk.defaultdict(int)
        self.prod = nltk.defaultdict(list)
        self.reject = 0
        LM.__init__(self)

    def create_model(self, corpus, smoothing = 0): # corpus argument doesnt serve here but to match the LM class
        if self.ngram != None:
            self.ngram.create_model([re.sub("-","",x) for x in corpus])
        g = open(self.grammar_file, 'r')
        self.format_grammar(g, smoothing)
        LM.create_model(self, corpus, smoothing)


    def format_grammar(self, f, smoothing):
        tot = nltk.defaultdict(int)
        gram = nltk.defaultdict(lambda: nltk.defaultdict(int))
        lines = f.readlines()[1:-1]
        for line in lines:
            p, rule = line.split("\t")
            A, B = rule.split(" --> ")
            B = B.rstrip('\n')
            gram[A][B] = float(p)
            tot[A] += float(p)
#            self.prod[A].append((B, float(p)))
#            self.nonterminal_counts[A] += float(p) 
            #print A, B, len(B), freq
#            if " " in B: #BINARY RULE
#                B1, B2 = B.split()
#                self.binary_rule_counts[(A,B1,B2)] = float(p)
#            else:
#                if len(B) == 1: #UNARY RULE
#                    self.unary_rule_counts[(A,B)] = float(p)
        for i in gram.keys():
            for j in gram[i].keys():
                p_new = (gram[i][j] + smoothing) / float(tot[i] + smoothing * len(gram[i].keys()))
                self.prod[i].append((j, p_new))
                self.nonterminal_counts[i] += float(p_new)
                if " " in j: #BINARY RULE
                    B1, B2 = j.split()
                    self.binary_rule_counts[(i,B1,B2)] = float(p_new)
                else:
                    if len(j) == 1: #UNARY RULE
                        self.unary_rule_counts[(i,j)] = float(p_new)

    def format_grammar_old(self, f):
        #print ">>> Loading cfg counts ..."
        f_out = open("grammars/grammar_formated.txt", 'w')
        gram = nltk.defaultdict(lambda: nltk.defaultdict(int))
        tot = nltk.defaultdict(int)
        for line in f:
            freq, rule = line.split("\t")
            A, B = rule.split(" --> ")
            B = B.rstrip('\n')
            self.nonterminal_counts[A] += float(freq) 
            #print A, B, len(B), freq
            if " " in B: #BINARY RULE
                B1, B2 = B.split()
                self.binary_rule_counts[(A,B1,B2)] = float(freq)
                gram[A][B1+"+"+B2] = float(freq)
                tot[A] += float(freq)
            else:
                gram[A][B] = float(freq)
                tot[A] += float(freq)
                if len(B) == 1: #UNARY RULE
                    self.unary_rule_counts[(A,B)] = float(freq)

        
        for i in gram.keys():
            f_out.write(i+"\t")
            for j in sorted(gram[i].keys()):
                if j == sorted(gram[i].keys())[-1]:
                    f_out.write(j+":"+str(float(gram[i][j])/tot[i]))
                else:
                    f_out.write(j+":"+str(float(gram[i][j])/tot[i])+" | ")
            f_out.write("\n")
    
    #print ">>> Done and wrote in file"
    
    def add_prod(self, lhs, rhs):
        """ Add production to the grammar. 'rhs' can
            be several productions separated by '|'.
            Each production is a sequence of symbols
            separated by whitespace.

            Usage:
                grammar.add_prod('S', 'NP VP')
                grammar.add_prod('Num', '1|2|3|4')
        """
        prods = rhs.split('|')
        for prod in prods:
            self.prod[lhs].append(tuple(prod.strip().split(':')))
            
    def generate(self, ngen = 1):
        """Generate as many words as specified by ngen"""
        LM.generate(self, ngen)
        words = [self.filter_word() for xx in range(ngen)]
        return words
        
    def generate_one(self, symbol='Word'):
        """ Generate a random word from the
            grammar, starting with the given
            symbol.
        """
        word = ''
        tot_p = 0
        if ' ' in symbol:
            s = symbol.split(' ')
        else:
            rand_prod, p = weighted_choice(self.prod[symbol])
            tot_p += log(p, 10)
            s =  [x for x in self.prod[symbol][rand_prod][0].strip().split(",")]
        for sym in s:
            if sym in self.prod or ' ' in sym:# for non-terminals, recurse
                w, p = self.generate_one(sym)
                word += w
                tot_p += p
            else: #terminals
                word += sym 
        return word, tot_p

    def filter_word(self):
        word, p = self.generate_one(symbol='Word')
        if self.ngram != None:
            while self.ngram.evaluate(re.sub("-","",word))==0:
                word, p = self.generate_one(symbol='Word')
                self.reject +=1
        #self.p_word[word] = p
        return word
      
    
    def evaluate(self, word):
        oov, p = self.parse(word)
        return len(re.sub("-","",word)), float(oov), float(p)
        #if self.ngram != None:
        #    while self.ngram.evaluate(word)==0:
        #        word, p = self.generate_one(symbol='Word')
        #        self.reject +=1
        #self.p_word[word] = p
        #for item in corpus:
#       #     print self.p_word[item], self.ngram.evaluate(item)
        #    p += self.p_word[item]
        #return p 
        
        
        
        """
        Parsing Code from https://github.com/usami/pcfg/blob/master/pcfg_parser.py
        reuse the CYK algorithm - modification to get the likelihood of rule instead of getting the tree
        """
        
    def parse(self, word):
        return self.CKY(list(word))
    
    
    def q(self, x, y1, y2):
        """
        Return binary rule parameters for a rule such that x -> y1 y2.
        """
        return float(self.binary_rule_counts[x, y1, y2]) / self.nonterminal_counts[x]   
        
        
    def q_unary(self, x, w):
        """
        Return unary rule parameters for a rule such that x -> w.
        """
        return float(self.unary_rule_counts[x, w]) / self.nonterminal_counts[x]
        
    def CKY(self, x):
        """
        Implementation of CKY algorithm.
        """
        n = len(x) # length of word x
        pi = defaultdict(float) # DP table pi
        bp = {} # back pointers
        N = self.nonterminal_counts.keys() # set of nonterminals
        # Base case
        for i in xrange(n):
            c = x[i] 
            for node in N:
                pi[i, i, node] = self.q_unary(node, c) # if X -> x[i] not in the set of rules, assign 0
       
        # Recursive case
        for l in xrange(1, n): 
            for i in xrange(n-l):
                j = i + l
                for node in N:
                    max_score = 0
                    args = None
                    for R in self.binary_rule_counts.keys(): # search only within the rules with non-zero probability
                        if R[0] == node: # consider rules which start from node
                            Y, Z = R[1:]
                            for s in xrange(i, j):
                                if pi[i, s, Y] and pi[s + 1, j, Z]: # calculate score if both pi entries have non-zero score
                                    score = self.q(node, Y, Z) * pi[i, s, Y] * pi[s + 1, j, Z]
                                    if max_score < score:
                                        max_score = score
                                        args = Y, Z, s
                    if max_score: # update DP table and back pointers
                        pi[i, j, node] = max_score
                        bp[i, j, node] = args
        
        # Return
        if pi[0, n-1, 'Word']:
            return 0, self.recover_tree(x, pi, bp, 0, n-1, 'Word')
        else: # if the tree does not have the start symbol 'S' as the root
            max_score = 0
            args = None
            for node in N:
                if max_score <= pi[0, n-1, node]:
                    max_score = pi[0, n-1, node]
                    args = 0, n-1, node
            return self.recover_tree(x, pi, bp, *args)

    def recover_tree(self, x, pi, bp,  i, j, X):
        """
        Return the list of the parsed tree with back pointers.
        """

        if i == j:
            return log(pi[i, j, X], 10)
        else:
            Y, Z, s = bp[i, j, X]
            return self.recover_tree(x, pi, bp, i, s, Y) + self.recover_tree(x, pi, bp, s+1, j, Z)



"-------utils"

def weighted_choice(weights):
    totals = []
    running_total = 0
    for w, p in weights:
        running_total += float(p)
        totals.append(running_total)

    rnd = random.random() * running_total
    #print rnd
    for i, total in enumerate(totals):
        if rnd < total:
            return i, float(weights[i][1])

