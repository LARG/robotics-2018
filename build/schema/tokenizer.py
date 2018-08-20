#!/usr/bin/env python
import re

class Token:
  def __init__(self, token, value):
    self.token = token
    self.value = value

  def __repr__(self):
    return "%s: %s" % (self.token, self.value)

class Tokenizer(object):
  def __init__(self):
    self.expressions = []
    self.pvalue = self.cvalue = None
    self.tokens = None

  def remove_comments(self, string):
    string = re.sub(r'\/\/.*$', '', string, count=0, flags=re.MULTILINE)
    string = re.sub(r'/\*.*?\*/', '', string, count=0, flags=re.DOTALL)
    return string


  def tokenize(self, string):
    string = self.remove_comments(string)
    self.pvalue = self.cvalue = ''
    self.tokens = []
    def checkToken(force=False):
      for t, e in self.expressions:
        e = '^' + e + '$'
        accept = False
        if force and re.match(e, self.cvalue, re.DOTALL):
          accept = True
        elif re.match(e, self.pvalue, re.DOTALL) and not re.match(e, self.cvalue, re.DOTALL):
          accept = True
        elif t == self.pvalue:
          accept = True
        elif self.cvalue.endswith(t):
          exp = self.cvalue[0:-len(t)]
          kw = t
          if len(exp) > 0:
            self.tokens += [Token('EXPRESSION',exp)]
          self.tokens += [Token(t,t)]
          self.cvalue = ''
          break
        if accept:
          self.tokens += [Token(t,self.pvalue)]
          if self.cvalue:
            self.pvalue = self.cvalue = self.cvalue[-1]
          break
    for s in string:
      self.cvalue += s
      checkToken()
      self.pvalue = self.cvalue
    checkToken(force=True)
    return self.tokens
