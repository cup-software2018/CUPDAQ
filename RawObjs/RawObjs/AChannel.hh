#pragma once

#include "RawObjs/AbsChannel.hh"

class AChannel : public AbsChannel {
public:
  AChannel();
  explicit AChannel(unsigned short id);
  AChannel(const AChannel & ch);

  ~AChannel() override = default;

  void SetADC(unsigned int count);
  void SetTime(unsigned int time);

  unsigned int GetADC() const;
  unsigned int GetTime() const;

protected:
  unsigned int fADC = 0;
  unsigned int fTime = 0;

  ClassDef(AChannel, 1)
};

// Inline functions

inline void AChannel::SetADC(unsigned int count) { fADC = count; }
inline void AChannel::SetTime(unsigned int time) { fTime = time; }
inline unsigned int AChannel::GetADC() const { return fADC; }
inline unsigned int AChannel::GetTime() const { return fTime; }
