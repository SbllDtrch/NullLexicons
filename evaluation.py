from math import log, exp

def logprob(model, words):
    p = 0.0
    for w in words:
        temp_n, temp_oov, temp_p = model.evaluate(w)
        p += temp_p
    return p


def cross_entropy(model, words):
  W_T = 0.0
  e = 0.0
  n = 0.0
  oov = 0.0
  for w in words:
    temp_n, temp_oov, temp_e = model.evaluate(w)
    e += temp_e
    oov += temp_oov
    n += temp_n
  print oov
  return -e/(n-oov)
  
def perplexity(e):
	return 10**(e)
