#pragma once

#include <cstdint>

#include "NewNotice/USB3Com.hh"

class NKFADC500 {
public:
  explicit NKFADC500(int sid);
  ~NKFADC500();

  int Open();
  void Close();

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;

  int Sid() const;

private:
  int _sid;
  USB3Com _usb;
};

inline int NKFADC500::Sid() const { return _sid; }
