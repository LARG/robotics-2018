DECLARE_INTERNAL_SCHEMA(class A : public B, public MemoryBlock {
  public:
    SCHEMA_METHODS(A);
    A() : B(3), f1(3), f2(5) { 
      f3 = f1 == f2;
    }
    SCHEMA_FIELD(int f1);
    SCHEMA_FIELD(int f2);
    SCHEMA_FIELD(int f3);
    bool getf3() { return f3; }
    bool test_equality() { return getf3() ==f3; }
});
