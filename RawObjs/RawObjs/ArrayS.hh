// @(#)root/cont:$Id: ArrayS.hh,v 1.3 2022/12/19 00:43:47 cupsoft Exp $
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_ArrayS
#define ROOT_ArrayS

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ArrayS                                                              //
//                                                                      //
// Array of shorts (16 bits per element).                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TArray.h"

class ArrayS : public TArray {
public:
  unsigned short * fArray; //[fN] Array of fN shorts

  ArrayS();
  ArrayS(Int_t n);
  ArrayS(Int_t n, const unsigned short * array);
  ArrayS(const ArrayS & array);
  ArrayS & operator=(const ArrayS & rhs);
  virtual ~ArrayS();

  void Adopt(Int_t n, unsigned short * array);
  void AddAt(unsigned short c, Int_t i);
  unsigned short At(Int_t i) const;
  void Copy(ArrayS & array) const { array.Set(fN, fArray); }
  const unsigned short * GetArray() const { return fArray; }
  unsigned short * GetArray() { return fArray; }
  Double_t GetAt(Int_t i) const { return At(i); }
  Stat_t GetSum() const
  {
    Stat_t sum = 0;
    for (Int_t i = 0; i < fN; i++)
      sum += fArray[i];
    return sum;
  }
  void Reset() { memset(fArray, 0, fN * sizeof(unsigned short)); }
  void Reset(unsigned short val)
  {
    for (Int_t i = 0; i < fN; i++)
      fArray[i] = val;
  }
  void Set(Int_t n);
  void Set(Int_t n, const unsigned short * array);
  void SetAt(Double_t v, Int_t i) { AddAt((unsigned short)v, i); }
  unsigned short & operator[](Int_t i);
  unsigned short operator[](Int_t i) const;

  void DumpArray() const;

  ClassDef(ArrayS, 1) // Array of shorts
};

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer & operator>>(TBuffer & buf, ArrayS *& obj)
{
  // Read ArrayS object from buffer.

  obj = (ArrayS *)TArray::ReadArray(buf, ArrayS::Class());
  return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer & operator<<(TBuffer & buf, const ArrayS * obj)
{
  // Write a ArrayS object into buffer
  return buf << (const TArray *)obj;
}

inline unsigned short ArrayS::At(Int_t i) const
{
  if (!BoundsOk("ArrayS::At", i)) return 0;
  return fArray[i];
}

inline unsigned short & ArrayS::operator[](Int_t i)
{
  if (!BoundsOk("ArrayS::operator[]", i)) { i = 0; }
  return fArray[i];
}

inline unsigned short ArrayS::operator[](Int_t i) const
{
  if (!BoundsOk("ArrayS::operator[]", i)) return 0;
  return fArray[i];
}

#endif
