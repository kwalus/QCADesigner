#ifndef _ENKIDU_PARCEL_H_
#define _ENKIDU_PARCEL_H_

#include <map>
#include <string>
using namespace std;

class component;

//: A parcel to be passed between components
//
// This class represents an event passed between components. This
// event could be data, a thread, or run-time meta information. An
// attribute map of name/value pairs is provided to give an extensible
// interface to hold information.
class parcel
{
public:
  typedef map<string, void*> nameValueMap;
protected:
  parcel();
  ~parcel();
  //: Where it came from
  component *_source;
  //: Where it is going
  component *_destination;
  //: The thread associated with this parcel
  //
  // Since a parcel is frequenly a thread, we have a pointer prepared
  // for it so you don't have to cast stuff all the time.
  cellInfo *_cellInfo;
  //: User defined attributes
  //
  // A map structure. <tt>strings</tt> are the index, <tt>void*</tt>
  // are the values.
  nameValueMap _attributes; // NOT REALLY USED
  //: When the message is schedualed to arrive
  int _arrivalTime; // NOT REALLY USED
public:
  static parcel* newParcel();
  static void deleteParcel(parcel*);
  int& arrivalTime();
  component*& source();
  component*& destination();
  cellInfo*& getCellInfo();
  nameValueMap& attributes();
};

#endif
