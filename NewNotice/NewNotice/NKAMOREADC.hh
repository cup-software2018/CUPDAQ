#pragma once

#include <cstdint>

#include "NewNotice/USB3Com.hh"

class NKAMOREADC {
public:
  explicit NKAMOREADC(int sid);
  ~NKAMOREADC();

  int Open();
  void Close();

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;

  int Sid() const;

private:
  void WaitFPGAAndInit();

  int _sid;
  USB3Com _usb;
};

inline int NKAMOREADC::Sid() const { return _sid; }
