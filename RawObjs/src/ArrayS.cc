// @(#)root/cont:$Id: ArrayS.cc,v 1.3 2022/12/19 00:43:47 cupsoft Exp $
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \class ArrayS
\ingroup Containers
Array of shorts (16 bits per element).
*/

#include "RawObjs/ArrayS.hh"
#include "TBuffer.h"
#include <iostream>

ClassImp(ArrayS);

////////////////////////////////////////////////////////////////////////////////
/// Default ArrayS ctor.

ArrayS::ArrayS() { fArray = 0; }

////////////////////////////////////////////////////////////////////////////////
/// Create ArrayS object and set array size to n shorts.

ArrayS::ArrayS(Int_t n)
{
  fArray = 0;
  if (n > 0) Set(n);
}

////////////////////////////////////////////////////////////////////////////////
/// Create ArrayS object and initialize it with values of array.

ArrayS::ArrayS(Int_t n, const unsigned short * array)
{
  fArray = 0;
  Set(n, array);
}

////////////////////////////////////////////////////////////////////////////////
/// Copy constructor.

ArrayS::ArrayS(const ArrayS & array)
    : TArray(array)
{
  fArray = 0;
  Set(array.fN, array.fArray);
}

////////////////////////////////////////////////////////////////////////////////
/// ArrayS assignment operator.

ArrayS & ArrayS::operator=(const ArrayS & rhs)
{
  if (this != &rhs) { Set(rhs.fN, rhs.fArray); }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
/// Delete ArrayS object.

ArrayS::~ArrayS()
{
  delete[] fArray;
  fArray = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Adopt array arr into ArrayS, i.e. don't copy arr but use it directly
/// in ArrayS. User may not delete arr, ArrayS dtor will do it.

void ArrayS::Adopt(Int_t n, unsigned short * arr)
{
  if (fArray) { delete[] fArray; }

  fN = n;
  fArray = arr;
}

////////////////////////////////////////////////////////////////////////////////
/// Add short c at position i. Check for out of bounds.

void ArrayS::AddAt(unsigned short c, Int_t i)
{
  if (!BoundsOk("ArrayS::AddAt", i)) return;
  fArray[i] = c;
}

////////////////////////////////////////////////////////////////////////////////
/// Set size of this array to n shorts.
/// A new array is created, the old contents copied to the new array,
/// then the old array is deleted.
/// This function should not be called if the array was declared via Adopt.

void ArrayS::Set(Int_t n)
{
  if (n < 0) return;
  if (n != fN) {
    unsigned short * temp = fArray;
    if (n != 0) {
      fArray = new unsigned short[n];
      if (n < fN) { memcpy(fArray, temp, n * sizeof(unsigned short)); }
      else {
        memcpy(fArray, temp, fN * sizeof(unsigned short));
        memset(&fArray[fN], 0, (n - fN) * sizeof(unsigned short));
      }
    }
    else {
      fArray = 0;
    }
    if (fN) delete[] temp;
    fN = n;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Set size of this array to n shorts and set the contents.
/// This function should not be called if the array was declared via Adopt.

void ArrayS::Set(Int_t n, const unsigned short * array)
{
  if (fArray && fN != n) {
    delete[] fArray;
    fArray = 0;
  }
  fN = n;
  if (fN == 0) return;
  if (array == 0) return;
  if (!fArray) fArray = new unsigned short[fN];
  memmove(fArray, array, n * sizeof(unsigned short));
}

////////////////////////////////////////////////////////////////////////////////
/// Stream a ArrayS object.
/*
void ArrayS::Streamer(TBuffer & b)
{
  if (b.IsReading()) {
    Int_t n;
    b >> n;
    Set(n);
    b.ReadFastArray(fArray, n);
  }
  else {
    b << fN;
    b.WriteFastArray(fArray, fN);
  }
}

void ArrayS::DumpArray() const
{
  for (int i = 0; i < fN; i++) {
    std::cout << fArray[i] << " ";
  }
  std::cout << std::endl;
}
*/