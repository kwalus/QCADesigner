// cell.cpp
// class to hold information and procedures for simulating a QCA cell
//
// Written by Dominic Antonelli (dantonel@nd.edu)
// 5 APR 2003

#include "cell.h"

/*
Cell::Cell() : component("NO NAME") {
	setX(0);
	setY(0);
	setOrientation(0);
	setClockZone(0);
	setState(UND);
	setIsInput(0);
	setIsOutput(0);
	setIsFixed(0);
	setIsPotentialMaj(false);
	neighbors = new CellNeighborSet;
	neighbors->clear();
	inputs = new CellSet;
	inputs->clear();
//	who_i_sent_to = new CellSet;
//	who_i_sent_to->clear();
//	who_sent_to_me = new CellSet;
//	who_sent_to_me->clear();
}
*/

// Create a Cell with the data from c
Cell::Cell(const qcell * c) : component(c->label), my_qcell(c) {
	setClockZone(c->clock);
	setState(LOW);
	setIsPotentialMaj(false);
	setIsMajority(false);
	neighbors = new CellNeighborSet;
	neighbors->clear();
	inputs = new CellSet;
	inputs->clear();
}

// Create a Cell that is a copy of Cell c
Cell::Cell(const Cell & c) : component("NO NAME"), my_qcell(c.getMyqcell()){
	setClockZone(c.clock_zone);
	setState(c.state);
	setIsPotentialMaj(c.isPotentialMaj());
	setIsMajority(c.isMajority());
	neighbors = new CellNeighborSet;
	(*neighbors) = (*(c.neighbors));
	inputs = new CellSet;
	(*inputs) = (*(c.inputs));
}

// Destroy the Cell
Cell::~Cell() {
	delete neighbors;
	delete inputs;
}

void Cell::setClockZone(const int a) {
	if ((a >= 0)	&& (a <= 3))
		clock_zone = a;
}

// Move to the next clock phase
void Cell::incrementClockZone() {
	clock_zone++;
	if (clock_zone > RELAX) {
		clock_zone = SWITCH;
	}
}

void Cell::setState(const int a) {
	if ((a == LOW) || (a == HIGH) || (a == UND))
		state = a;
}

void Cell::setIsPotentialMaj(const bool a) {
	is_potential_maj = a;
}

void Cell::setIsMajority(const bool a) {
	is_majority = a;
}

void Cell::setNumInputs(const int a) {
	if (a >= 0) {
		num_inputs = a;
	}
}

// Deletes the old neighbors and sets the new ones from input
// The new set should NOT be used any more outside of this class
// as it will be deleted when this object is destroyed or
// setNeighbors is called again.
void Cell::setNeighbors(const CellNeighborSet * n) {
	delete neighbors;
	neighbors = (CellNeighborSet *)((unsigned int)n);
}

void Cell::setName(const string * n) {
	_name = n->c_str();
}

// Print the cell's infomation
void Cell::printCellInfo() const {
	printf("name=%s\n", _name.c_str());
	printf("state=%d\n", state);
	printf("bias=%d\n", bias);
	printf("neighbors=%d\n", neighbors->size());
	printf("x=%f\n", my_qcell->x);
	printf("y=%f\n", my_qcell->y);
	printf("orientation=%d\n", getOrientation());
	printf("clock_zone=%d\n", clock_zone);
	printf("is_input=%d\n",	isInput());
	printf("is_output=%d\n", isOutput());
	printf("is_fixed=%d\n",	isFixed());
	printf("\n");
}

// Print info about the cell's neighbors
void Cell::printNeighbors()	const {
	printf("Neighbors: \n");
	if (neighbors->empty())
		printf("NONE\n\n");
	for (CellNeighborSet::iterator iter = neighbors->begin(); iter != neighbors->end(); iter++) {
		printf("distance=%f\n",	(*iter)->distance);
		printf("angle=%f\n", (*iter)->angle);
		(*iter)->neighbor->printCellInfo();
	}
	printf("\n");
}

// Used to sort by x then y coordinants
bool Cell::operator<(const Cell	a) const {
	if (getX() < a.getX())
		return true;
	else if (getX() > a.getX())
		return false;
	else if (getY() < a.getY())
		return true;
	else
		return false;
}

// Find and return the set of the cells within distance RANGE from set_of_cells
// Note that the cell that this function is called for MUST be in set_of_cells
CellNeighborSet * Cell::extractNeighbors(CellSet * set_of_cells) {
	CellNeighborSet * n = new CellNeighborSet;
	n->clear();
	CellSet::iterator this_iter = set_of_cells->find(this);
	CellSet::iterator iter;
	if (this_iter == set_of_cells->end()) {
		printf("ERROR: cannot extract neighbors from cell set. Cell is not in set\n");
		return NULL;
	}
	// First go to the right
	iter = this_iter;
	iter++;
	while ((iter != set_of_cells->end()) && ((*iter)->getX() <= (getX() + RANGE))) {
		if (((*iter)->getY() <= (getY() + RANGE)) && ((*iter)->getY() >= (getY() - RANGE))) {
			CellNeighbor * new_neighbor = new CellNeighbor;
			new_neighbor->neighbor = (*iter);
			new_neighbor->distance = getDistance(*iter);
			if (new_neighbor->distance <= RANGE) {
				float angle = atan2((*iter)->getY() - getY(), (*iter)->getX() - getX());
				if (angle < 0) {
					angle += TWO_PI;
				}
				if (angle >= TWO_PI) {
					angle -= TWO_PI;
				}
				new_neighbor->angle = angle;
				while (angle >= PI_OVER_TWO) {
					angle -= PI_OVER_TWO;
				}
				new_neighbor->normalized_angle = angle;
				n->insert(new_neighbor);
			}
			else {
				delete new_neighbor;
			}
		}
		else {
		}
		iter++;
	}
	// Then go to the left
	iter = this_iter;
	if (iter != set_of_cells->begin()) {
		iter--;
		while ((*iter)->getX() >= getX() - RANGE)	{
			if (((*iter)->getY() <= getY() + RANGE) && ((*iter)->getY() >= getY() - RANGE)) {
				CellNeighbor * new_neighbor	= new CellNeighbor;
				new_neighbor->neighbor = (*iter);
				new_neighbor->distance = getDistance(*iter);
				if (new_neighbor->distance <= RANGE) {
					float angle	= atan2((*iter)->getY() - getY(), (*iter)->getX() - getX());
					if (angle <	0) {
						angle += TWO_PI;
					}
					if (angle >= TWO_PI) {
						angle -= TWO_PI;
					}
					new_neighbor->angle = angle;
					while (angle >= PI_OVER_TWO) {
						angle -= PI_OVER_TWO;
					}
					new_neighbor->normalized_angle = angle;
					n->insert(new_neighbor);
				}
				else {
					delete new_neighbor;
				}
			}
			else {
			}
			if (iter == set_of_cells->begin())
				break;
			iter--;
		}
	}
	return n;
}

const qcell * Cell::getMyqcell() const {
	return my_qcell;
}

float Cell::getX() const {
	return my_qcell->x;
}

float Cell::getY() const {
	return my_qcell->y;
}

int	Cell::getOrientation() const {
	return ((my_qcell->y > my_qcell->cell_dots[0].y - OTHRESHOLD) && (my_qcell->y < my_qcell->cell_dots[0].y + OTHRESHOLD));
//	return (my_qcell->orientation & 0x1);
}

int	Cell::getClockZone() const {
	return clock_zone;
}

float Cell::getDistance(const Cell * a)	const {
	return sqrt(pow((a->getX() - getX()), 2) + pow((a->getY() - getY()), 2));
}

bool Cell::isInput() const {
	return (my_qcell->is_input == 1);
}

bool Cell::isOutput() const	{
	return (my_qcell->is_output == 1);
}

bool Cell::isFixed() const {
	return (my_qcell->is_fixed == 1);
}

bool Cell::isPotentialMaj() const {
	return (is_potential_maj && !is_majority);
}

bool Cell::isMajority() const {
	return is_majority;
}

bool Cell::isWaiting() const {
	return is_waiting;
}

int Cell::getNumInputs() const {
	return num_inputs;
}

CellNeighborSet * Cell::getNeighbors() {
	return neighbors;
}

// Removes Cell a from the neighbor list, if it is there, otherwise does nothing
void Cell::removeNeighbor(const Cell * a) {
	for (CellNeighborSet::iterator iter = neighbors->begin(); iter != neighbors->end(); iter++) {
		if (a == (*iter)->neighbor) {
			neighbors->erase(iter);
			break;
		}
	}
}

int Cell::getState() const {
	return state;
}

// Receive info about a neighbor and decide whether to change state
// If state changes, send parcels to neighbors telling them this
void Cell::handleParcel(parcel * p) {

	if ((clock_zone == SWITCH) && (!isInput())) {
		float normalized_angle = p->getCellInfo()->getAngle();
		while (normalized_angle < 0) {
			normalized_angle += PI_OVER_TWO;
		}
		while (normalized_angle >= PI_OVER_TWO) {
			normalized_angle -= PI_OVER_TWO;
		}
		// Sender and receiver have different orientation
		if (getOrientation() != p->getCellInfo()->getOrientation()) {
			// Wire crossing case
			if ((normalized_angle < CROSSTHRESHOLD) || (normalized_angle > PI/2 - CROSSTHRESHOLD)) {
				cellInfo * c_info = p->getCellInfo();
				sendParcelsToNeighbors(c_info->getOldState(), c_info->getNewState(), c_info->getOrientation(), (Cell *)p->source());
				parcel::deleteParcel(p);
				return;
			}
			// Normal ripper case
			else if ((normalized_angle > RIPANGLE1 - RIPTHRESHOLD) && (normalized_angle < RIPANGLE1 + RIPTHRESHOLD)) {
				bias -= p->getCellInfo()->getOldState();
				bias += p->getCellInfo()->getNewState();
			}
			// Inverting ripper case
			else if ((normalized_angle > RIPANGLE2 - RIPTHRESHOLD) && (normalized_angle < RIPANGLE2 + RIPTHRESHOLD)) {
				bias += p->getCellInfo()->getOldState();
				bias -= p->getCellInfo()->getNewState();
			}
		}
		// Sender and receiver have same orientation
		else {
			// 90-degree cells
			if (getOrientation() == 0) {
				// normal wire
				if ((normalized_angle < NORMALTHRESHOLD) || (normalized_angle > PI_OVER_TWO - NORMALTHRESHOLD)) {
					bias -= p->getCellInfo()->getOldState();
					bias += p->getCellInfo()->getNewState();
				}
				// inverting wire
				else if ((normalized_angle < INVANGLE + INVTHRESHOLD) && (normalized_angle > INVANGLE - INVTHRESHOLD)) {
					bias += p->getCellInfo()->getOldState()/2;
					bias -= p->getCellInfo()->getNewState()/2;
				}
			}
			// 45-degree cells
			else if (getOrientation() == 1) {
				// inverting wire
				if ((normalized_angle < NORMALTHRESHOLD) || (normalized_angle > PI_OVER_TWO - NORMALTHRESHOLD)) {
					bias += p->getCellInfo()->getOldState();
					bias -= p->getCellInfo()->getNewState();
				}
				// normal wire
				else if ((normalized_angle < INVANGLE + INVTHRESHOLD) && (normalized_angle > INVANGLE - INVTHRESHOLD)) {
					bias -= p->getCellInfo()->getOldState()/2;
					bias += p->getCellInfo()->getNewState()/2;
				}
			}
		}


		int new_state;

		if ((bias >= HIGH)) {
			new_state = HIGH;
		}
		else if ((bias <= LOW)) {
			new_state = LOW;
		}
		else {
			new_state = UND;
		}
		// If this cell is a potential majority gate and just received a new input
		if (isPotentialMaj() && (inputs->find((Cell *)p->source()) == inputs->end()) && is_waiting) {
			inputs->insert((Cell *)p->source());
			if (num_inputs == 0) {
				pushSelfOntoMajQueue();
			}
			num_inputs++;
		}
		else if (isMajority()) {
			if (inputs->find((Cell *)p->source()) == inputs->end()) {
				inputs->insert((Cell *)p->source());
				num_inputs++;
			}
			if ((state != new_state) && ((bias >= 2*HIGH) || (bias <= 2*LOW) || (num_inputs == 3))) {
				sendParcelsToNeighbors(state, new_state);
				state = new_state;
			}
		}
		else if (state != new_state) {
			sendParcelsToNeighbors(state, new_state);
			state = new_state;
		}
	}
	else {
	}
	parcel::deleteParcel(p);
}

// Process the inputs to a potential majority gate
// Only do this once the right number of inputs have arrived
void Cell::processInputs() {

	is_waiting = false;

	int new_state;
	if (bias >= HIGH) {
		new_state = HIGH;
	}
	else if (bias <= LOW) {
		new_state = LOW;
	}
	else {
		new_state = UND;
	}
	if (state != new_state) {
		sendParcelsToNeighbors(state,new_state);
	}
	state = new_state;
}

// Send parcels tellings neighbors that state changed from old_state to new_state
void Cell::sendParcelsToNeighbors(const int old_state, const int new_state) {
	CellNeighborSet::iterator end_iter = neighbors->end();
	for (CellNeighborSet::iterator iter = neighbors->begin(); iter != end_iter; iter++) {
			parcel * p = parcel::newParcel();
			cellInfo * c_info = new cellInfo;
			c_info->setDistance((*iter)->distance);
			c_info->setAngle((*iter)->angle);
			c_info->setOldState(old_state);
			c_info->setNewState(new_state);
			c_info->setOrientation(getOrientation());
			p->getCellInfo() = c_info;
			sendParcel(p, (*iter)->neighbor , cycle());
	}
}

// Send parcels to all neighbors with orientation dest_orientation telling them that state chagned from
// old_state to new_state
// Used to pass a parcel through cells with different orientation (wire crossing case)
void Cell::sendParcelsToNeighbors(const int old_state, const int new_state, const int dest_orientation, const Cell * src_cell) {
	CellNeighborSet::iterator end_iter = neighbors->end();
	for (CellNeighborSet::iterator iter = neighbors->begin(); iter != end_iter;  iter++) {
		if ((dest_orientation == (*iter)->neighbor->getOrientation()) && ((*iter)->neighbor != src_cell)) {
			parcel * p = parcel::newParcel();
			cellInfo * c_info = new cellInfo;
			c_info->setDistance((*iter)->distance);
			c_info->setAngle((*iter)->angle);
			c_info->setOldState(old_state);
			c_info->setNewState(new_state);
			c_info->setOrientation(dest_orientation);
			p->getCellInfo() = c_info;
			sendParcel(p, (*iter)->neighbor , cycle());
		}
	}
}

// Set up the cell for the next cycle
void Cell::preTic() {
	if (isFixed()) {
		clock_zone = HOLD;
	}
	if (!isInput() && (clock_zone == SWITCH)) {
		state = UND;
	}
	else if ((clock_zone == RELEASE) || (clock_zone == RELAX)) {
		state = UND;
	}
	else if ((clock_zone == HOLD) || (isInput() && (clock_zone == SWITCH))) {
		sendParcelsToNeighbors(UND, state);
	}

	bias = state;
	inputs->clear();
	num_inputs = 0;
	if (isPotentialMaj() && (clock_zone == SWITCH)) {
		is_waiting = true;
	}
	else {
		is_waiting = false;
	}
}

// increment clock zone at the end of the cycle
// Console printing of states is disabled in this version
void Cell::postTic() {
//	if (name() != "NO NAME") {
//		string * states = new string[HIGH-LOW+1];
//		states[0] = "LOW";
//		states[UND-LOW] = "X";
//		states[HIGH-LOW] = "HIGH";
//		string clocks[4] = {"SWITCH", "HOLD", "RELEASE", "RELAX"};
//		printf("%9s : %6s  %7s\n", name().c_str(), states[state - LOW].c_str(), clocks[clock_zone].c_str());
//		delete states;
//	}

	incrementClockZone();
}
