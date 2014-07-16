// ****************************************************************************
//
// BinarySerialiserTestSource.cpp : A simple generator of a set of data
// serialised with a BinarySerialiser, to test for architectural compatability.
//
// Copyright 2004 Paul Swoboda.
//
// This file is part of the Intense project.
//
// Intense is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Intense is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Intense; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
//
// ****************************************************************************


using namespace std;


#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <pthread.h>
#include "intense-io.hpp"


using namespace intense::io;


int main
()
{
  ofstream out("BinarySerialiserTestSource.out");
  BinaryBaseSerialiser binarySerialiser;
  string testString = "this is a test";
  char* testCharArray = "this is also a test";
  int testCharArrayLength = strlen(testCharArray) + 1;
  int testInt0 = -123;
  int testInt1 = 123;
  long long testLongLong0 = -12345678910LL;
  long long testLongLong1 = 12345678910LL;
  unsigned int testUnsignedInt = 234;
  short int testShortInt0 = -345;
  short int testShortInt1 = 345;
  short unsigned int testShortUnsignedInt = 234;
  char testChar = 'z';
  float testFloat0 = -123.456e-10;
  float testFloat1 = 123.456e10;
  double testDouble0 = -123456.789e-50;
  double testDouble1 = 123456.789e50;
  bool testBool0 = false;
  bool testBool1 = true;

  try {
    binarySerialiser.out(testString, out);
    binarySerialiser.out(testCharArray, testCharArrayLength, out);
    binarySerialiser.out(testInt0, out);
    binarySerialiser.out(testInt1, out);
    binarySerialiser.out(testLongLong0, out);
    binarySerialiser.out(testLongLong1, out);
    binarySerialiser.out(testUnsignedInt, out);
    binarySerialiser.out(testShortInt0, out);
    binarySerialiser.out(testShortInt1, out);
    binarySerialiser.out(testShortUnsignedInt, out);
    binarySerialiser.out(testChar, out);
    binarySerialiser.out(testFloat0, out);
    binarySerialiser.out(testFloat1, out);
    binarySerialiser.out(testDouble0, out);
    binarySerialiser.out(testDouble1, out);
    binarySerialiser.out(testBool0, out);
    binarySerialiser.out(testBool1, out);
    out.flush();
    out.close();
  } catch (exception& e) {
    cerr << "ERROR: Caught exception: " << e.what() << '\n';
    return 1;
  }
}
