#ifndef _CELL_NEIGHBOR_H_
#define _CELL_NEIGHBOR_H_

#include <set>
using namespace std;

#include "ptr_comp.h"

class Cell;

class CellNeighbor {
	friend class Cell;
public:
	CellNeighbor();
	CellNeighbor(const CellNeighbor & n);
	void printCellInfo() const;
	bool operator<(const CellNeighbor a) const;
	float distance;
	float angle;
	float normalized_angle;
	Cell * neighbor;
};

//class CellNeighborSet : public set<CellNeighbor *, ptr_comp<CellNeighbor> > {};

/*
struct cell_neighbor_comp {
	bool operator()(CellNeighbor * a, CellNeighbor * b) {
		return (*(a->neighbor) < *(b->neighbor));
	}
};
*/

#endif
