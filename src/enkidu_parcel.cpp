// enkidu_parcel.cpp
//
// Originally written by Arun Rodrigues
// Written by Dominic Antonelli (dantonel@nd.edu)
// 5 APR 2003

#include "enkidu.h"

//: Constructor
//
// Protected. Should never be called by user, only by parcel::newParcel
parcel::parcel() : _source(0), _destination(0), _cellInfo(0)
{
  
}

//: Destructor
//
// Protected. Should never be called by user, only by parcel::deleteParcel
parcel::~parcel()
{

}

//:Generator function
//
// Creates a new parcel. The component which first sends a parcel
// should create it unless it is passing on a parcel it has already
// recieved. A component which recieves a parcel should destroy it,
// unless it is passing it to another component.
parcel* parcel::newParcel()
{
  return new parcel;
}

//: Detructor function
//
// Destroys a parcel. The component which first sends a parcel
// should create it unless it is passing on a parcel it has already
// recieved. A component which recieves a parcel should destroy it,
// unless it is passing it to another component.
void parcel::deleteParcel(parcel *p)
{
  delete p;
}

//: Accessor for arrivalTime
int& parcel::arrivalTime() {return _arrivalTime;}

//: Accessor for parcel source
component*& parcel::source() {return _source;}

//: Accessor for destination
component*& parcel::destination() {return _destination;}

//: Accessor for cellInfo
cellInfo*& parcel::getCellInfo() {return _cellInfo;}

//: Accessor for attribute map
parcel::nameValueMap& parcel::attributes() {return _attributes;}

