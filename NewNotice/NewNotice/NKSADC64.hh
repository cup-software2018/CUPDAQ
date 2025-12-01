#pragma once

#include <cstdint>

#include "NewNotice/USB3Com.hh"

class NKSADC64 {
public:
  explicit NKSADC64(int sid);
  ~NKSADC64();

  int Open();
  void Close();

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;

  int Sid() const;

private:
  int _sid;
  USB3Com _usb;
};

inline int NKSADC64::Sid() const { return _sid; }
