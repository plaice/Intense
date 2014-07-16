// ****************************************************************************
//
// IntenseIOTest0.cpp : A basic sanity test for binary and XDR serialisation.
//
// Copyright 2003 Paul Swoboda.
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


#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <pthread.h>
#include "intense-io.hpp"
#include "Test.hpp"
#include "IntenseIOTest.hpp"


using namespace std;
using namespace intense::io;


class IntenseIOTest0
  : public IntenseIOTest {

private:

  BinaryBaseSerialiser binarySerialiser;

  XDRBaseSerialiser xdrSerialiser;

  string testString;

  char testCharArray[64];

  int testCharArrayLength;

  int testInt0;

  int testInt1;

  unsigned int testUnsignedInt;

  short int testShortInt0;

  short int testShortInt1;

  short unsigned int testShortUnsignedInt;

  char testChar;

  float testFloat0;

  float testFloat1;

  double testDouble0;

  double testDouble1;

  bool testBool0;

  bool testBool1;

public:

  IntenseIOTest0
  ()
    : IntenseIOTest("IntenseIOTest0")
  {
    testString = "this is a test";
    strcpy(testCharArray, "this is also a test");
    testCharArrayLength = strlen(testCharArray) + 1;
    testInt0 = -123;
    testInt1 = 123;
    testUnsignedInt = 234;
    testShortInt0 = -345;
    testShortInt1 = 345;
    testShortUnsignedInt = 234;
    testChar = 'z';
    testFloat0 = -123.456e-10;
    testFloat1 = 123.456e10;
    testDouble0 = -123456.789e-50;
    testDouble1 = 123456.789e50;
    testBool0 = false;
    testBool1 = true;
  }

  void test
  ();

  void writeAll
  (BaseSerialiser& serialiser, ostream& os);

  void readAll
  (BaseSerialiser& serialiser, istream& in);

};


IntenseIOTest* test = new IntenseIOTest0();


void IntenseIOTest0::test
()
{
  //  stringstream buffer;
  int pipes[2];
  unsigned int incomingByteCount;
  unsigned int outgoingByteCount;
  ByteCountFileInStream in(incomingByteCount);
  ByteCountFileOutStream out(outgoingByteCount);
  //  FileInStream in;
  //  FileOutStream out;
  BinaryBaseSerialiser bs;
  XDRBaseSerialiser xs;

  if (pipe(pipes) != 0) {
    throw TestException("Couldn't create pipes");
  }
  out.attach(pipes[1]);
  out.exceptions(ios::failbit|ios::badbit);
  in.attach(pipes[0]);
  in.exceptions(ios::failbit|ios::badbit);
  writeAll(bs, out);
  readAll(bs, in);
  writeAll(xs, out);
  readAll(xs, in);
  in.close();
  out.close();
}


void IntenseIOTest0::writeAll
(BaseSerialiser& serialiser, ostream& os)
{
  serialiser.out(testString, os);
  serialiser.out(testCharArray, testCharArrayLength, os);
  serialiser.out(testInt0, os);
  serialiser.out(testInt1, os);
  serialiser.out(testUnsignedInt, os);
  serialiser.out(testShortInt0, os);
  serialiser.out(testShortInt1, os);
  serialiser.out(testShortUnsignedInt, os);
  serialiser.out(testChar, os);
  serialiser.out(testFloat0, os);
  serialiser.out(testFloat1, os);
  serialiser.out(testDouble0, os);
  serialiser.out(testDouble1, os);
  serialiser.out(testBool0, os);
  serialiser.out(testBool1, os);
  os.flush();
}


void IntenseIOTest0::readAll
(BaseSerialiser& serialiser, istream& is)
{
  string tempString;
  char* tempCharArray;
  int tempCharArrayLength;
  int tempInt0;
  int tempInt1;
  unsigned int tempUnsignedInt;
  short int tempShortInt0;
  short int tempShortInt1;
  short unsigned int tempShortUnsignedInt;
  char tempChar;
  float tempFloat0;
  float tempFloat1;
  double tempDouble0;
  double tempDouble1;
  bool tempBool0;
  bool tempBool1;

  serialiser.in(tempString, is);
  if (tempString != testString) {
    ostringstream oss;

    oss << "Deserialised testString \"" << tempString << "\" != \""
        << testString << "\"";
    throw TestException(oss.str());
  }
  serialiser.in(tempCharArray, tempCharArrayLength, is);
  if (strcmp(tempCharArray, testCharArray)) {
    ostringstream oss;

    oss << "Deserialised testCharArray \"" << tempCharArray
        << "\" != \"" << testCharArray << "\"";
    throw TestException(oss.str());
  }
  if (tempCharArrayLength != testCharArrayLength) {
    ostringstream oss;

    oss << "Deserialised testCharArrayLength " << tempCharArrayLength
        << " != " << testCharArrayLength;
    throw TestException(oss.str());
  }
  delete[] tempCharArray;
  serialiser.in(tempInt0, is);
  if (tempInt0 != testInt0) {
    ostringstream oss;

    oss << "Deserialised testInt0 " << tempInt0 << " != " << testInt0;
    throw TestException(oss.str());
  }
  serialiser.in(tempInt1, is);
  if (tempInt1 != testInt1) {
    ostringstream oss;

    oss << "Deserialised testInt1 " << tempInt1 << " != " << testInt1;
    throw TestException(oss.str());
  }
  serialiser.in(tempUnsignedInt, is);
  if (tempUnsignedInt != testUnsignedInt) {
    ostringstream oss;

    oss << "Deserialised testUnsignedInt " << tempUnsignedInt
        << " != " << testUnsignedInt;
    throw TestException(oss.str());
  }
  serialiser.in(tempShortInt0, is);
  if (tempShortInt0 != testShortInt0) {
    ostringstream oss;

    oss << "Deserialised testShortInt0 " << tempShortInt0
        << " != " << testShortInt0;
    throw TestException(oss.str());
  }
  serialiser.in(tempShortInt1, is);
  if (tempShortInt1 != testShortInt1) {
    ostringstream oss;

    oss << "Deserialised testShortInt1 " << tempShortInt1
        << " != " << testShortInt1;
    throw TestException(oss.str());
  }
  serialiser.in(tempShortUnsignedInt, is);
  if (tempShortUnsignedInt != testShortUnsignedInt) {
    ostringstream oss;

    oss << "Deserialised testShortUnsignedInt " << tempShortUnsignedInt
        << " != " << testShortUnsignedInt;
    throw TestException(oss.str());
  }
  serialiser.in(tempChar, is);
  if (tempChar != testChar) {
    ostringstream oss;

    oss << "Deserialised testChar " << tempChar << " != " << testChar;
    throw TestException(oss.str());
  }
  serialiser.in(tempFloat0, is);
  if (tempFloat0 != testFloat0) {
    ostringstream oss;

    oss << "Deserialised testFloat0 " << tempFloat0 << " != " << testFloat0;
    throw TestException(oss.str());
  }
  serialiser.in(tempFloat1, is);
  if (tempFloat1 != testFloat1) {
    ostringstream oss;

    oss << "Deserialised testFloat1 " << tempFloat1 << " != " << testFloat1;
    throw TestException(oss.str());
  }
  serialiser.in(tempDouble0, is);
  if (tempDouble0 != testDouble0) {
    ostringstream oss;

    oss << "Deserialised testDouble0 " << tempDouble0 << " != " << testDouble0;
    throw TestException(oss.str());
  }
  serialiser.in(tempDouble1, is);
  if (tempDouble1 != testDouble1) {
    ostringstream oss;

    oss << "Deserialised testDouble1 " << tempDouble1 << " != " << testDouble1;
    throw TestException(oss.str());
  }
  serialiser.in(tempBool0, is);
  if (tempBool0 != testBool0) {
    ostringstream oss;

    oss << "Deserialised testBool0 " << tempBool0 << " != " << testBool0;
    throw TestException(oss.str());
  }
  serialiser.in(tempBool1, is);
  if (tempBool1 != testBool1) {
    ostringstream oss;

    oss << "Deserialised testBool1 " << tempBool1 << " != " << testBool1;
    throw TestException(oss.str());
  }
}
