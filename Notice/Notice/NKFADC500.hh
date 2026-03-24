
#pragma once

#include <cstdint>

#include "Notice/USB3Com.hh"

class NKFADC500 {
public:
  NKFADC500();
  explicit NKFADC500(int sid);
  ~NKFADC500();

  void SetSID(int sid);

  int Open();
  void Close();

  // Changed to uint32_t for hardware consistency
  uint32_t ReadBCount() const;

  // bcount: number of 256-byte blocks
  int ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout = 1000u) const;

  int Sid() const;

private:
  int _sid{0};
  USB3Com _usb;
};

inline int NKFADC500::Sid() const { return _sid; }