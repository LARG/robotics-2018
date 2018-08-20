#!/usr/bin/env python

import sys, os, re
import schema_parser
import type_parser
import shutil
from functools import partial
from command_cache import CommandCache

SCHEMA_TEMPLATE = \
"""
table <schema_type> {
  <fields>
}

root_type <schema_type>;
"""
class ConsoleColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

VERBOSE = False
def console(message):
  if not VERBOSE: return
  print message

class TypeDB(object):
  def __init__(self):
    self.types = {}

  def write(self, path):
    with open(path, 'w') as f:
      for type in self.types.values():
        f.write("%s,%s\n" % (type.t, "external" if type.external else "internal"))

  def read(self, path):
    with open(path, 'r') as f:
      for line in f:
        t, designation = line.strip().split(',')
        self.types[t] = Type(t, external=(designation == "external"), from_database=True)

  def init(self, types):
    for t in types: 
      self.types[t.t] = t

  def __getitem__(self, key):
      return self.types.__getitem__(key)
  
  def __setitem__(self, key, value):
      self.types.__setitem__(key, value)

  def __contains__(self, key):
    return self.types.__contains__(key)

  def __iter__(self):
    return self.types.__iter__()

  def __eq__(self, other):
    return self.types == other.types

  def __ne__(self, other):
    return self.types != other.types

class Type(object):
  db = TypeDB()
  def __init__(self, t, external=False, from_database=False):
    self.t = t
    self.from_database = from_database
    self.unqualified = re.sub(r'^.*::(?=[^:])', '', t)
    self.variable_name = self.path = t.replace("::","_") \
      .replace("<","{").replace(">","}")
    self.schema_type = t.replace("::",".") \
      .replace("<","_").replace(">","_").replace(",","_").replace(" ","")
    self.enum = False
    if self.schema_type in API.MAPPING:
      self.schema_type = API.MAPPING[self.schema_type]
    elif not self.from_database:
      self.schema_type = "int"
      self.enum = True
    self.parsed = type_parser.parse(self.t)
    self.outer_type = self.parsed.name
    self.subtypes = [Type.load(st.name) for st in self.parsed.subtypes]
    self.container = self.outer_type in API.CONTAINERS
    self.external = external
    self.vector = self.outer_type == "std::vector"
    self.namespace = re.sub(r'::[^:]*$', '', self.outer_type)
    if '::' in self.outer_type:
      self.qualifier = re.sub(r'(?<=::)[^:]*$', '', self.outer_type)
    else:
      self.qualifier = ''
    self.qualified = t
    
    if self.external:
      self.include = "memory/schema/external/%s.h" % self.path
    else:
      self.include = None

  def get_schema_type(self):
    if self.unqualified == "string": return "string"
    if self.is_container():
      return "[%s]" % self.subtypes[0].get_schema_type()
    return self.schema_type

  def get_unqualified_schema_type(self):
    st = self.get_schema_type()
    return re.sub(r'^.*\.', '', st)

  def get_generated_header_path(self, sourcepath):
    return os.path.join(sourcepath, self.path + ".h")

  def is_from_database(self):
    return self.from_database
  
  def is_string(self):
    return self.get_schema_type() == "string"

  def is_primitive(self):
    return self.get_schema_type() in API.PRIMITIVES

  def is_numeric(self):
    return self.get_schema_type() in API.NUMERICS
  
  def is_composite(self):
    return not self.is_primitive()

  def is_allocated(self):
    return self.is_string() or self.is_composite() or self.is_container()

  def is_external(self):
    return self.external

  def is_internal(self):
    return self.from_database and not self.external

  def is_primitive_container(self):
    return self.is_container() and self.subtypes[0].is_primitive()

  def is_container(self):
    return self.container

  def is_vector(self):
    return self.vector

  def get_allocation_pointer_type(self):
    if self.is_container():
      assert len(self.subtypes) > 0
      return "flatbuffers::Offset<flatbuffers::Vector<%s>>" % self.subtypes[0].get_allocation_pointer_type()
    else:
      return "flatbuffers::Offset<schema::%s>" % self.t

  def get_allocation_name(self, name):
    return "<name>_<varname>_alloc".replace("<name>",name).replace("<varname>",self.variable_name)
  
  def allocate_composite_container(self, name, source, depth):
        # 1. create a std::vector of the allocated subtype pointer (don't allocate)
          # 1.1 you will need to recursively determine the subtype's allocated pointer type
        # 2. for each item (of type subtype):
          # 2.1 allocate the subtype with subtype.allocate_field_value. this may recurse down.
          # 2.2 push the allocated item onto the vector you created
        # 3. allocate the vector of subtype pointers using CreateVector
    item_name = name + "%02i_item" % depth
    vec_name = name + "%02i_vec" % depth
    subvar_name = name + "%02i_alloc" % (depth + 1)
    if depth > 0:
      var_name = name + "%02i_alloc" % depth
    else:
      var_name = name + "_alloc"
    return """
  auto <vec_name> = std::vector<<aptype>>();
  for(const auto& <item_name> : <source>) {
    <suballoc>
    <vec_name>.push_back(<subvar_name>);
  }
  auto <var_name> = __serializer__->CreateVector(<vec_name>);
""" \
      .replace("<vec_name>", vec_name) \
      .replace("<item_name>", item_name) \
      .replace("<aptype>", self.subtypes[0].get_allocation_pointer_type()) \
      .replace("<source>", source) \
      .replace("<suballoc>", self.subtypes[0].allocate_field_value(name=name, source=item_name, depth=depth+1)) \
      .replace("<subvar_name>", subvar_name) \
      .replace("<var_name>", var_name)

  def allocate_field_value(self, name=None, source=None, depth=0):
    assert name != None
    if source == None: source = "__source_object__." + name
    alloc_name = "<name><depth>_alloc".replace("<name>",name).replace("<depth>","%02i"%depth if depth > 0 else "")
    if self.is_string():
      return "auto <alloc_name> = __serializer__->CreateString(<source>);" \
        .replace("<alloc_name>", alloc_name) \
        .replace("<source>", source)
    elif self.is_vector() and self.is_primitive_container():
      return "auto <alloc_name> = __serializer__->CreateVector(<source>);" \
        .replace("<alloc_name>", alloc_name) \
        .replace("<source>", source)
    elif self.is_container():
      if self.is_primitive_container():
        return "auto <alloc_name> = __serializer__->CreateVector(<source>.data(), <source>.size());" \
          .replace("<alloc_name>", alloc_name) \
          .replace("<source>", source)
      else:
        return self.allocate_composite_container(name=name, source=source, depth=depth)
    elif self.is_external():
      return "auto <alloc_name> = schema::<qualifier>serialize(__serializer__, <source>);" \
        .replace("<qualifier>",self.qualifier) \
        .replace("<alloc_name>",alloc_name) \
        .replace("<source>", source)
    else:
      return "auto <alloc_name> = <source>.serialize(__serializer__);" \
        .replace("<type>",self.t) \
        .replace("<alloc_name>",alloc_name) \
        .replace("<source>", source) \
        .replace("<name>", name)

  def __str__(self):
    return self.t

  def __eq__(self, other):
    return self.t == other.t

  def __ne__(self, other):
    return self.t != other.t

  @staticmethod
  def write_db(path, types):
    Type.db.init(types)
    try:
      db = TypeDB()
      db.read(path)
      if db == Type.db:
        console("New database matches the current one, no need to rewrite.")
        return
    except IOError: pass
    
    console("Writing %i types to the database" % len(types))
    Type.db.write(path)

  @staticmethod
  def read_db(path):
    try:
      Type.db.read(path)
    except IOError: pass

  @staticmethod
  def load(name):
    return Type.db[name] if name in Type.db else Type(name)

class API(object):
  CONTAINERS = set(["std::array","std::list","std::vector","std::set"])
  MAPPING = {
    "int8_t":"byte", "uint8_t":"ubyte", 
    "int16_t":"short", "uint16_t":"ushort", 
    "int32_t":"int", "uint32_t":"uint", 
    "int64_t":"long", "uint64_t":"ulong", 
    "bool":"bool","char":"byte", "short":"short", "int":"int",
    "float":"float","double":"double"
  }
  NUMERICS = set(MAPPING.values())
  PRIMITIVES = NUMERICS.union("string")

class Schema(object):
  def __init__(self, tree):
    name = tree.get_schema_name()
    self.tree = tree
    assert(type(self) != Schema)
    self.type = Type.load(name)
    self.name = self.type.unqualified
    self.namespaces = ["schema"] + [ns for ns in name.split("::")][:-1]
    self.write_field_values = ''
    self.read_field_values = ''
    self.pre_serialization = ''
    self.post_serialization = ''
    self.pre_deserialization = ''
    self.post_deserialization = ''
    self.fields = []

  def schema(self):
    fields = []
    for field in self.fields:
      if field.value:
        fields.append("<name>:<type> = <value>;"
          .replace("<type>",field.type.get_schema_type())
          .replace("<name>",field.name)
          .replace("<value>",field.value)
        )
      else:
        fields.append("<name>:<type>;"
          .replace("<type>",field.type.get_schema_type())
          .replace("<name>",field.name)
        )
    field_str = "\n  ".join(fields)
    schema = SCHEMA_TEMPLATE \
      .replace("<schema_type>", self.type.get_unqualified_schema_type()) \
      .replace("<fields>", field_str)
    return schema

  def recurse_types(self, t=None):
    if t is None:
      types = [f.type for f in self.fields]
      for t in types:
        for st in self.recurse_types(t):
          yield st
    else:
      yield t
      for st in t.subtypes:
        for sst in self.recurse_types(st):
          yield sst

  def dependencies(self):
    paths = [t.path for t in set(self.recurse_types()) if t.is_from_database()]
    return paths

class InternalSchema(Schema):
  def __init__(self, *args, **kwargs):
    super(InternalSchema, self).__init__(*args, **kwargs)
    self.type.external = False

class ExternalSchema(Schema):
  def __init__(self, *args, **kwargs):
    super(ExternalSchema, self).__init__(*args, **kwargs)
    self.type.external = True

class Field(object):
  def __init__(self, t, name, value=None):
    self.name = name
    self.type = Type.load(t)
    self.value = value
  
  def allocate_field_value(self):
    return self.type.allocate_field_value(name=self.name)

  def is_string(self):
    return self.type.is_string()

  def is_primitive(self):
    return self.type.is_primitive()

  def is_composite(self):
    return not self.type.is_primitive()

  def is_allocated(self):
    return self.is_string() or self.is_composite() or self.is_container()

  def is_external(self):
    return self.type.external

  def is_primitive_container(self):
    return self.is_container() and self.subtypes[0].is_primitive()

  def is_container(self):
    return self.type.container

  def is_numeric(self):
    return self.type.is_numeric()

  def is_vector(self):
    return self.type.vector

  def is_enum(self):
    return self.type.enum

  def is_internal(self):
    return self.type.is_internal()

  def declare(self):
    if self.is_primitive(): return None
    if not self.type.external: return None
    serialize = "void schema::<qualifier>serialize(serialization::serializer&, <type>&);"
    deserialize = "void schema::<qualifier>deserialize(serialization::data_pointer, <type>&);"
    declarations = [serialize, deserialize]
    declarations = "\n".join([
      d \
        .replace("<type>",self.type.t) \
        .replace("<qualifier>",self.type.qualifier)
      for d in declarations
    ])
    return declarations

  def write_value(self):
    if self.is_allocated():
      return "__builder__.add_<name>(<name>_alloc);" \
        .replace("<name>", self.name)
    else:
      assert(self.is_primitive())
      return "__builder__.add_<name>(__source_object__.<name>);" \
        .replace("<name>", self.name)

  def read_value(self):
    if self.is_primitive():
      if self.is_string():
        return "__target_object__.<name> = data-><name>()->str();".replace("<name>", self.name)
      elif self.is_enum():
        return "__target_object__.<name> = static_cast<<type>>(data-><name>());" \
          .replace("<name>", self.name) \
          .replace("<type>", self.type.t)
      else:
        return "__target_object__.<name> = data-><name>();".replace("<name>", self.name)
    console("reading value with name:" + self.name + ", ns: " + self.type.namespace)

    # Numerics are non-string primitives
    if self.is_container() and self.type.subtypes[0].is_external():
      return "schema::<qualifier>deserialize_composite_external(data-><name>(), __target_object__.<name>);" \
        .replace("<qualifier>",self.type.qualifier) \
        .replace("<name>",self.name)
    if self.is_container() and self.type.subtypes[0].is_internal():
      return "schema::<qualifier>deserialize_composite_internal(data-><name>(), __target_object__.<name>);" \
        .replace("<qualifier>",self.type.qualifier) \
        .replace("<name>",self.name)
    if self.is_internal():
      return "__target_object__.<name>.deserialize(data-><name>());" \
        .replace("<name>",self.name)
    if self.is_container() and self.type.subtypes[0].is_numeric():
      return "schema::<qualifier>deserialize(data-><name>(), __target_object__.<name>);" \
        .replace("<qualifier>",self.type.qualifier) \
        .replace("<name>",self.name)
    if self.is_external():
      return "schema::<qualifier>deserialize(data-><name>(), __target_object__.<name>);" \
        .replace("<qualifier>",self.type.qualifier) \
        .replace("<name>",self.name)
    error_message = "Error generating deserialization method call"
    print error_message
    print self.type
    raise Exception(error_message)

class Generator(object):
  def __init__(self, paths, schemapath=None, sourcepath=None, action=None, includepath=''):
    self.paths = paths
    self.build_db = action == 'build_db'
    self.verify = action == 'schema'
    self.includepath = includepath
    self.schemapath = schemapath
    self.sourcepath = sourcepath
    self.schemas = []

  def dependencies(self):
    dependencies = []
    for s in self.schemas:
      for d in s.dependencies():
        dependencies.append(d)
    return dependencies
    
  def parse(self):
    self.result = self._parse()

  def _parse(self):
    found = False
    schema = None
    for path in self.paths:
      self.forest = schema_parser.parse(path)
      trees = self.forest.get_trees() if self.forest else None
      
      # If we're not just checking for validity then go to the next path
      if not trees:
        if self.verify:
          raise Exception("No valid schemas found in header: '%s'" % path)
        else: continue
      console("%i trees identified" % len(trees))
      
      for t in trees:
        if t.is_external():
          schema = ExternalSchema(t)
        else:
          schema = InternalSchema(t)
        self.schemas.append(schema)
        for f in t.get_fields():
          f = Field(*f.args)
          schema.fields.append(f)
        for s in t.get_custom_serialization():
          schema.write_field_values = s.args[0][1:-1] # Remove brackets to avoid scoping
          break
        for d in t.get_custom_deserialization():
          schema.read_field_values = d.args[0]
        for d in t.get_pre_serialization():
          schema.pre_serialization = d.args[0]
        for d in t.get_post_serialization():
          schema.post_serialization = d.args[0]
        for d in t.get_pre_deserialization():
          schema.pre_deserialization = d.args[0]
        for d in t.get_post_deserialization():
          schema.post_deserialization = d.args[0]
    return True

  def check(self):
    assert len(self.paths) > 0
    return schema_parser.check(self.paths[0])

  def generate_schema(self, schema):
    lines = []
    includes = ['include "%s.fbs";' % i for i in schema.dependencies()]
    namespace = "namespace <namespaces>;" \
      .replace("<namespaces>", ".".join(schema.namespaces))
    lines += includes
    lines.append(namespace)
    lines.append(schema.schema())
    return "\n".join(lines).strip()

  def generate_header_external(self, schema):
    tabs = ""
    headers = [
      "#pragma once",
      "#include <common/Serialization.h>",
      "#include <%s>" % schema.type.include,
    ]
    header = "\n".join(headers) + "\n\n"
    
    for ns in schema.namespaces:
      header += "<tabs>namespace <ns> {\n" \
      .replace("<tabs>",tabs) \
      .replace("<ns>",ns)
      tabs += "  "

    struct = "struct %s;\n" % schema.type.get_unqualified_schema_type()
    serialize = "flatbuffers::Offset<<qualifier><schema_type>> serialize(serialization::serializer& __serializer__, const ::<type>& __source_object__);\n"
    deserialize_from_buffer = "bool deserialize(serialization::data_pointer __data_pointer_, ::<type>& __target_object__);\n"
    deserialize_from_instance = "bool deserialize(const schema::<qualifier><schema_type>* data, ::<type>& __target_object__);\n"

    for d in [struct, serialize, deserialize_from_buffer, deserialize_from_instance]:
      header += tabs + d \
        .replace("<schema_type>",schema.type.get_unqualified_schema_type()) \
        .replace("<qualifier>",schema.type.qualifier) \
        .replace("<type>",schema.type.t)

    while len(tabs) > 0:
      tabs = tabs[2:]
      header += "%s}\n" % tabs

    return header

  def generate_source_external(self, schema, includes=[]):
    template = """
<include>
flatbuffers::Offset<schema::<qualifier><schema_type>> schema::<qualifier>serialize(serialization::serializer& __serializer__, const ::<type>& __source_object__) {
  <pre_serialization>
  <allocate_field_values>
  <write_field_values>
  <post_serialization>
  <serialize_return_statement>
}

bool schema::<qualifier>deserialize(serialization::data_pointer __data_pointer__, ::<type>& __target_object__) {
  auto data = schema::<qualifier>Get<schema_type>(__data_pointer__);
  return deserialize(data, __target_object__);
}

bool schema::<qualifier>deserialize(const schema::<qualifier><schema_type>* data, ::<type>& __target_object__) {
  <pre_deserialization>
  <read_field_values>
  <post_deserialization>
  return true;
}
    """
    return self.generate_source(schema, template, includes=includes)

  def generate_source_internal(self, schema, includes=[]):
    template = """
<include>
flatbuffers::Offset<schema::<qualifier><schema_type>> <qualifier><schema_type>::serialize(serialization::serializer& __serializer__) const {
  auto& __source_object__ = *this;
  assert(__serializer__ != nullptr);
  <pre_serialization>
  <allocate_field_values>
  <write_field_values>
  <post_serialization>
  <serialize_return_statement>
}

bool <qualifier><schema_type>::deserialize(const serialization::serializer& __serializer__) {
  return deserialize(__serializer__->GetBufferPointer());
}

bool <qualifier><schema_type>::deserialize(serialization::data_pointer __data_pointer__) {
  auto data = schema::<qualifier>Get<schema_type>(__data_pointer__);
  return deserialize(data);
}

bool <qualifier><schema_type>::deserialize(const schema::<qualifier><schema_type>* data) {
  auto& __target_object__ = *this;
  <pre_deserialization>
  <read_field_values>
  <post_deserialization>
  return true;
}
    """
    return self.generate_source(schema, template, includes=includes)

  def generate_source(self, schema, template, includes=[]):
    if schema.write_field_values:
      write_field_values_str = schema.write_field_values
      allocate_field_values_str = ""
      serialize_return_statement_str = ""
    else:
      write_field_values_str = "  schema::<qualifier><schema_type>Builder __builder__(*__serializer__);\n  " \
        .replace("<qualifier>",schema.type.qualifier).replace("<schema_type>",schema.type.get_unqualified_schema_type())
      write_field_values_str += "\n  ".join(f.write_value() for f in schema.fields)
      allocate_field_values_str = "\n  ".join(f.allocate_field_value() for f in schema.fields if f.is_allocated())
      serialize_return_statement_str = "  return __builder__.Finish();"

    if schema.read_field_values:
      read_field_values_str = schema.read_field_values
    else:
      read_field_values_str = "\n  ".join(f.read_value() for f in schema.fields)

    pre_serialization_str = schema.pre_serialization
    pre_deserialization_str = schema.pre_deserialization
    post_serialization_str = schema.post_serialization
    post_deserialization_str = schema.post_deserialization

    # If no includes are specified, at least include this schema's declaration
    if len(includes) == 0:
      includes = [os.path.join(self.includepath,schema.type.path + ".h")]

    includes = includes + [
      "%s/%s_generated.h" % (self.schemapath,schema.type.path),
      "common/SchemaExtensions.h"
    ]
    for t in set(schema.recurse_types()):
      if t.include:
        includes += [t.include]
      if t.external:
        generated_header = t.get_generated_header_path(self.sourcepath)
        includes += [generated_header]
    include_str = "\n".join(["#include <%s>" % i for i in includes])

    source = template \
      .replace("<name>",schema.name) \
      .replace("<qname>",schema.type.qualified) \
      .replace("<type>",str(schema.type)) \
      .replace("<schema_type>",schema.type.get_unqualified_schema_type()) \
      .replace("<namespace>",schema.type.namespace) \
      .replace("<qualifier>",schema.type.qualifier) \
      .replace("<include>",include_str) \
      .replace("<write_field_values>",write_field_values_str) \
      .replace("<read_field_values>",read_field_values_str) \
      .replace("<pre_serialization>", pre_serialization_str) \
      .replace("<pre_deserialization>", pre_deserialization_str) \
      .replace("<post_serialization>", post_serialization_str) \
      .replace("<post_deserialization>", post_deserialization_str) \
      .replace("<allocate_field_values>",allocate_field_values_str) \
      .replace("<serialize_return_statement>", serialize_return_statement_str)

    return source.strip()

  def write_content(self, content, path):
    #print ConsoleColors.BOLD + ConsoleColors.OKBLUE + "Generating " + path + ConsoleColors.ENDC
    with open(path, 'w') as f:
      f.write(content)

  def write_schema(self, schema, path):
    path = os.path.join(path, schema.type.path + ".fbs")
    schema = self.generate_schema(schema)
    self.write_content(schema, path)

  def write_swig(self, outdir):
    for path in self.paths:
      forest = schema_parser.parse(path)
      if forest and forest.get_trees():
        forest.remove_directives()
        swig = forest.getValue()
        outpath = os.path.join(outdir, os.path.basename(path))
        self.write_content(swig, outpath)
      else:
        with open(path, 'r') as f:
          header = f.read()
          header = re.sub(r'NO_SCHEMA\([A-Za-z]+\);', '', header)
        with open(os.path.join(outdir, os.path.basename(path)), 'w') as f:
          f.write(header)

  def write_source(self, schema, path):
    if schema.type.external:
      header = self.generate_header_external(schema)
      header_path = os.path.join(path, schema.type.path + ".h")
      self.write_content(header, header_path)
      generated_header = schema.type.get_generated_header_path(self.sourcepath)
      source = self.generate_source_external(schema, includes=[generated_header])
    else:
      source = self.generate_source_internal(schema)
    source_path = os.path.join(path, schema.type.path + ".cpp")
    self.write_content(source, source_path)

import argparse
def parse_args():
  parser = argparse.ArgumentParser(description='Schema Generator')
  parser.add_argument('--action', dest='action', action='store', type=str, 
                     choices=['check', 'build_db', 'schema', 'swig', 'dependencies'], required=True,
                     help='The action to be performed', default='schema')
  parser.add_argument('--header', action='append', dest='headers', type=str,
                     help='Path to the input header files', default=None)
  parser.add_argument('--include', dest='include', action='store', type=str,
                     help='Include path for the provided header', default='')
  parser.add_argument('--schema', dest='schema', action='store', type=str,
                     help='Path to the generated schema output directory', default=None)
  parser.add_argument('--source', dest='source', action='store', type=str,
                     help='Path to the generated source output directory', default=None)
  parser.add_argument('--swig', dest='swig', action='store', type=str,
                     help='Path to the generated swig output directory', default=None)
  parser.add_argument('--dbpath', dest='dbpath', action='store', type=str,
                     help='Build a dbpath of internal and external schemas', default=None)
  parser.add_argument('--verbose', dest='verbose', action='store_true',
                     help='Output debugging information to the console', default=False)
  args, unk = parser.parse_known_args()
  global VERBOSE
  VERBOSE = args.verbose
  return args

def main(args):
  console("starting generator with arguments:")
  console(args)
  if not args.dbpath and args.action in ['check', 'build_db', 'schema']:
    print "Type database path needs to be specified"
    sys.exit(1)
  if args.action not in ['swig', 'build_db']:
    console("reading database from '%s'" % args.dbpath)
    Type.read_db(args.dbpath)
  console("instantiating generator")
  if args.action == 'swig':
    generator = Generator(paths=args.headers, action=args.action)
    generator.write_swig(args.swig)
    return
  elif args.action == 'check':
    if not args.schema:
      print "Schema generation directory required for check."
      sys.exit(1)
    generator = Generator(paths=args.headers, action=args.action, includepath=args.include, schemapath=args.schema, sourcepath=args.source)
    if not generator.check():
      return "NO_SCHEMA"
    generator.parse()
    results = ["SCHEMA_FOUND"] + generator.dependencies()
    return ";".join(results)
  generator = Generator(paths=args.headers, action=args.action, includepath=args.include, schemapath=args.schema, sourcepath=args.source)
  generator.parse()
  if args.action == 'build_db':
    Type.write_db(args.dbpath, [s.type for s in generator.schemas])
  elif args.action == 'dependencies':
    d = generator.dependencies()
    if len(d) > 0: return ";".join(d)
  elif args.action == 'schema':
    console("writing %i schemas" % len(generator.schemas))
    for schema in generator.schemas:
      generator.write_schema(schema, args.schema)
      generator.write_source(schema, args.source)
      if args.swig:
        generator.write_swig(args.swig)

if __name__ == '__main__':
  args = parse_args()
  try:
    if args.action in ['check','build_db'] and args.schema: 
      cache = CommandCache(args.schema, args.headers)
      callback = partial(main, args)
      print cache.cached_read(callback)
    else:
      main(args)
  except:
    print args
    raise
