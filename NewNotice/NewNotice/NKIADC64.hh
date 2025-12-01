#pragma once

#include <cstdint>

#include "NewNotice/USB3Com.hh"

class NKIADC64 {
public:
  explicit NKIADC64(int sid);
  ~NKIADC64();

  int Open();
  void Close();

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;
  void FlushData() const;

  int Sid() const;

private:
  int _sid;
  USB3Com _usb;
};

inline int NKIADC64::Sid() const { return _sid; }
