#pragma once

#include "TObject.h"

class AbsChannel : public TObject {
public:
  AbsChannel();
  explicit AbsChannel(unsigned short id);
  AbsChannel(const AbsChannel & ch);

  ~AbsChannel() override = default;

  void SetID(unsigned short id);
  void SetBit(unsigned short bit);

  unsigned short GetID() const;
  unsigned short GetBit() const;

protected:
  unsigned short fID = 0;
  unsigned short fBit = 0;

  ClassDef(AbsChannel, 1)
};

// Inline functions

inline void AbsChannel::SetID(unsigned short id) { fID = id; }
inline void AbsChannel::SetBit(unsigned short bit) { fBit = bit; }
inline unsigned short AbsChannel::GetID() const { return fID; }
inline unsigned short AbsChannel::GetBit() const { return fBit; }
