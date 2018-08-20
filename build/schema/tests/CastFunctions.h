DECLARE_INTERNAL_SCHEMA(class A : public MemoryBlock {
  public:
    SCHEMA_METHODS(A);
    A();
    SCHEMA_FIELD(std::array<int8_t,10> f1);
    std::string convertToString() {
      return std::string(static_cast<const char*>(f1.data())).substr(0, 15);
    }
});
