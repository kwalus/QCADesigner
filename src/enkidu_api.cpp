// enkidu_api.cpp
//
// Originally written by Arun Rodrigues
// Modified by Dominic Antonelli (dantonel@nd.edu)
// 5 APR 2003

#include "enkidu.h"
#include "cell.h"
#include <stdio.h>
#include <stdlib.h>

list<component*> component::componentList;
queue<parcel*> component::parcelQ;
queue<component*> component::majQ;

int component::_cycle = 0;
bool component::_keepAlive = 0;
static const int Mqsize = 100;
static int Oqsize = 0;
static int times_same = 0;
static int times_thru = 0;

// component::componentNameMap component::componentsByName;

//: Run the simulation for a set period of time or until done
//
// The simulation is declared done when there are no more pending
// events in the queue, unless the keepAlive has been set. For each
// cycle, the following order is observed:
//
//
// 1. foreach component: preTic()
//
// 2. foreach parcel in this cycle: parcelDestination->handleParcel(parcel)
//
// 3. foreach idle component: idle()
//
// 4. foreach component: postTic()
//!NOTE: Presently the pre/postTic() and idle() functions are called
//!NOTE: on every component. We could add an
//!NOTE: "(en/dis)able(Pre/Post)Tic()" function to improve speed if
//!NOTE: necessary.
void component::run(int forCycles)
{
	_cycle = 0;
	/* Set up */ // DOESN'T WORK FOR SOME STRANGE REASON - but I don't need it
//	for (componentVec::iterator i = componentList.begin(); i != componentList.end(); ++i) {
//		(*i)->setup();
//	}
	/* Run the simulation */
	while((parcelQ.size() > 0) || _keepAlive) {
		/* If we are running for a finite period of time */
		if (forCycles > 0) {
			if (_cycle >= forCycles) break;
		}
			/* Perform all pre-tics */
		for (componentVec::iterator i = componentList.begin(); i != componentList.end(); ++i) {
			(*i)->preTic();
		}
		fprintf(stderr, "done w/ preTic\n");
		fflush(stderr);
		bool done = false;
		while (!done) {
			done = true;
			/* Process the parcels for this cycle */
			while(!parcelQ.empty()) {
				parcel *p = parcelQ.front();
				fprintf(stderr, "Here10.\n");
				    fflush(stderr);
				    if (p->source() != NULL){
				  printf("source node!\n");
				  p->source()->printCellInfo();
				  fflush(stdout);
				}
				p->destination()->handleParcel(p);
				/* DEBUG CODE */
				// if (10 == parcelQ.size())
// 				  times_same++;
// 				fflush(stderr);
// 				if (times_same > 50){
// 				  fprintf(stderr, "enkidu parcels -- 1\n");
// 				  fflush(stderr);
// 				    /* print out offending cell locations */
// 				  for (int i = 0; i < parcelQ.size(); i++){
// 				    fprintf(stderr, "Here3.\n");
// 				    fflush(stderr);
// 				    if (p == NULL)
// 				      continue;
// 				    fprintf(stderr, "Here4.\n");
// 				    fflush(stderr);
// 				    /*if (p->source() != NULL){
// 				      printf("source node!\n");
// 				      p->source()->printCellInfo();
// 				      fflush(stdout);
// 				      }*/
// 				    fprintf(stderr, "Here5.\n");
// 				    fflush(stderr);
// 				    if (p->destination() != NULL){
// 				      printf("dest node\n.");
// 				      p->destination()->printCellInfo();
// 				      fflush(stdout);
// 				    }
// 				    fprintf(stderr, "Here.\n");
// 				    fflush(stderr);
// 				  }
// 				  exit(0);
// 				}
// 				Oqsize = parcelQ.size();
				/* END DEBUG CODE */
				p->destination()->didWorkThisCycle = 1;
				parcelQ.pop();
				times_thru++;
				if (times_thru == 1000){
				  fprintf(stderr, "Digital Simulation did not work.  Program Exiting.\n");
				  fflush(stderr);
				  exit(0);
				}
				  
				
			}
			fprintf(stderr, "print me 3!\n");
			fflush(stderr);
			if (!majQ.empty()) {
				done = false;
				int Qsize = majQ.size();
				int n = majQ.front()->getNumInputs();
				int tries = 1;
				while ((n != 1) && (n != 3)) {
					if (tries > Qsize) {
						fprintf(stderr, "ERROR.  tries = %d\n",tries);
						exit(1);
					}
					component * f = majQ.front();
					majQ.pop();
					majQ.push(f);
					n = majQ.front()->getNumInputs();
					tries++;
				}
				// Process this cell
				component * f = majQ.front();
				majQ.pop();
				f->processInputs();
				f->setIsMajority(n != 1);
				f->setIsPotentialMaj(false);
			}
			fprintf(stderr, "print me 2\n");
		}
		/* Run idle functions, if needed */
		for (componentVec::iterator i = componentList.begin(); i != componentList.end(); ++i) {
			if ((*i)->didWorkThisCycle == 0) {
				(*i)->idle();
			} else {
				(*i)->didWorkThisCycle = 0;
			}
		}
		/* Perform all post-tics */
		for (componentVec::iterator i = componentList.begin(); i != componentList.end(); ++i) {
			(*i)->postTic();
		}
		/* Advance the cycle count */
		_cycle++;
		fprintf(stderr, "add a cycle\n");
		fflush(stderr);
	}
		/* Finish */
//	for (list<component*>::iterator i = componentList.begin(); i != componentList.end(); ++i) {
//		(*i)->finish();
//	}
}
//: Run the simulation until done
//
// The simulation is declared done when there are no more pending
// events in the queue.
void component::run()
{
	run(-1);
}

//: Constructor
component::component(string nm) : _name(nm)
{
	componentList.push_back(this);
//	componentsByName[_name] = this;
	didWorkThisCycle = 0;
}

//: Destructor
component::~component()
{
	componentList.remove(this);
}

void component::sendParcel(parcel *p, component* destination, int arrivalTime)
{
  p->arrivalTime() = arrivalTime;
  p->destination() = destination;
  p->source() = this;
  parcelQ.push(p);
}

void component::pushSelfOntoMajQueue() {
  majQ.push(this);
}
