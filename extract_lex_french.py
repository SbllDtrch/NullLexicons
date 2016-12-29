#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import nltk
import argparse
import re


path = "/Users/isa/Desktop/lexicon/french_lex.txt"

def read_lines(path):
    """ Return lines in given celex file. """
    return [line.rstrip().split("\t") for line in open(path, "r").readlines()]

def build_corpus(path, language, lemma, mono):
    """ Return corpus from celex, given path and parameters. """
    lines = read_lines(path); corpus = []; 
    for line in lines[1:]:
        if line[0].islower() and line[5] != "p" and "-" not in line[0]  and " " not in line[0] and line[3] != "ONO" and len(line[0]) > 1 and float(line[34]) == 1 and float(line[13]) == 1:
            corpus += [(line[22], float(line[6]))]
    return [i for i in corpus if i != None]

def build_real_lex(path, lemma, language, mono, homo, minlength, maxlength,celex_list):
    corpus = build_corpus(path, language, lemma, mono)
    corpus = [c for c in corpus if float(c[1]) > 0]
    #corpus = sorted(corpus, key=lambda x: float(x[0]))
    corpus = [c for c in corpus if (len(re.sub("-", "", c[0])) >= minlength and len(re.sub("-", "", c[0])) <= maxlength)]
    if args.syll != 1: corpus = [re.sub("-","",x[0]) for x in corpus] 
    print len(corpus)
#       lines = read_lines(path); 
    ortho = nltk.defaultdict(str)
#    for i in lines:
#    	if i[5] != "p":
#        	ortho[i[1]] = i[0]


    if homo == 0: corpus = list(set(corpus))
    print ">>>TOTAL NB OF WORDS", len(corpus)
    f = open("celexes/" + "_".join([str(i) for i in celex_list]) + ".txt", "w")
    for c in corpus: f.write(c[0]+"\n")
    f.close()
    return corpus


parser = argparse.ArgumentParser()
parser.add_argument('--lemma', metavar='--lem', type=str, nargs='?',
                    help='lemma or wordform in celex', default="lemma")
parser.add_argument('--mono', metavar='--mono', type=int, nargs='?',
                    help='1 for mono only, 0 ow', default=1)
parser.add_argument('--homocel', metavar='--homocel', type=int, nargs='?',
                    help='1 for allow homo in celex, 0 ow', default=0)
parser.add_argument('--minlength', metavar='--minl', type=int, nargs='?',
                    help='minimum length of word allowed from celex', default=4)
parser.add_argument('--maxlength', metavar='--maxl', type=int, nargs='?',
                    help='maximum length of word allowed from celex', default=8)
parser.add_argument('--language', metavar='--lang', type=str, nargs='?',
                    help='', default="french")
#
parser.add_argument('--syll', metavar='--syll', type=int, nargs='?', help='include syll in celex', default=0)
parser.add_argument('--pcfg', metavar='--pcfg', type=int, nargs='?', help='format for pcfg', default=0)
parser.add_argument('--inputsim', metavar='--inputsim', type=str, nargs='?', help='use this if you want to input a file that contains simulated lexicons instead of ngrams. the file should be a csv in format simnum,word', default="none")
parser.add_argument('--cv', metavar='--cv', type=int, nargs='?', help='put 1 here to match for CV pattern', default=0)

args = parser.parse_args()

celex_list = [args.lemma, args.language, args.mono, args.homocel, args.minlength, args.maxlength]

if args.syll == 1: celex_list = ['syll_'] + celex_list

a = build_real_lex(path, args.lemma, args.language, args.mono, args.homocel, args.minlength, args.maxlength, celex_list)
argslist = "_".join([str(j) for j in celex_list] + [str(args.homocel), str(args.cv)])


