
#ifndef TriggerLookupTable_hh
#define TriggerLookupTable_hh

class TriggerLookupTable {
public:
  static TriggerLookupTable & Instance()
  {
    static TriggerLookupTable instance;
    return instance;
  }

  unsigned short GetTLT(const char * val);

private:
  TriggerLookupTable() = default;
  ~TriggerLookupTable() = default;

  TriggerLookupTable(const TriggerLookupTable &) = delete;
  TriggerLookupTable & operator=(const TriggerLookupTable &) = delete;
};

#endif