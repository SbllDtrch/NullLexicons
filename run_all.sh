## STEP 1: extract the list of monomorphemes (our real lexicons)
#the max set here is a randomly high number (we want all words)
#all other parameters are the default ones
#note that for the French lexicon, the words resulting from these scripts have been manually checked: i.e., you cannot get the result we have automatically, because the corpora is not well annotated for monomorphemes (although it says so)

python extract_lex_celex.py --min 0 --max 40 --lang german
python extract_lex_celex.py --min 0 --max 40 --lang german --pcfg 1
python extract_lex_celex.py --min 0 --max 40 --lang german --syll 1
python extract_lex_celex.py --min 0 --max 40 --lang dutch
python extract_lex_celex.py --min 0 --max 40 --lang dutch --pcfg 1
python extract_lex_celex.py --min 0 --max 40 --lang dutch --syll 1
python extract_lex_celex.py --min 0 --max 40 --lang english
python extract_lex_celex.py --min 0 --max 40 --lang english --pcfg 1
python extract_lex_celex.py --min 0 --max 40 --lang english --syll 1
python extract_lex_french.py --min 0 --max 40

## STEP 2: evaluate the different models for each language
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 1
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 2
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 3
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 4
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 5
python main.py --lang english --lex celexes/lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 6
python main.py --lang english --lex celexes/syll__lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 1
python main.py --lang english --lex celexes/syll__lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 2
python main.py --lang english --lex celexes/pcfg__lemma_english_1_0_0_40.txt --fnc evaluate --iter 30 --model pcfg --grammar grammars/grammar_english.wlt

python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 1
python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 2
python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 3
python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 4
python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 5
python main.py --lang dutch --lex celexes/lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 6
python main.py --lang dutch --lex celexes/syll__lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 1
python main.py --lang dutch --lex celexes/syll__lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 2
python main.py --lang dutch --lex celexes/pcfg__lemma_dutch_1_0_0_40.txt --fnc evaluate --iter 30 --model pcfg --grammar grammars/grammar_dutch.wlt


python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 1
python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 2
python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 3
python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 4
python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 5
python main.py --lang german --lex celexes/lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 6
python main.py --lang german --lex celexes/syll__lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 1
python main.py --lang german --lex celexes/syll__lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 2
python main.py --lang german --lex celexes/pcfg__lemma_german_1_0_0_40.txt --fnc evaluate --iter 30 --model pcfg --grammar grammars/grammar_german.wlt


python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 1
python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 2
python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 3
python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 4
python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 5
python main.py --lang french --lex celexes/lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nphone --n 6
python main.py --lang french --lex celexes/syll__lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 1
python main.py --lang french --lex celexes/syll__lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model nsyll --n 2
python main.py --lang french --lex celexes/pcfg__lemma_french_1_0_0_40.txt --fnc evaluate --iter 30 --model pcfg --grammar grammars/grammar_french.wlt

Rscript figure_eval.r


## STEP 3: generate null lexicons using the best mode of the evaluation and extract lexical stats
# here are the command only for the best model
python main.py --lex celexes/lemma_english_1_0_0_40.txt --lang english --model nphone --n 5
python main.py --lex celexes/lemma_dutch_1_0_0_40.txt --lang dutch --model nphone --n 5
python main.py --lex celexes/lemma_german_1_0_0_40.txt --lang german --model nphone --n 5
python main.py --lex celexes/lemma_french_1_0_0_40.txt --lang french --model nphone --n 5

Rscript figures.r
