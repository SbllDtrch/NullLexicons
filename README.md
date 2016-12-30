Data from Dautriche, Mahowald, Gibson, Christophe and Piantadosi, Words cluster phonetically beyond phonotactic regularities
The scripts are minimally annotated but you can always contact isabelle.dautriche@gmail.com if anything is unclear.

#### FILES DESCRIPTION
#DMGCP_Words cluster phonetically beyond phonotactic regularities_final_version.pdf
OUR PAPER!

#run_all.sh
run all the scripts with the appropriate commands to get the main results in the paper

#extract_lex_celex.py and extract_lex_french.py 
output lexicon of monomorphemes in folder celexes (one word per line)

# main.py
main script (!):
- evaluate a model (and write results in the evaluation folder)
- generate the null lexicons from a real lexicon (and write it in the Lexicons folder)
- calculate stats on that (and write in the rifles folder)
create a folder Graphs and save the lexical network graph in that if you put the graph option (see below)

options:
- --lex (real lexicon file generated from extract_lex_celex.py or extract_lex_french.py) NO DEFAULT
- --lang (language: english/german/dutch/french; DEFAULT: english)
- --fnc generate/evaluate (either generate lexicons or evaluate a model; DEFAULT: generate)
- --homo 0/1 (to exclude homophones being generated; DEFAULT: 0)
- --model nphone/nsyll/pcfg (DEFAULT: nphone)
- --n (n for ngram; works only until 2 for nsyll; doesn't matter for pcfg; DEFAULT: 3)
- --iter (number of null lexicons to be generated); DEFAULT: 2
- --cv 0/1 (match cv pattern; not used in any model presented in the paper for your entertainment only; DEFAULT: 0)
- --grammar grammars/FILE.wlt (grammar file to use when using --model pcfg; DEFAULT: grammars/grammar_german.wlt)
- --train number between 0 and 1 (portion of the lexicon to used for training in the evaluation; not used in the generate mode of the script; DEFAULT: 0.75)
- --graph 0/1 (output a network graph of the minimal pairs; DEFAULT: 0)
- --smoothing (discount used for the add-smoothing, DEFAULT: 0.01)
 
#lm.py (with methods create_model, generate, evaluate)
Additional file called inside main.py to create the models
inherits from the subclasses:
-> nphone.py
-> nsyll.py
-> pcfg.py

#evaluation.py 
Methods for words evaluation called inside main.py

#generation.py
Methods for words generation called inside main.py

#calc_stats.py
Methods calculating the lexical stats for he real and the null lexicons.
Write files into rfiles folder
Called inside main.py
Can be used independently by decommenting the portion of text at the end of the script.

#perplexity.py uses srilm for smoothing
currently not in used in the script but you can uncomment the lines in the evaluate part of main.py to compare with our current version
Currently can do either addsmooth or wbdiscount. If addsmooth, set the smoothing alpha (default = .01)

#figure_eval.r
Plot the evaluation figure of the paper comparing the results of all the models tested.

#figures.r
Plot all the figures of the paper.

### FOLDERS DESCRIPTION
#full_lex
contains the list of all lemmas for each of the 4 languages under study

#raw_lex
contains celex and Lexique, in their raw format
- http://celex.mpi.nl/
- http://www.lexique.org/docLexique.php

#celexes
contains the lexicons extracted using extract_lex_celex.py and extract_lex_french.py

#Lexicons
contains files listing real and simulated words for n null lexicons (output of main.py)

#rfiles
contains the lexical statistics generated from the files in Lexicons (output of main.py or calc_stats.py)

#evaluation
contains the evaluation scores for each model and each language

#Graph
contains the network graphs generated from main.py or calc_stats.py (optional)

#pcfg 
contains the python scripts for computing pcfg out of a corpora (available at http://web.science.mq.edu.au/~mjohnson/Software.htm)


#grammars
contains the grammars used for the pcfg for each language

#srilm
contains all the srilm package which optionally can be used to compare different smoothing methods 
http://www.speech.sri.com/projects/srilm/
