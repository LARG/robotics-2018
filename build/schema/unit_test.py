import unittest
import schema_parser, type_parser, generator
import os

class SchemaParsing(unittest.TestCase):

  def test_access_modifiers(self):
    forest = schema_parser.parse('AccessModifiers.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()

    self.assertEqual(len(fields), 2)

    self.assertEqual(fields[0].args[0], 'int')
    self.assertEqual(fields[0].args[1], 'f1')
    
    self.assertEqual(fields[1].args[0], 'int')
    self.assertEqual(fields[1].args[1], 'f2')
    self.assertEqual(fields[1].args[2], '3')
  
  def test_constructor_initializers(self):
    forest = schema_parser.parse('ConstructorInitializers.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()

    self.assertEqual(len(fields), 2)

    self.assertEqual(fields[0].args[0], 'int')
    self.assertEqual(fields[0].args[1], 'f1')
    
    self.assertEqual(fields[1].args[0], 'int')
    self.assertEqual(fields[1].args[1], 'f2')
  
  def test_multiple_assignment(self):
    forest = schema_parser.parse('MultipleAssignment.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()

    self.assertEqual(len(fields), 4)
    
    for i, field in enumerate(fields):
      self.assertEqual(field.args[0], 'int')
      self.assertEqual(field.args[1], 'f%i' % (i + 1))
  
  def test_equality(self):
    forest = schema_parser.parse('Equality.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()

    self.assertEqual(len(fields), 3)
    
    for i, field in enumerate(fields):
      self.assertEqual(field.args[0], 'int')
      self.assertEqual(field.args[1], 'f%i' % (i + 1))

  def test_type_mapping(self):
    forest = schema_parser.parse('TypeMapping.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)
    
    tree = trees[0]
    fields = tree.get_fields()
    self.assertEqual(len(fields), 14)

  def test_cast_functions(self):
    forest = schema_parser.parse('CastFunctions.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()
    self.assertEqual(len(fields), 1)

    field = fields[0]

    self.assertEqual(field.args[0], "std::array<int8_t,10>")
  
  def test_externals(self):
    forest = schema_parser.parse('Externals.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

    tree = trees[0]
    fields = tree.get_fields()
    self.assertEqual(len(fields), 3)
    
    self.assertEqual(fields[0].args[0], 'float')
    self.assertEqual(fields[0].args[1], 'x')
    
    self.assertEqual(fields[1].args[0], 'float')
    self.assertEqual(fields[1].args[1], 'y')
    
    self.assertEqual(fields[2].args[0], 'float')
    self.assertEqual(fields[2].args[1], 't')
  
  def test_compound_containers(self):
    forest = schema_parser.parse('CompoundContainers.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)

class Generating(unittest.TestCase):

  def test_type_mapping(self):
    forest = schema_parser.parse('TypeMapping.h')
    trees = forest.get_trees()
    self.assertEqual(len(trees), 1)
    
    tree = trees[0]
    fields = tree.get_fields()
    self.assertEqual(len(fields), 14)

    schema_types = [generator.Type(f.args[0]) for f in fields]
    for st in schema_types:
      self.assertTrue(st.is_primitive())
      self.assertTrue(st.is_numeric())

class TypeParsing(unittest.TestCase):

  def test_compound_containers(self):
    t = "std::array<Eigen::Matrix<float, STATE_SIZE, 1, Eigen::DontAlign>, MAX_MODELS_IN_MEM>"
    tree = type_parser.parse(t)
    self.assertNotEqual(tree, False)

if __name__ == '__main__':
  testpath = os.path.expandvars('$NAO_HOME/build/schema/tests')
  os.chdir(testpath)
  unittest.main()
