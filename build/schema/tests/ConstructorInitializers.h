DECLARE_INTERNAL_SCHEMA(class A : public B, public MemoryBlock {
  public:
    SCHEMA_METHODS(A);
    A() : B(3), f1(3), f2(5) { }
    SCHEMA_FIELD(int f1);
    SCHEMA_FIELD(int f2);
});
