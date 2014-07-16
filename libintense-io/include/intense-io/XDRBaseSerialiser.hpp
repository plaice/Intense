// ****************************************************************************
//
// XDRBaseSerialiser.h - A class for sending and receiving stdc++ strings
// and c++ base types through XDR (quasi-)binary serialisation, a subclass of
// BaseSerial.
//
// Copyright 2002 by Paul Swoboda.  All Rights Reserved.
//
// ****************************************************************************


#ifndef __XDRBASESERIALISER_HPP__
#define __XDRBASESERIALISER_HPP__


// This is just to prevent someone from sending us a string that is
// 2^20 bytes long (1meg):
#define DEFAULT_XDRBASESERIALISER_MAX_STRING_LENGTH 1048576


namespace intense {


  namespace io {


    class XDRBaseSerialiser
      : public virtual BaseSerialiser {

    protected:

      XDR encoder;

      XDR decoder;

      const static int bufferSize = 8;

      char inBuffer[bufferSize];

      char outBuffer[bufferSize];

    public:

      XDRBaseSerialiser
      (int initMaxStringLength = defaultMaxStringLength);

      virtual ~XDRBaseSerialiser
      ()
      {}

      void out
      (const std::string& s, std::ostream& os);

      void in
      (std::string& s, std::istream& is, int maxLength = -1);

      void out
      (char* src, int length, std::ostream& os);

      void in
      (char*& dest, int& length, std::istream& is, int maxLength = -1);

      void out
      (int i, std::ostream& os);

      void in
      (int& i, std::istream& is);

      void out
      (unsigned int i, std::ostream& os);

      void in
      (unsigned int& i, std::istream& is);

      void out
      (short int i, std::ostream& os);

      void in
      (short int& i, std::istream& is);

      void out
      (short unsigned int i, std::ostream& os);

      void in
      (short unsigned int& i, std::istream& is);

      virtual void out
      (long long int i, std::ostream& os);

      virtual void in
      (long long int& i, std::istream& is);

      virtual void out
      (unsigned long long int i, std::ostream& os);

      virtual void in
      (unsigned long long int& i, std::istream& is);

      void out
      (char c, std::ostream& os);

      void in
      (char& c, std::istream& is);

      void out
      (float f, std::ostream& os);

      void in
      (float& f, std::istream& is);

      void out
      (double d, std::ostream& os);

      void in
      (double& d, std::istream& is);

      void out
      (bool b, std::ostream& os);

      void in
      (bool& b, std::istream& is);

    };


  }


}


#endif // __XDRBASESERIALISER_HPP__
