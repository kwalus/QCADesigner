#ifndef _CELL_H_
#define _CELL_H_

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <set>
using namespace std;

#include "enkidu.h"
#include "cell_neighbor.h"
#include "ptr_comp.h"
#include "globals.h"

#define RANGE 30.1  // Max distance between interacting cells

#define SWITCH 0
#define HOLD 1
#define RELEASE 2
#define RELAX 3

// The simulation assumes LOW < UND < HIGH
// and LOW and HIGH are even numbers with equal distance from UND
#define LOW (-2)
#define HIGH 2
#define UND 0

const float TWO_PI = 2*PI;
const float PI_OVER_TWO = PI/2;

// Angles and Thresholds for determining whether cells interact and in what way
const float NORMALTHRESHOLD = PI/8;
const float INVANGLE = PI/4;
const float INVTHRESHOLD = PI/8;
const float RIPANGLE1 = atan((double)0.5);
const float RIPANGLE2 = atan((double)2);
const float RIPTHRESHOLD = RIPANGLE1/2;
const float CROSSTHRESHOLD = RIPANGLE1-RIPTHRESHOLD;

const float OTHRESHOLD = 0.01; // Threshold for saying that two positions are equal

class Cell;
class CellNeighbor;

typedef set<CellNeighbor *, ptr_comp<CellNeighbor> > CellNeighborSet;
typedef set<Cell *, ptr_comp<Cell> > CellSet;

class Cell : public component {
public:
	Cell(const qcell * c);
	Cell(const Cell & c);
	~Cell();
	void setClockZone(const int a);
	void incrementClockZone();
	void setState(const int a);
	void setIsPotentialMaj(const bool a);
	void setIsMajority(const bool a);
	void setNumInputs(const int a);
	void setNeighbors(const CellNeighborSet * n);
	void setName(const string * n);
	virtual void printCellInfo() const;
	void printNeighbors() const;
	bool operator<(const Cell a) const;
	CellNeighborSet * extractNeighbors(CellSet * set_of_cells);
	const qcell * getMyqcell() const;
	float getX() const;
	float getY() const;
	int getOrientation() const;
	int getClockZone() const;
	int getState() const;
	float getDistance(const Cell * a) const;
	bool isInput() const;
	bool isOutput() const;
	bool isFixed() const;
	bool isPotentialMaj() const;
	bool isWaiting() const;
	bool isMajority() const;
	virtual int getNumInputs() const;
	CellNeighborSet * getNeighbors();
	void removeNeighbor(const Cell * a);
	virtual void handleParcel(parcel *);
	virtual void processInputs();
	virtual void setup() {;}
	virtual void preTic();
	virtual void postTic();
	virtual void finish() {;}
	void sendParcelsToNeighbors(const int old_state, const int new_state);
	void sendParcelsToNeighbors(const int old_state, const int new_state, const int dest_orientation, const Cell * src_cell);
protected:
	const qcell * my_qcell;
	int state, clock_zone;
	bool is_potential_maj;
	bool is_waiting;
	bool is_majority;
	CellNeighborSet * neighbors;
	int bias;
	int num_inputs;
	CellSet * inputs;
};

#endif
