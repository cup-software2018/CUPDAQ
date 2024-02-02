#ifndef SADCHistogramer_hh
#define SADCHistogramer_hh

#include "OnlHistogramer/AbsHistogramer.hh"

class SADCHistogramer : public AbsHistogramer {
public:
  SADCHistogramer();
  virtual ~SADCHistogramer();

  virtual void Book();
  virtual void Fill(BuiltEvent * bevent);
};

#endif