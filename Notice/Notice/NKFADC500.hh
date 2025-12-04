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

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;

  int Sid() const;

private:
  int _sid{0};
  USB3Com _usb{};
};

inline int NKFADC500::Sid() const { return _sid; }
