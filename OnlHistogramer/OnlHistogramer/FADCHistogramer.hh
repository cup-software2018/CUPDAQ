#ifndef FADCHistogramer_hh
#define FADCHistogramer_hh

#include "OnlHistogramer/AbsHistogramer.hh"

class FADCHistogramer : public AbsHistogramer {
public:
  FADCHistogramer();
  virtual ~FADCHistogramer();

  virtual void Book();
  virtual void Fill(BuiltEvent * bevent);

private:
  int fNDP;
};

#endif