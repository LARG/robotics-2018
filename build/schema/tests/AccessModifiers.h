DECLARE_INTERNAL_SCHEMA(class A : public MemoryBlock {
  public:
    SCHEMA_METHODS(A);
    A();
    SCHEMA_FIELD(int f1);
  public:
    SCHEMA_FIELD(int f2 = 3);
});
