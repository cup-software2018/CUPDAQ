#ifndef TriggerLookupTable_hh
#define TriggerLookupTable_hh

#include "TObject.h"

class TriggerLookupTable : public TObject {
public:
  TriggerLookupTable();
  ~TriggerLookupTable() override = default;

  UShort_t GetTLT(const char * val);

  ClassDef(TriggerLookupTable, 0)
};

#endif
