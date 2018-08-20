#!/usr/bin/env python

from tokenizer import Tokenizer
from parser import Parser, ParseTree
import type_grammar as grammar

class TypeTokenizer(Tokenizer):
  def __init__(self, *args, **kwargs):
    super(TypeTokenizer, self).__init__(*args, **kwargs)
    grammar.build_tokenizer(self)

class TypeParser(Parser):
  def __init__(self, *args, **kwargs):
    if 'tokenizer' not in kwargs:
      kwargs['tokenizer'] = TypeTokenizer()
    super(TypeParser, self).__init__(*args, **kwargs)
    grammar.build_parser(self, self.tokenizer)
    self.Tree = TypeTree

class Type(object):
  def __init__(self, tree, node=None):
    if node == None: node = tree
    self.name = tree.get_outer_type(node)
    self.subtypes = []
    subnodes = node.lookup('TemplateArg', max_depth=0, min_depth=0)
    for subnode in subnodes:
      subnode = subnode.lookupSingle('Type')
      self.subtypes.append(Type(tree, subnode))

  def __str__(self, indent=''):
    s = indent + self.name + "\n"
    for t in self.subtypes:
      s += t.__str__('  ')
    return s

class TypeTree(ParseTree):
  def get_outer_type(self, node=None):
    if node == None:
      node = self
    return node.lookupValue('Identifier')

  def get_inner_types(self, node=None):
    if node == None:
      node = self
    args = node.lookup('TemplateArg', max_depth=1, min_depth=1)
    for arg in args:
      yield arg.lookupValue('Type')

  def str(self, node=None, indent=""):
    if node == None: node = self
    otype = self.get_outer_type(node)
    if otype == node.getValue() and indent != '': return ""
    s = indent + "Outer: " + self.get_outer_type(node) + "\n"
    try:
      inners = list(node.lookup('TemplateArg', max_depth=1, min_depth=1))
      if len(inners) == 0: return s
      indent += "  "
      for i, inner in enumerate(inners):
        s += indent + "arg %i: %s\n" % (i,inner.getValue())
        s += self.str(inner, indent)
    except StopIteration: pass
    return s

  def __str__(self):
    return self.str()

parser = TypeParser()
def parse(s):
  tree = parser.parse(s)
  if not tree: return tree
  return Type(tree)

if __name__ == "__main__":
  import sys
  tree = parser.parse('std::array<std::vector<std::array<float,10>>>')
  #tree = parser.parse('std::array<float,10>')
  print tree
