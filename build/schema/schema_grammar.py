def build_tokenizer(tokenizer):
  tokenizer.functions = [
    'DECLARE_INTERNAL_SCHEMA', 'DECLARE_EXTERNAL_SCHEMA', 
    'SCHEMA_FIELD', 
    'SCHEMA_METHODS', 'NO_SCHEMA', 
    'SCHEMA_SERIALIZATION', 'SCHEMA_DESERIALIZATION',
    'SCHEMA_PRE_SERIALIZATION', 'SCHEMA_POST_SERIALIZATION',
    'SCHEMA_PRE_DESERIALIZATION', 'SCHEMA_POST_DESERIALIZATION'
  ]
  for function in tokenizer.functions:
    tokenizer.expressions.append((function, function))
  tokenizer.expressions += [
    ('LPAREN', r'\('),
    ('PUBLIC', r'public'),
    ('PRIVATE', r'private'),
    ('PROTECTED', r'protected'),
    ('RPAREN', r'\)'),
    ('LBRACKET', r'\{'),
    ('RBRACKET', r'\}'),
    ('LANGLE', r'<'),
    ('RANGLE', r'>'),
    ('COMMA', r','),
    ('COLON', ':'),
    ('SEMICOLON', ';'),
    ('EQUALS', r'='),
    ('SIMPLE_IDENTIFIER', r'\w+'),
    ('WHITESPACE', r'\s+'),
    ('NEGATION', '-'),
    ('EXPRESSION', r'[^,\(\)\{\}:\<\>-]+')
  ]

def build_parser(parser, tokenizer):
  parser.addRules(parser.START_SYMBOL, ['SchemaDeclarations'])
  parser.addRules('SchemaDeclarations', [
    'Ignored SchemaDeclaration Ignored SchemaDeclarations', 'EMPTY_STRING'
  ])
  parser.addRules('SchemaDeclaration', ['SchemaDeclarationFunction LPAREN SchemaDefinition RPAREN SEMICOLON'])
  parser.addRules('SchemaDeclarationFunction', ['DECLARE_INTERNAL_SCHEMA', 'DECLARE_EXTERNAL_SCHEMA'])
  parser.addRules('SchemaDefinition', ['Identifier WHITESPACE SchemaName WHITESPACE BaseClassList Boundary MemberDeclarationsBlock'])
  parser.addRules('SchemaName', ['TypeIdentifier'])
  parser.addRules('BaseClassList', ['COLON Boundary BaseClasses', 'EMPTY_STRING'])
  parser.addRules('BaseClasses', ['BaseClass Boundary BaseClassesEnd'])
  parser.addRules('BaseClass', ['AccessModifier WHITESPACE Identifier'])
  parser.addRules('BaseClassesEnd', ['COMMA Boundary BaseClasses', 'EMPTY_STRING'])
  parser.addRules('MemberDeclarationsBlock', ['LBRACKET Boundary MemberDeclarations RBRACKET'])
  parser.addRules('MemberDeclarations', ['MemberDeclaration MemberDeclarationsEnd', 'AccessModifier COLON MemberDeclarationsEnd', 'EMPTY_STRING'])
  parser.addRules('AccessModifier', ['PUBLIC', 'PRIVATE', 'PROTECTED'])
  parser.addRules('MemberDeclarationsEnd', ['CompoundExpressionList MemberDeclarations', 'EMPTY_STRING'])
  parser.addRules('SchemaField', [
    'SCHEMA_FIELD LPAREN Boundary TypeIdentifier WHITESPACE NameIdentifier Boundary OptionalValue Boundary RPAREN SEMICOLON'
  ])
  parser.addRules('OptionalValue', ['EQUALS Boundary ValueIdentifier', 'EMPTY_STRING'])
  parser.addRules('NameIdentifier', ['SIMPLE_IDENTIFIER'])
  parser.addRules('ValueIdentifier', ['SIMPLE_IDENTIFIER', 'NEGATION SIMPLE_IDENTIFIER'])
  parser.addRules('TypeIdentifier', ['SIMPLE_IDENTIFIER TypeIdentifierEnd'])
  parser.addRules('TypeIdentifierEnd', ['COLON COLON TypeIdentifier', 'LANGLE Boundary TemplateArgs Boundary RANGLE TypeIdentifierEnd', 'EMPTY_STRING'])
  parser.addRules('TemplateArgs', ['TypeIdentifier Boundary TemplateArgsEnd', 'EMPTY_STRING'])
  parser.addRules('TemplateArgsEnd', ['COMMA Boundary TemplateArgs', 'EMPTY_STRING'])
  parser.addRules('SchemaSerialization', ['SCHEMA_SERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaDeserialization', ['SCHEMA_DESERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaPreSerialization', ['SCHEMA_PRE_SERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaPreDeserialization', ['SCHEMA_PRE_DESERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaPostSerialization', ['SCHEMA_POST_SERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaPostDeserialization', ['SCHEMA_POST_DESERIALIZATION LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('SchemaMethods', ['SCHEMA_METHODS LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('NoSchema', ['NO_SCHEMA LPAREN MemberArguments RPAREN SEMICOLON'])
  parser.addRules('MemberDeclaration', ['SchemaField', 'SchemaSerialization', 'SchemaDeserialization', 'SchemaPreSerialization', 'SchemaPostSerialization', 'SchemaPreDeserialization', 'SchemaPostDeserialization', 'SchemaMethods', 'NoSchema'])
  parser.addRules('MemberArguments', ['MemberArgument MemberArgumentsEnd', 'EMPTY_STRING'])
  parser.addRules('MemberArgument', ['CompoundExpressionList'])
  parser.addRules('MemberArgumentsEnd', ['COMMA MemberArguments', 'EMPTY_STRING'])
  parser.addRules('CompoundExpressionList', ['CompoundExpression Boundary CompoundExpressionListEnd', 'EMPTY_STRING'])
  parser.addRules('CompoundExpressionListEnd', ['NEGATION Boundary CompoundExpressionList', 'COMMA Boundary CompoundExpressionList', 'SEMICOLON Boundary CompoundExpressionList', 'COLON Boundary CompoundExpressionList', 'LBRACKET CompoundExpressionList RBRACKET Boundary CompoundExpressionList', 'Operator Boundary CompoundExpressionList', 'SIMPLE_IDENTIFIER Boundary CompoundExpressionList', 'EMPTY_STRING'])
  parser.addRules('Operator', ['EXPRESSION', 'EQUALS'])
  parser.addRules('CompoundExpression', ['Expression CompoundExpression', 'LPAREN Boundary CompoundExpressionList Boundary RPAREN', 'LBRACKET Boundary CompoundExpressionList Boundary RBRACKET', 'NEGATION CompoundExpression', 'EMPTY_STRING'])
  parser.addRules('Expression', ['EXPRESSION', 'Identifier', 'WHITESPACE', 'SEMICOLON', 'EQUALS', 'LANGLE', 'RANGLE'])
  parser.addRules('Identifier', ['SIMPLE_IDENTIFIER IdentifierEnd'])
  parser.addRules('IdentifierEnd', ['COLON COLON Identifier', 'EMPTY_STRING'])
  parser.addRules('Boundary', ['WHITESPACE', 'EMPTY_STRING'])
  parser.addRules('Ignored', ['EMPTY_STRING'] + [t + ' IgnoredEnd' for t, _ in tokenizer.expressions if t not in tokenizer.functions])
  parser.addRules('IgnoredEnd', ['Ignored', 'EMPTY_STRING'])
  parser.addTerminals(tokenizer.expressions)
  parser.build()
