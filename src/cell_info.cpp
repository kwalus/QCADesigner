// cell_info.cpp
// class to hold Cell information sent with parcels
//
// Written by Dominic Antonelli (dantonel@nd.edu)
// 5 APR 2003


#include "enkidu.h"
#include "cell_info.h"

cellInfo::cellInfo(float d, float a, int o_s, int n_s) : distance(d), angle(a),
 old_state(o_s), new_state(n_s) {}

float cellInfo::getDistance() const {
	return distance;
}

float cellInfo::getAngle() const {
	return angle;
}

int cellInfo::getOldState() const {
	return old_state;
}

int cellInfo::getNewState() const {
	return new_state;
}

int cellInfo::getOrientation() const {
	return orientation;
}

void cellInfo::setDistance(const float d) {
	distance = d;
}

void cellInfo::setAngle(const float a) {
	angle = a;
}

void cellInfo::setOldState(const int s) {
	old_state = s;
}

void cellInfo::setNewState(const int s) {
	new_state = s;
}

void cellInfo::setOrientation(const int o) {
	orientation = o;
}

