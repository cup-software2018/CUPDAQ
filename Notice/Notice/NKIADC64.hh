#pragma once

#include <cstdint>

#include "Notice/USB3Com.hh"

class NKIADC64 {
public:
  NKIADC64();
  explicit NKIADC64(int sid);
  ~NKIADC64();

  void SetSID(int sid);

  int Open();
  void Close();

  // Changed to uint32_t for consistency
  uint32_t ReadBCount() const;

  // bcount: number of 256-byte blocks
  int ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout = 0) const;

  void FlushData() const;

  int Sid() const;

private:
  int _sid{0};
  USB3Com _usb{};
};

inline int NKIADC64::Sid() const { return _sid; }