#!/usr/bin/env python

DEBUG = False

def DEBUG_SYMBOL(name):
  if not DEBUG: return False
  if name == "WHITESPACE": return True
  return name.startswith("Identifier")

class ParseTree(object):
  def __init__(self, symbol=None, root=None):
    if root:
      self.root = root
    else:
      self.root = ParseNode(symbol)
      self.root.tree = self

  def buildValues(self):
    self.root.buildValue()

  def lookup(self, *args, **kwargs):
    return self.root.lookup(*args, **kwargs)

  def lookupSingle(self, symbol):
    return self.root.lookupSingle(symbol)

  def lookupValue(self, symbol):
    return self.root.lookupValue(symbol)

  def getValue(self):
    return self.root.getValue()

  def __repr__(self):
    return "Tree:\n" + str(self.root)

class ParseNode:
  def __init__(self, symbol):
    self.children = []
    self.parent = None
    self.value = None
    self.symbol = symbol
    self.prev = None
    self.next = None

  def lookup(self, symbol, min_depth=None, max_depth=None, depth=0):
    current = None
    cnodes, nnodes = [], [self]
    while len(nnodes) > 0:
      cnodes, nnodes = nnodes, []
      found = False
      for node in cnodes:
        if node.symbol == symbol:
          if min_depth == None or depth >= min_depth:
            yield node
          found = True
        nnodes += node.children
      if found:
        depth += 1
      if max_depth != None and depth > max_depth: raise StopIteration

  def lookupSingle(self, symbol):
    return next(self.lookup(symbol))

  def lookupValue(self, symbol):
    try:
      return self.lookupSingle(symbol).getValue()
    except StopIteration: return None

  def getValue(self):
    self.buildValue()
    return self.value

  def buildValue(self):
    if self.value != None: return
    self.value = ""
    if len(self.children) == 0: return
    for child in self.children:
      child.buildValue()
      if child.value:
        self.value += child.value

  def addChild(self, node):
    if len(self.children):
      prev = self.children[-1]
      node.prev = prev
      prev.next = node
    self.children += [node]
    node.parent = self
    node.tree = self.tree

  def tostring(self, indent):
    self.buildValue()
    istr = ""
    for i in range(indent): istr += "  "
    s = "%s: %s\n" % (self.symbol, self.value)
    for child in reversed(self.children):
      s += istr + child.tostring(indent + 1)
    return s

  def __str__(self):
    return self.tostring(indent=0)

  def __repr__(self):
    return str(self)

class ParseSymbol:
  def __init__(self, node):
    self.symbol = node.symbol
    self.node = node

  def __repr__(self):
    return self.symbol

class GrammarRule:
  def __init__(self, nonterminal):
    self.nonterminal = nonterminal
    self.symbols = []
    self.expression = None

class Terminal:
  def __init__(self, name):
    self.name = name
    self.followSet = set()

  def __repr__(self):
    return self.name

  def is_terminal(self): return True

class NonTerminal:
  def __init__(self, name):
    self.name = name
    self.rules = list()
    self.firstSet = set()
    self.followSet = set()
    self.expression = None

  def __repr__(self):
    return self.name
  
  def is_terminal(self): return False

class ParseRule:
  def __init__(self, name, ruleString):
    self.nonterminal = name
    self.symbols = ruleString.split()

  def __repr__(self):
    return self.nonterminal + " -> " + " ".join(self.symbols)

class Parser(object):
  START_SYMBOL = "START"
  def __init__(self, tokenizer):
    self.rules = list()
    self.terminals = None
    self.nonterminals = dict()
    self.table = dict()
    self.tokenizer = tokenizer
    self.EMPTY_STRING = "EMPTY_STRING"

  def addTerminals(self, terminals):
    self.terminals = {name : Terminal(name) for name, value in terminals}
  
  def addRules(self, name, rules):
    nt = NonTerminal(name)
    self.nonterminals[name] = nt
    for r in rules:
      rule = ParseRule(name, r)
      self.rules += [rule]
      nt.rules += [rule]
  
  def build(self):
    self.buildFirstSets()
    self.buildFollowSets()
    self.buildRuleSets()
    for t in self.terminals:
      self.table[t] = dict()
      #if DEBUG: print "FOLLOW: ",t,self.terminals[t].followSet
      for nt in self.nonterminals:
        self.table[t][nt] = self.lookupRule(t, nt)

  def buildFirstSets(self):
    changed = True
    while changed:
      changed = False
      for _, nt in self.nonterminals.items():
        for rule in nt.rules:
          #if DEBUG: print rule
          fset = self.lookupFirstSet(rule.symbols)
          for s in fset:
            if s not in nt.firstSet:
              nt.firstSet.add(s)
              changed = True

  def buildFollowSets(self):
    changed = True
    while changed:
      changed = False
      for rule in self.rules:
        if DEBUG_SYMBOL(rule.nonterminal):
          print "building follow set for",rule.nonterminal,"current:",self.nonterminals[rule.nonterminal].followSet
        if len(rule.symbols) > 1:
          for i in range(len(rule.symbols)):
            symbol = rule.symbols[i]
            if symbol in self.terminals:
              sym = self.terminals[symbol]
            elif symbol in self.nonterminals:
              sym = self.nonterminals[symbol]
            rightSymbols = rule.symbols[i + 1:]
            if DEBUG_SYMBOL(sym.name):
              print "looking up first-set of right symbols for %s[%i] for follow(%s):" % (rule,i,sym.name)
              for s in rightSymbols: print "\t",s
            fset = self.lookupFirstSet(rightSymbols, debug=DEBUG_SYMBOL(sym.name))
            for s in fset:
              if s in self.terminals and s not in sym.followSet:
                if DEBUG_SYMBOL(sym.name):
                  print "[step 2.1] terminal %s in first set for right symbols %s, adding %s to follow set of %s" % (
                    s,rightSymbols,s,sym
                  )
                sym.followSet.add(s)
                changed = True
              elif s == self.EMPTY_STRING:
                nt = self.nonterminals[rule.nonterminal]
                for si in nt.followSet:
                  if si not in sym.followSet:
                    if DEBUG_SYMBOL(sym.name):
                      print "[step 2.2] empty string in first-set for right symbols, adding",si,"to follow set of",sym
                    sym.followSet.add(si)
                    changed = True
      for _, nt in self.nonterminals.items():
        for rule in nt.rules:
          if len(rule.symbols):
            for symbol in reversed(rule.symbols):
              if symbol == self.EMPTY_STRING: continue
              if symbol in self.terminals:
                sym = self.terminals[symbol]
              else:
                sym = self.nonterminals[symbol]
              # if sym == None: break
              if DEBUG_SYMBOL(sym.name):
                print "[step 2.3]: Considering %s for %s" % (symbol,nt)
              for s in nt.followSet:
                if s not in sym.followSet:
                  if DEBUG_SYMBOL(sym.name):
                    print "[step 2.3] adding",s,"to follow set of",sym,"because it's in follow set of",nt
                  sym.followSet.add(s)
                  changed = True
              if sym.is_terminal() or self.EMPTY_STRING not in sym.firstSet:
                if DEBUG_SYMBOL(sym.name):
                  print "[phase 2.3] break"
                break

  def buildRuleSets(self):
    for rule in self.rules:
      rule.firstSet = self.lookupFirstSet(rule.symbols)

  def lookupFirstSet(self, inSymbols, debug=False):
    symbols = list(inSymbols)
    fset = set()
    if len(symbols) == 0: return fset
    symbol = symbols[0]
    if symbol in self.terminals:
      if debug: print symbol,"is terminal, returning it as unary first-set"
      fset.add(symbol)
      return fset
    elif symbol in self.nonterminals:
      nt = self.nonterminals[symbol]
      if nt.firstSet == None: return fset
      if self.EMPTY_STRING not in nt.firstSet:
        if debug: 
          print "empty string isn't in self first-set, returning these:"
        for s in nt.firstSet:
          if debug: print s
          fset.add(s)
        return fset
      else:
        for s in nt.firstSet:
          if s != self.EMPTY_STRING:
            fset.add(s)
        symbols = symbols[1:]
        nextSet = self.lookupFirstSet(symbols)
        for s in nextSet:
          fset.add(s)
      return fset
    elif symbol == self.EMPTY_STRING:
      fset.add(self.EMPTY_STRING)
      return fset

  def lookupRule(self, terminal, nonterminal):
    nt = self.nonterminals[nonterminal]
    if DEBUG_SYMBOL(terminal) and DEBUG_SYMBOL(nonterminal):
      print "looking up rule for",terminal,"and",nonterminal
      for r in nt.rules: 
        print "rule:",r
        print "first set:",r.firstSet
        print "follow set:",nt.followSet
    selected = []
    for rule in nt.rules:
      if terminal in rule.firstSet:
        selected.append(rule)
      elif self.EMPTY_STRING in rule.firstSet and terminal in nt.followSet:
        selected.append(rule)
    if len(selected) == 0: 
      return None
    if len(selected) > 1:
      try:
        empty = next(r for r in selected if r.symbols == [self.EMPTY_STRING])
        selected.remove(empty)
      except: pass
    if len(selected) > 1:
      for r in selected:
        print "Rule:",r
      print "Follow Set:",nt.followSet
      #raise Exception("Multiple rules satisfy terminal=%s, nonterminal=%s" % (terminal,nonterminal))
      print "Multiple rules satisfy terminal=%s, nonterminal=%s" % (terminal,nonterminal)
    return selected[0]

  def parse(self, string=None, tokens=None):
    if tokens == None:
      tokens = self.tokenizer.tokenize(string)
    if not tokens: 
      if DEBUG: print "No tokens"
      return False
    if DEBUG:
      print "TOKENS:"
      for t in tokens: print t
      print "----------------------------------------------------"
    tokens = list(reversed(tokens))
    tree = self.Tree(symbol=Parser.START_SYMBOL)
    tree.string = string
    start = ParseSymbol(tree.root)
    parseStack = [start]
    while len(tokens) > 0:
      if len(parseStack) == 0:
        if DEBUG:
          print "Parse stack is empty, but tokens are left"
          print tokens
        return False
      topSymbol = parseStack.pop()
      if topSymbol.symbol == self.EMPTY_STRING: 
        continue
      topToken = tokens[-1]
      if DEBUG: print "TOP TOKEN IS %s (from %s)" % (topToken.token, topToken.value)
      if topSymbol.symbol == topToken.token:
        if DEBUG: print "TOKEN/SYMBOL MATCH"
        topSymbol.node.value = topToken.value
        tokens.pop()
      else:
        if topToken.token not in self.table:
          if DEBUG: print "Token %s not in table" % (topToken.token)
          return False
        if topSymbol.symbol not in self.table[topToken.token]:
          if DEBUG: print "Symbol '%s' not in lookup table for token '%s': %s" % (topSymbol.symbol, topToken.token, self.table[topToken.token])
          #if DEBUG: print "PARSE STACK:\n",parseStack
          #if DEBUG: print "TOKENS:\n",tokens
          return False
        if DEBUG: print "Lookup: T:",topToken.token," NT: ",topSymbol.symbol
        rule = self.table[topToken.token][topSymbol.symbol]
        if rule is None:
          if DEBUG: 
            print "No rule identified for symbol '%s' and token '%s': %s" % (topSymbol.symbol, topToken.token, self.table[topToken.token])
            print parseStack
            print tokens
          #if DEBUG: print "PARSE STACK:\n",parseStack
          #if DEBUG: print "TOKENS:\n",tokens
          return False
        if DEBUG: print "Decomposing with rule:",rule
        nodes = []
        for symbol in reversed(rule.symbols):
          node = ParseNode(symbol)
          nodes.append(node)
          sym = ParseSymbol(node)
          parseStack += [sym]
          if DEBUG: print "Adding symbol to parse stack:",sym
        for node in reversed(nodes):
          topSymbol.node.addChild(node)
    # Ensure that the remainder of the parse stack matches with the empty token set
    if DEBUG: print "FINISHED PARSING WITH TOKENS LEFT:",tokens
    if DEBUG: print "FINISHED PARSING WITH PARSE STACK:",parseStack
    while len(parseStack) > 0:
      p = parseStack.pop() 
      if DEBUG: print "REMAINING PARSE ITEM:",p
      fset = self.nonterminals[p.symbol].firstSet
      if self.EMPTY_STRING not in fset:
        if DEBUG: print "Failed parsing because of dangling symbol:",p
        return False
    return tree
