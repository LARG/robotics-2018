DECLARE_INTERNAL_SCHEMA(class A : public B, public MemoryBlock {
  public:
    SCHEMA_METHODS(A);
    A();
    SCHEMA_FIELD(int8_t f1);
    SCHEMA_FIELD(uint8_t f2);
    SCHEMA_FIELD(int16_t f3);
    SCHEMA_FIELD(uint16_t f4);
    SCHEMA_FIELD(int32_t f5);
    SCHEMA_FIELD(uint32_t f6);
    SCHEMA_FIELD(int64_t f7);
    SCHEMA_FIELD(uint64_t f8);
    SCHEMA_FIELD(bool f9);
    SCHEMA_FIELD(char f10);
    SCHEMA_FIELD(short f11);
    SCHEMA_FIELD(int f12);
    SCHEMA_FIELD(float f13);
    SCHEMA_FIELD(double f14);
});
