#include "TFormula.h"
#include "TString.h"

#include "DAQConfig/TriggerLookupTable.hh"

ClassImp(TriggerLookupTable)

TriggerLookupTable::TriggerLookupTable()
    : TObject()
{
}

TriggerLookupTable::~TriggerLookupTable() {}

UShort_t TriggerLookupTable::GetTLT(const char * val)
{
  TString a(val);
  a.ReplaceAll(" ", "");
  a.ToLower();

  a.ReplaceAll("+", 1, "|", 1);
  a.ReplaceAll("*", "x");

  int n = 0;
  bool isAndBefore = false;
  while (true) {
    if (a.Length() == n) break;
    if (a[n] == 'x') {
      a.Replace(n, 1, "&");
      if (isAndBefore) {
        a.Remove(n - 1, 1);
        a.Insert(n + 1, ")");
      }
      else {
        a.Insert(n - 1, "(");
        a.Insert(n + 3, ")");
      }
      isAndBefore = true;
      n = 0;
    }
    else if (a[n] == '|') {
      isAndBefore = false;
    }
    n += 1;
  }

  for (int i = 0; i < 4; i++) {
    a.ReplaceAll(TString(Form("%d", i + 1)).Data(), 1,
                 TString(Form("int([%d])", i)).Data(), 8);
  }

  a = "(" + a + ")";

  for (int i = 0; i < 4; i++) {
    if (!a.Contains(Form("%d", i))) { a += Form(" + 0*[%d]", i); }
  }

  auto * tlt = new TFormula("TLT", a.Data());

  unsigned int tch1, tch2, tch3, tch4, shift;
  unsigned long tltf, triggerf;

  tltf = 0;
  for (tch4 = 0; tch4 <= 1; tch4++) {
    for (tch3 = 0; tch3 <= 1; tch3++) {
      for (tch2 = 0; tch2 <= 1; tch2++) {
        for (tch1 = 0; tch1 <= 1; tch1++) {
          tlt->SetParameter(0, tch1);
          tlt->SetParameter(1, tch2);
          tlt->SetParameter(2, tch3);
          tlt->SetParameter(3, tch4);

          triggerf = (unsigned long)tlt->Eval(0);

          shift = (tch4 << 3) | (tch3 << 2) | (tch2 << 1) | tch1;
          tltf = tltf + (triggerf << shift);
        }
      }
    }
  }

  tltf = tltf & 0xFFFF;

  delete tlt;

  return tltf;
}
