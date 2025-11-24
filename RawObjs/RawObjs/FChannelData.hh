#pragma once

#include "TClonesArray.h"

class FChannel;

class FChannelData : public TClonesArray {
public:
  FChannelData();
  FChannelData(const FChannelData & data);
  ~FChannelData() override = default;

  void Clear(const Option_t * opt = "") override;

  FChannel * Add();
  FChannel * Add(unsigned short id, int ndp);
  FChannel * Add(unsigned short id, int ndp, const unsigned short * wave);
  FChannel * Add(FChannel * ch);

  int GetN() const;
  FChannel * Get(int n) const;
  FChannel * GetChannel(unsigned short id) const;
  void CopyFrom(const FChannelData * data);

private:
  int fN = 0; //! just for counter

  ClassDef(FChannelData, 1)
};

// Inline functions

inline int FChannelData::GetN() const { return GetEntriesFast(); }

inline FChannel * FChannelData::Get(int n) const { return static_cast<FChannel *>(At(n)); }
