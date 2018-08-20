def build_tokenizer(tokenizer):
  tokenizer.expressions += [
    ('LPAREN', r'\('),
    ('RPAREN', r'\)'),
    ('LANGLE', r'<'),
    ('RANGLE', r'>'),
    ('COMMA', r','),
    ('COLON', ':'),
    ('SIMPLE_IDENTIFIER', r'\w+'),
    ('WHITESPACE', r'\s+')
  ]

def build_parser(parser, tokenizer):
  parser.addRules(parser.START_SYMBOL, ['Boundary Type'])
  parser.addRules('Boundary', ['WHITESPACE', 'EMPTY_STRING'])
  parser.addRules('Type', ['Identifier TypeEnd'])
  parser.addRules('Identifier', ['SIMPLE_IDENTIFIER IdentifierEnd'])
  parser.addRules('IdentifierEnd', ['COLON COLON Identifier', 'EMPTY_STRING']), 
  parser.addRules('TypeEnd', ['TemplateArgs', 'EMPTY_STRING'])
  parser.addRules('TemplateArgs', ['LANGLE TemplateArgList RANGLE'])
  parser.addRules('TemplateArgList', ['TemplateArg Boundary TemplateArgsEnd'])
  parser.addRules('TemplateArg', ['Type'])
  parser.addRules('TemplateArgsEnd', ['COMMA Boundary TemplateArgList', 'EMPTY_STRING'])
  parser.addTerminals(tokenizer.expressions)
  parser.build()
