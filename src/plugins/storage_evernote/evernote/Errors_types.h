/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
#ifndef Errors_TYPES_H
#define Errors_TYPES_H

#include <Thrift.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>



namespace evernote { namespace edam {

enum EDAMErrorCode {
  UNKNOWN = 1,
  BAD_DATA_FORMAT = 2,
  PERMISSION_DENIED = 3,
  INTERNAL_ERROR = 4,
  DATA_REQUIRED = 5,
  LIMIT_REACHED = 6,
  QUOTA_REACHED = 7,
  INVALID_AUTH = 8,
  AUTH_EXPIRED = 9,
  DATA_CONFLICT = 10,
  ENML_VALIDATION = 11,
  SHARD_UNAVAILABLE = 12
};

class EDAMUserException : public apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "24652790C81ECE22B629CB60A19F1E93";
  static const uint8_t binary_fingerprint[16]; // = {0x24,0x65,0x27,0x90,0xC8,0x1E,0xCE,0x22,0xB6,0x29,0xCB,0x60,0xA1,0x9F,0x1E,0x93};

  EDAMUserException() : parameter("") {
  }

  virtual ~EDAMUserException() throw() {}

  EDAMErrorCode errorCode;
  std::string parameter;

  struct __isset {
    __isset() : parameter(false) {}
    bool parameter;
  } __isset;

  bool operator == (const EDAMUserException & rhs) const
  {
    if (!(errorCode == rhs.errorCode))
      return false;
    if (__isset.parameter != rhs.__isset.parameter)
      return false;
    else if (__isset.parameter && !(parameter == rhs.parameter))
      return false;
    return true;
  }
  bool operator != (const EDAMUserException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const EDAMUserException & ) const;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

class EDAMSystemException : public apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "24652790C81ECE22B629CB60A19F1E93";
  static const uint8_t binary_fingerprint[16]; // = {0x24,0x65,0x27,0x90,0xC8,0x1E,0xCE,0x22,0xB6,0x29,0xCB,0x60,0xA1,0x9F,0x1E,0x93};

  EDAMSystemException() : message("") {
  }

  virtual ~EDAMSystemException() throw() {}

  EDAMErrorCode errorCode;
  std::string message;

  struct __isset {
    __isset() : message(false) {}
    bool message;
  } __isset;

  bool operator == (const EDAMSystemException & rhs) const
  {
    if (!(errorCode == rhs.errorCode))
      return false;
    if (__isset.message != rhs.__isset.message)
      return false;
    else if (__isset.message && !(message == rhs.message))
      return false;
    return true;
  }
  bool operator != (const EDAMSystemException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const EDAMSystemException & ) const;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

class EDAMNotFoundException : public apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "D0297FC5011701BD87898CC36146A565";
  static const uint8_t binary_fingerprint[16]; // = {0xD0,0x29,0x7F,0xC5,0x01,0x17,0x01,0xBD,0x87,0x89,0x8C,0xC3,0x61,0x46,0xA5,0x65};

  EDAMNotFoundException() : identifier(""), key("") {
  }

  virtual ~EDAMNotFoundException() throw() {}

  std::string identifier;
  std::string key;

  struct __isset {
    __isset() : identifier(false), key(false) {}
    bool identifier;
    bool key;
  } __isset;

  bool operator == (const EDAMNotFoundException & rhs) const
  {
    if (__isset.identifier != rhs.__isset.identifier)
      return false;
    else if (__isset.identifier && !(identifier == rhs.identifier))
      return false;
    if (__isset.key != rhs.__isset.key)
      return false;
    else if (__isset.key && !(key == rhs.key))
      return false;
    return true;
  }
  bool operator != (const EDAMNotFoundException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const EDAMNotFoundException & ) const;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

}} // namespace

#endif
