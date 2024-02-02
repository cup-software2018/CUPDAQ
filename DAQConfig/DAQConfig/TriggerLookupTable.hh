#ifndef TriggerLookupTable_hh
#define TriggerLookupTable_hh

#include "TNamed.h"
#include "TObject.h"

class TriggerLookupTable : public TObject {
public:
  TriggerLookupTable();
  ~TriggerLookupTable();

  UShort_t GetTLT(const char * val);

  ClassDef(TriggerLookupTable, 0)
};
#endif
