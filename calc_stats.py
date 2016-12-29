from nltk import *
import random, sys, re, os
from math import log, sqrt, exp
import math
import nltk
import argparse
import itertools
import Levenshtein
import time, datetime
import pickle
t = time.time()
import random, sys, re
from networkx import *
import networkx as nx
import matplotlib.pyplot as plt
from collections import *
import numpy as np

"""Take in simulated lexicons file with -1 for real lexicon and 0-N for simulated lexicon
and output lexical stats"""

try: 
    os.mkdir('rfiles')
except OSError: 
    pass

def find_minimal_pair_diff(word1, word2):
    """ return minimal difference, only use when you know it's an mp"""
    for i in range(len(word1)):
        if word1[i] != word2[i]: return [word1[i], word2[i]]

def entropy_from_dict(d):
    """Input dictionary, return the entropy."""
    prob = [float(i)/sum(d.values()) for i in d.values()]
    return sum([- p * math.log(p, 2) for p in prob ])

def num_ambiguous(d):
    """Find how many values in the dict are 1.
    A word is unique at that length if it has a 1 in its start_dict."""
    return float(sum([i for i in d.values() if i != 1]))

def celex_diphthong_sub(word):
    """ Do celex dipthong subs. """
    word = re.sub("2", "#I", word)
    word = re.sub("4", "QI", word)
    word = re.sub("6", "#U", word)
    word = re.sub("7", "I@", word)
    word = re.sub("8", "E@", word)
    word = re.sub("9", "U@", word)
    word = re.sub("X", "Oy", word)
    word = re.sub("W", "ai", word)
    word = re.sub("B", "au", word)
    word = re.sub("K", "EI", word)
    word = re.sub("'", "", word)
    word = re.sub('"', "", word)
    return word

def write_lex_stats(b, num, f, f2, d_pos, lang, Graph = False):
    """Use Levenshtein package to calcualte lev and count up mps, neighbors, etc"""
    total = 0.
    total_diff = nltk.defaultdict(int)
    total_same = nltk.defaultdict(int)
    mps = 0
    neighbors = 0
    homophones = 0
    lev_total = 0.0
    lev_total_diff =0.0
    lev_total_same =0.0
    diff = 0
    init = 0
    last = 0
    specific_mps = defaultdict(int)
    specific_mps_init = defaultdict(int)
    ndict = nltk.defaultdict(int)
    mdict = nltk.defaultdict(int)
    hdict = nltk.defaultdict(int)
    uniq = nltk.defaultdict(int)
    avg_lev = nltk.defaultdict(list)
 
    tot = len(b)*1.0
    g = nx.Graph()
    g.l = {}
    lengths_all = nltk.defaultdict(int)
    for item in b:
        g.add_node(item) 
        length = len(item)
        lengths_all[len(item)] += 1
    for item in itertools.combinations(b, 2):
        lev = Levenshtein.distance(item[0], item[1])
        if len(item[0]) == len(item[1]):
            avg_lev[item[0]].append(lev)
            avg_lev[item[1]].append(lev)
        if lev == 0: 
            homophones += 1
            hdict[item[0]] += 1
        elif lev == 1:
            g.add_edge(item[0], item[1])
            neighbors += 1
            ndict[item[0]] += 1
            ndict[item[1]] += 1
            if (d_pos[len(item[0])][item[0]] != d_pos[len(item[1])][item[1]]):
                diff += 1

            if len(item[0]) == len(item[1]): #if it's a minimal pair
                l = len(item[0])
                pair_ph = find_minimal_pair_diff(item[0], item[1])
                specific_mps["_".join(sorted(pair_ph))] += 1              
                pos1 = item[0].index(pair_ph[0])
                pos2 = item[1].index(pair_ph[1])
                if (d_pos[len(item[0])][item[0]] != d_pos[len(item[1])][item[1]]):
                    lev_total_diff += lev
                    total_diff[len(item[0])] += 1
                else:
                    lev_total_same += lev
                    total_same[len(item[0])] += 1
                if pos1 == pos2 and pos1 == 0:
                    specific_mps_init["_".join(sorted(pair_ph))] += 1              
                    init += 1
                if pos1 == pos2 and pos1 == len(item[0])-1:
                    last += 1
                mps += 1
                mdict[item[0]] += 1#*log(dict_b[item[1]])
                mdict[item[1]] += 1#*log(dict_b[item[0]])
        
        uniq[item[0]] = 1
        total += 1
        lev_total += lev
    
    poss_same = nltk.defaultdict(int)
    poss_diff = nltk.defaultdict(int)
    for l in d_pos.keys():
        count = nltk.defaultdict(int)
        for cat in ['A', 'ADV', 'C', 'ART', 'N', 'PRON', 'NUM', 'EXP', 'V', 'PREP', 'NOM', 'VER', 'PRO', 'PRE', 'AUX', 'ADJ', 'CON']:
            count[cat] = len([i for i in d_pos[l].keys() if d_pos[l][i] == cat])
            poss_same[l] += (count[cat] * (count[cat] - 1)) / 2
        for p in itertools.combinations(count.keys(), 2):
            poss_diff[l] += count[p[0]] * count[p[1]]
        if poss_diff[l] == 0: poss_diff[l] = 1
        if poss_same[l] == 0: poss_same[l] = 1
#        total_diff[l] = 1.0*total_diff[l]/ poss_diff[l]
#        total_same[l] = 1.0*total_same[l]/ poss_same[l]
#        print l, total_diff[l], total_same[l]
    total_d = 1.0*sum(total_diff.values())/sum(poss_diff.values())
    total_s = 1.0*sum(total_same.values())/sum(poss_same.values())
    Gcc=nx.connected_component_subgraphs(g)
    #print num, len(Gcc[0]),  len(Gcc[1])
#    print "neighbors", neighbors
#    print "average clustering", average_clustering(g)
    if graph == True:
        plt.figure(figsize=(50,50))
        pos=nx.spring_layout(g)
        nx.draw_networkx(g,pos, with_labels = False,  node_size = 40, edge_color = '0.8', node_color='k')
        plt.savefig('graph/' + str(num))
    conf = specific_mps["b_p"] + specific_mps["d_t"] + specific_mps["g_k"] + specific_mps["f_v"] + specific_mps["s_z"] + specific_mps["S_Z"]
    dist = specific_mps["t_Z"] + specific_mps["d_S"] + specific_mps["g_f"] + specific_mps["p_z"] + specific_mps["k_v"] + specific_mps["b_s"]
    conf_init = specific_mps["b_p"] + specific_mps["d_t"] + specific_mps["g_k"] + specific_mps["f_v"] + specific_mps["s_z"] + specific_mps["S_Z"]
    dist_init = specific_mps["t_Z"] + specific_mps["d_S"] + specific_mps["g_f"] + specific_mps["p_z"] + specific_mps["k_v"] + specific_mps["b_s"]
    f.write(",".join([str(x) for x in [num, len(hdict), len(b) - (len(uniq) - len(hdict)) - 1 , mps, neighbors, lev_total/total, len(b), nx.average_clustering(g), nx.transitivity(g), len(nx.connected_component_subgraphs(g)[0])/tot, specific_mps["b_p"], specific_mps["d_t"], specific_mps["g_k"], total_d, total_s, conf, dist, conf_init, dist_init, diff/neighbors, init , last]]) + "\n")

    for item in b:
        if len(item) < 15:
            f2.write(",".join([str(num), str(item), str(hdict[item]), str(mdict[item]/(hdict[item] + 1.)), str(ndict[item]/(hdict[item] + 1.)), str(1.0*sum(avg_lev[item])/len(avg_lev[item])), str(len(item))]) + "\n")
    return


def write_all(inputsim, graph, lang):
    if lang == "english":
        all_lex = "./full_lex/lem_eng.txt"
    elif lang == "dutch":
        all_lex = "./full_lex/lem_dutch.txt"
    elif lang == "german":
        all_lex = "./full_lex/lem_german.txt"
    else:
        all_lex = "./full_lex/french_lex.txt"
    
    if lang != "french":
        lex = [i.rstrip().split("\\") for i in open(all_lex).readlines()]
        l = [(i[0], celex_diphthong_sub(re.sub("\.", "", i[1])), i[2]) for i in lex]
        l = sorted(l, key=lambda tup: tup[0], reverse=False)# !!!! least frequent homophones
        l = [(i[1], i[2]) for i in l]
        d_cat = dict(l)
    else:
        lex = [i.rstrip().split("\t") for i in open(all_lex).readlines()[1:]]
        l = [(i[6], re.sub("\.", "", i[1]), i[3]) for i in lex]
        l = sorted(l, key=lambda tup: tup[0], reverse = False)
        l = [(i[1], i[2]) for i in l]
        d_cat = dict(l)
    
    ###global file
    ftowrite = inputsim.split("/")[-1][:-4]
    f = open("rfiles/" + "global_" + ftowrite + ".txt", "w")
    f.write("lexicon,homophones_type,homophones_token,mps,neighbors,avg_lev,num_words,avg_cluster,transitivity,big_comp,bppair,tdpair,kgpair,avg_lev_diff,avg_lev_same,confpairs,distpairs,confpairs_init,distpairs_init,diffpos,mpsinit,mpslast\n")
    ###word-level stats
    f2 = open("rfiles/" + "indwords_" + ftowrite + ".txt", "w")
    f2.write("lexicon,word,homophones,mps,neighbors,avg_lev,length\n")
    inputlist = nltk.defaultdict(list)
    lines = [line.strip().split(",") for line in open(inputsim).readlines()]
#    lines = [line for line in lines if len(re.sub("-","",line[1])) >= minlength and len(re.sub("-","",line[1])) <= maxlength]
    for item in lines: #divide up lexicons
        inputlist[int(item[0])] += [re.sub("-","",item[1])]
    print "writing real lexicon stats"
    length = set([len(i) for i in inputlist[-1]])
    d_pos = nltk.defaultdict(lambda: nltk.defaultdict(str))
    for l in length:
        for k in inputlist[-1]:
            if len(k) == l:
                if k not in d_cat.keys():
                    print l, k
        d_pos[l] = dict((k, d_cat[k]) for k in inputlist[-1] if len(k) == l)
#        print l, len(d_pos[l].keys()), "N", len([i for i in d_pos[l].keys() if d_pos[l][i] == "N"]), "V", len([i for i in d_pos[l].keys() if d_pos[l][i] == "V"])
#    print [k for k in inputlist[-1] if k not in d_cat.keys()]
    write_lex_stats(inputlist[-1], "real", f, f2, d_pos, lang)
    for i in range(0, max(inputlist.keys()) + 1):
        print "writing simulated lexicon stats: ", str(i)
        #shuffle grammatical categories according to length
        temp =  nltk.defaultdict(lambda: nltk.defaultdict(str))
        for l in length:
            keys = [k for k in inputlist[i] if len(k) == l]
            val = d_pos[l].values()
            random.shuffle(val)
            temp[l] = dict(zip(keys, val))
        write_lex_stats(inputlist[i], i, f, f2, temp, lang, graph)
    f.close()
    f2.close()
    return


## for independent usage
#parser = argparse.ArgumentParser()
#parser.add_argument('--inputsim', metavar='--inputsim', type=str, nargs='?', help='use this if you want to input a file that contains simulated lexicons instead of ngrams. the file should be a csv in format simnum,word', default="none")
#parser.add_argument('--lang', metavar='--lang', type=str, nargs='?', help='languaage english/dutch/german/french', default="english")
#args = parser.parse_args()
#write_all(args.inputsim, 0, args.lang)
