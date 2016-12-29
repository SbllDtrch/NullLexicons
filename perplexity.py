#!/usr/bin/env python
'''Calculates the perplexity using SRLIM
Using ngram-count and ngram from SRILM

'''
import os, sys, re, tempfile, subprocess, getopt

NGRAM = ["./srilm/bin/macosx/ngram"]
NGRAM_COUNT = ["./srilm/bin/macosx/ngram-count"]


"""set where srilm lives"""
for n in NGRAM_COUNT:
    if os.access(n, os.X_OK): ngramcount = n
if not ngramcount:
    raise ValueError('%s is not a working binary' %ngramcountlist[0])
for n in NGRAM:
    if os.access(n, os.X_OK): ngram = n
if not ngram:
    raise ValueError('%s is not a working binary' %ngramlist[0])

def turn_to_txt(f, fname):
    '''if f is a list, turn it to a temp txt file
    if it's already a txt file, check to see if it exists
    '''
    if isinstance(f, list):
        #print "converting to tmp text file"
        outfile = open(fname + '.tmp', 'w')
        for item in f:
            outfile.write(item + "\n")
        return fname + '.tmp'
    elif isinstance(f, str):
        if os.path.isfile(f): return f
    else:
        print "PROBLEM WITH FILE TYPE"
        return

def clean_up():
    """Remove tmp files"""
    os.remove('tmp.lm')
    try: 
        os.remove('train.tmp')
        os.remove('test.tmp')
    except OSError: pass
    return



def compute(train, test, order=3, smoothing='wbdiscount', add=.1):
    '''Calculates perplexity between 2 text files (one sentence per line)
    or pass in lists of training and text (which will be converted to tmp txt file)
    returns ppl and log prob from srilm
    ''' 
    
    train, test = turn_to_txt(train, 'train'), turn_to_txt(test, 'test')
    


    a = [ngramcount, '-text', train, '-lm', 'tmp.lm', '-order', str(order)] 
    for o in range(1, order + 1):
        if smoothing == 'addsmooth': 
            a += ['-' + smoothing + str(o), str(add), '-gt'+str(o)+'min', '0']
        else: a += ['-' + smoothing + str(o), '-gt'+str(o)+'min', '0']
    
    #print "ngram-count call ", " ".join(a)
    rv = subprocess.call(a)
    
    b = [ngram, '-lm', 'tmp.lm', '-ppl', test, '-order', str(order)]
    #print "ngram call ", " ".join(b)
    proc= subprocess.Popen(b, stdout=subprocess.PIPE)
    
    out, err = proc.communicate()
    del proc

    ppl = float(re.search("ppl=\s+-?(\d+(\.\d+)?)", out).group(1))
    logprob = float(re.search("logprob=\s+-?(\d+(\.\d+)?)", out).group(1))
    clean_up()
    return ppl, logprob
    
    
def _usage():
    print >>sys.stderr, '''
USAGE
    $ python perplexity.py testfile trainingfile
    
    files: files with one sentence per line
    
SRILM: http://www.speech.sri.com/projects/srilm
    '''
    
    
    
    
if __name__ == '__main__':        
    try:
        opts,args=getopt.getopt(sys.argv[1:],'hc:n:', ['help'])
    except getopt.GetoptError:
        # print help information and exit:
        _usage()
        sys.exit(2)

    if len(args) != 2:
        _usage()
        sys.exit(1)

    print compute(args[0], args[1])
    
    
    
