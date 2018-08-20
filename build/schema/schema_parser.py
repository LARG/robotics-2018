#!/usr/bin/env python

from tokenizer import Tokenizer
from parser import Parser, ParseTree
import schema_grammar as grammar

class SchemaTokenizer(Tokenizer):
  def __init__(self, *args, **kwargs):
    super(SchemaTokenizer, self).__init__(*args, **kwargs)
    grammar.build_tokenizer(self)

class SchemaParser(Parser):
  def __init__(self, *args, **kwargs):
    if 'tokenizer' not in kwargs:
      kwargs['tokenizer'] = SchemaTokenizer()
    super(SchemaParser, self).__init__(*args, **kwargs)
    grammar.build_parser(self, self.tokenizer)
    self.Tree = SchemaForest

class FunctionCall:
  def __init__(self, name, args):
    self.name = name
    self.args = args

  def __repr__(self):
    return self.__str__()

  def __str__(self):
    return "%s(%s)" % (self.name, ",".join(map(str,self.args)))

class SchemaForest(ParseTree):
  def get_trees(self):
    return [SchemaTree(root=node) for node in self.lookup("SchemaDeclaration") if len(node.children) > 0]
  
  def remove_directives(self):
    roots = [r for r in self.lookup('SchemaDeclaration') if len(r.children) > 0]
    for root in roots:
      root.children = [root.children[2], root.children[4]]
      declarations = root.lookup('MemberDeclaration')
      for d in declarations:
        d = d.children[0]
        if d.symbol in ['SchemaField']:
          d.children = d.children[2:]
          del d.children[-2]
        else:
          d.children = []

class SchemaTree(ParseTree):
  def __init__(self, *args, **kwargs):
    super(SchemaTree, self).__init__(*args, **kwargs)

  def is_external(self):
    node = self.lookupSingle('SchemaDeclarationFunction')
    return node.getValue().endswith("DECLARE_EXTERNAL_SCHEMA")
   
  def get_fields(self):
    nodes = self.lookup('SchemaField')
    fcalls = []
    for n in nodes:
      func = n.lookupValue('SCHEMA_FIELD')
      type = n.lookupValue('TypeIdentifier')
      name = n.lookupValue('NameIdentifier')
      value = n.lookupValue('ValueIdentifier')
      fcall = FunctionCall(func,[type, name, value])
      fcalls.append(fcall)
    return fcalls

  def get_pre_deserialization(self):
    nodes = self.lookup('SchemaPreDeserialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls
  
  def get_post_deserialization(self):
    nodes = self.lookup('SchemaPostDeserialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls

  def get_custom_serialization(self):
    nodes = self.lookup('SchemaSerialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls

  def get_custom_deserialization(self):
    nodes = self.lookup('SchemaDeserialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls
  
  def get_pre_serialization(self):
    nodes = self.lookup('SchemaPreSerialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls
  
  def get_post_serialization(self):
    nodes = self.lookup('SchemaPostSerialization')
    fcalls = []
    for n in nodes:
      args = [a.getValue() for a in n.lookup("MemberArgument")]
      fcall = FunctionCall(n.children[0].getValue(), args)
      fcalls.append(fcall)
    return fcalls

  def get_schema_name(self):
    node = self.lookupSingle('SchemaName')
    return node.getValue()

parser = SchemaParser()
def parse(path):
  with open(path, 'r') as f:
    s = f.read()
    forest = parser.parse(s)
    return forest

def check(path):
  with open(path, 'r') as f:
    for line in f:
      for func in parser.tokenizer.functions:
        if func == "NO_SCHEMA": continue
        if func in line: return True
  return False

if __name__ == "__main__":
  import sys
  forest = parse(sys.argv[1])
  if not forest:
    print "Parse Error"
    sys.exit(1)
  for t in forest.get_trees():
    print t.get_schema_name()
    print t.get_fields()
    print t.is_external()
