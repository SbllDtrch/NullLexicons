

class LM:
  def __init__(self):
    ''' store the model'''
    self.has_model = False # change to True when the model has been generated.
    pass

  def create_model(self, corpus, smoothing = None):
    ''' override this method to generate a model.  
    The model is a tuple whose contents depend onthe type of 
    smoothing to be performed.
    '''
    self.has_model = True
    pass
 
  def generate(self, l = None , ngen = 1):
    ''' generate words from lm
    '''
    self.has_model = True
    pass

  def evaluate(self, corpus):
    ''' Given a tokenized test sentence, use the model
    to evaluate its (smoothed or not) probability.
    Return the calculated probability
    '''
    if not self.has_model:
      return 0.0
    pass
