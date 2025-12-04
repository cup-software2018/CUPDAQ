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

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;

  int Sid() const;

private:
  void WaitFPGAAndInit();

  int _sid{0};
  USB3Com _usb{};
};

inline int NKAMOREADC::Sid() const { return _sid; }
