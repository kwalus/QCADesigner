#ifndef _ENKIDU_API_H_
#define _ENKIDU_API_H_

#include <stdio.h>
#include <string>
#include <list>
#include <queue>
#include <map>
using namespace std;

//: Functor for sorting parcels in the priority queue
//
// This is only used internally, the end user should not concern
// themselves with it. Basicly, it just compares two parcels based on
// arrival time.
struct timeFunc : public binary_function<parcel*, parcel*, bool>
{
  //: Sorts by arrival time
  bool operator()(parcel *a, parcel*b) {
    return a->arrivalTime() > b->arrivalTime();
  }
};

//: Physical Component of the system
//
// This class represents a component of the system such as a CPU, PIM,
// network link, etc. It can send and recieve parcels. Implementors
// should inherit from this class when creating new components.
class component
{
public:
  virtual void printCellInfo() const {;}
  typedef map<string, component*> componentNameMap;
  typedef list<component*> componentVec;
private:
  //: Vector of all the components in the simulation
  static componentVec componentList;
  //: Map of all the components, by name
  static componentNameMap componentsByName;
  //: The Queue of pending events
  //
  // Whenever a parcel is sent, it is entered into the queue. Each
  // iteration, an event is grabbed from the queue and passed to the
  // appropriate component to process.
//  static priority_queue<parcel*,vector<parcel*>,timeFunc> parcelQ;
  static queue<parcel*> parcelQ;
  static queue<component*> majQ;
  //: The time (in cycles) of the simulation
  static int _cycle;
  //: Did we do any work this cycle?
  //
  // Used to determine if we are idle. Set by run().
  bool didWorkThisCycle;
  //: Keep the simulation running, even with no parcels pending
  //
  // Setting this to high will allow the simulation to continue even
  // if there are no more parcels pending.
  static bool _keepAlive;
protected:
  //: Object's name
  string _name;
public:
  component(string name);
  virtual ~component();
//  //: Find a given component based on its name
//  static component* findComponentByName(string s){return componentsByName[s];}
  //: Accessor for the keepAlive toggle
  //
  // Setting the keepAlive to high allows the simulation to continue,
  // even if there are no pending events;
  static bool& keepAlive() {return _keepAlive;}
  static void run(int forCycles);
  static void run();
  //: Get the number of cycles the simulation has been running
  static const int cycle() {return _cycle;}
  //: Accessor for the component's name
  string& name() {return _name;}
  //: Post-constructor setup
  //
  // Called immediatly before the simulation starts. Setup which
  // needs to be performed after all the components have been created
  // should be performed here.
  virtual void setup() = 0;
  //: Post-simulation finish
  //
  // Called after the simulation is finished.
  virtual void finish() = 0;
  //: Handle incoming parcel
  //
  // Called when a parcel has been delivered to a component. The
  // component should perform whatever actions are required to handle
  // the parcel, including generating new parcels.
  virtual void handleParcel(parcel *p) = 0;
  //: Perform actions at start of clock cycle
  //
  // Empty by default, override this function if the component needs
  // to perform any actions at the start of the clock cycle, before
  // any pracels have been handled.
  virtual void preTic() {;}
  //: Perform actions at end of clock cycle
  //
  // Empty by default, override this function if the component needs
  // to perform any actions at the end of the clock cycle, after all
  // pracels have been handled and all idle() functions have been
  // called.
  virtual void postTic() {;}
  //: Perform actions if idle this cycle
  //
  // Empty by default, pverride this function if the component needs
  // to perform any actions at the end of a clock cycle in which is
  // recieved no parcels. This function is called after all parcels
  // are handled, but before the postTic() functions have been called.
  virtual void idle() {;}
  void sendParcel(parcel *p, component* destination,int arrivalTime);
  void pushSelfOntoMajQueue();
  virtual void processInputs() {}
  // declared so that the API can call the Cell processInputs function
  virtual int getNumInputs() const {return 0;}
  // declared so that the API can call the Cell setIsMajority function
  virtual void setIsMajority(const bool a) {;}
  // declared so that the API can call the Cell setIsPotentialMaj function
  virtual void setIsPotentialMaj(const bool a) {;}
};

#endif

