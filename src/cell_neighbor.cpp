// cellNeighbor.cpp
// class to hold cell and distance/angle data for each Cell's set of neighbors
//
// Written by Dominic Antonelli (dantonel@nd.edu)
// 23 JULY 2003

#include "cell_neighbor.h"
#include "cell.h"

CellNeighbor::CellNeighbor() {
}

CellNeighbor::CellNeighbor(const CellNeighbor & n) {
	distance = n.distance;
	angle = n.angle;
	normalized_angle = n.normalized_angle;
	neighbor = n.neighbor;
}

void CellNeighbor::printCellInfo() const {
	neighbor->printCellInfo();
}

bool CellNeighbor::operator<(const CellNeighbor a) const {
	if (neighbor->getX() < a.neighbor->getX())
		return true;
	else if (neighbor->getX() > a.neighbor->getX())
		return false;
	else if (neighbor->getY() < a.neighbor->getY())
		return true;
	else
		return false;
}
