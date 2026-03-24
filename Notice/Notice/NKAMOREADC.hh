
#pragma once

#include <cstdint>

#include "Notice/USB3Com.hh"

class NKAMOREADC {
public:
  NKAMOREADC();
  explicit NKAMOREADC(int sid);
  ~NKAMOREADC();

  void SetSID(int sid);

  int Open();
  void Close();

  // Changed to uint32_t for consistency with hardware registers
  uint32_t ReadBCount() const;

  // bcount: Block count, returns 0 on success or negative error code
  int ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout = 1000u) const;

  int Sid() const;

private:
  // Returns true if successful, false on timeout
  bool WaitFPGAAndInit();

  int _sid{0};
  USB3Com _usb;
};

inline int NKAMOREADC::Sid() const { return _sid; }