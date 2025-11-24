#pragma once

#include "TClonesArray.h"

class AChannel;

class AChannelData : public TClonesArray {
public:
  AChannelData();
  AChannelData(const AChannelData & data);
  ~AChannelData() override = default;

  void Clear(const Option_t * opt = "") override;

  AChannel * Add();
  AChannel * Add(unsigned short id);
  AChannel * Add(AChannel * ch);

  int GetN() const;
  AChannel * Get(int n) const;
  AChannel * GetChannel(unsigned short id) const;
  void CopyFrom(const AChannelData * data);

private:
  int fN = 0; //! just for counter

  ClassDef(AChannelData, 1)
};

// Inline functions

inline int AChannelData::GetN() const { return GetEntriesFast(); }

inline AChannel * AChannelData::Get(int n) const { return static_cast<AChannel *>(At(n)); }
