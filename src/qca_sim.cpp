// qcaSim.cpp
// Class to run the actual QCA simulation
//
// Written by Dominic Antonelli
// 5 APR 2003

#include "qca_sim.h"
#include "cell_info.h"
#include "run_dig_sim.h"
#include "vector_table.h"

#include <sys/time.h>

int pow(int , int );

#ifdef __cplusplus
extern "C" {
#endif

// Count the number of active inputs in a vector table
int CountActiveVTInputs (VectorTable *pvt) ;

static simulation_data *sim_data = NULL ;

simulation_data * run_digital_simulation(int sim_type, qcell * my_cell, void *p, VectorTable *pvt) {
	struct timeval start, end;
	long elapsed_time;
	if (my_cell == NULL) {
		return NULL;
	}
	gettimeofday(&start, NULL);
        qcaSim my_sim("who cares?", sim_type, my_cell, pvt);
	gettimeofday(&end, NULL);
	if (my_sim.simulationWasSuccessful()) {
		printf("Simulation complete\n");
	}
	else {
		printf("Simulation failed\n");
	}

	// NOT WORKING AND I DON'T KNOW WHY!
	elapsed_time = (long)(((double)end.tv_usec - (double)start.tv_usec) / 1000.0);
	printf("Elapsed time: %ld msec\n", elapsed_time);

        return sim_data;
}

#ifdef __cplusplus
}
#endif

// Load the cells and ask the user for the values of the inputs
qcaSim::qcaSim(string nm, const int sim_type, const qcell * my_cell, VectorTable *pvtIn) : component(nm) {

	error = false;
	SIMULATION_TYPE = sim_type ;
	pvt = pvtIn ;

	loadCells(my_cell);
	if (error) {
		return;
	}
	printf("Cells loaded\n");
	keepAlive() = 1;
	printf("Calculating zone width of design\n");
	calculateNumZones();
	printf("Initializing simulation data\n");
	init();
	printf("Running simulation\n");

	run(sim_data->number_samples);
}

qcaSim::~qcaSim() {
	for (CellSet::iterator iter = cells->begin(); iter != cells->end(); iter++) {
		delete (*iter);
	}
	delete cells;
	delete inputs;
	delete outputs;
	delete activated_inputs;
}


void qcaSim::handleParcel(parcel *p) {
	parcel::deleteParcel(p);
}

// Load the cell data into my Cell class
void qcaSim::loadCells(const qcell * my_cell) {
	cells = new CellSet;
	inputs = new CellSet;
	outputs = new CellSet;
	activated_inputs = new list<Cell *>;

	if (SIMULATION_TYPE == VECTOR_TABLE) {
		// Add all the activated inputs for vector table mode
		for (int i = 0; i < pvt->num_of_inputs; i++)
			// Fish out the active inputs
			if (pvt->active_flag[i]) {
				Cell * new_cell = new Cell(pvt->inputs[i]);
				cells->insert(new_cell);
				inputs->insert(new_cell);
				activated_inputs->push_back(new_cell);
			}
	}
	while (my_cell != NULL) {
		Cell * new_cell = new Cell(my_cell);
		if (new_cell->isInput()) {
			bool activated = false;
			// If this is a vector table sim, check if this input is active
			// If it is, then we already added it above
			if (SIMULATION_TYPE == VECTOR_TABLE) {
				for (int i = 0; i < pvt->num_of_inputs; i++) {
					if (my_cell == pvt->inputs[i] && pvt->active_flag[i]) {
						activated = true;
						delete new_cell;
						break;
					}
				}
			}
			// If it is not activated then we still need to insert it
			if (!activated) {
				cells->insert(new_cell);
				inputs->insert(new_cell);
			}
		}
		else {
			cells->insert(new_cell);
			if (new_cell->isOutput()) {
				outputs->insert(new_cell);
			}
			else if (new_cell->isFixed()) {
				double the_charge = new_cell->getMyqcell()->cell_dots[new_cell->getMyqcell()->number_of_dots - 1].charge;

				//Should we expand this "error" range so that if the user puts in a strange
				//number like 5.32 it will barf?
				if ((the_charge > 7e-20) && (the_charge < 9e-20)) {
					printf("ERROR: Fixed cell has invalid polarity\n");
					error = true;
					return;
				}
				else if (the_charge > 9e-20) {
					new_cell->setState(LOW);
				}
				else {
					new_cell->setState(HIGH);
				}
			}
		}
		my_cell = my_cell->next;
	}
	for (CellSet::iterator iter = cells->begin(); iter != cells->end(); iter++) {
		CellNeighborSet * n = (*iter)->extractNeighbors(cells);
		(*iter)->setNeighbors(n);
	}
	for (CellSet::iterator iter = cells->begin(); iter != cells->end(); iter++) {
		int num_diagonal = 0;
		int num_straight = 0;
		int num_opp_orientation = 0;
		for (CellNeighborSet::iterator n_iter = (*iter)->getNeighbors()->begin(); n_iter != (*iter)->getNeighbors()->end();) {
			float normalized_angle = (*n_iter)->angle;
			while (normalized_angle < 0) {
				normalized_angle += PI_OVER_TWO;
			}
			while (normalized_angle >= PI_OVER_TWO) {
				normalized_angle -= PI_OVER_TWO;
			}
			/* different orientations */
			if ((*iter)->getOrientation() != (*n_iter)->neighbor->getOrientation()) {
				if ((normalized_angle > RIPANGLE1 + RIPTHRESHOLD) && (normalized_angle < RIPANGLE2 - RIPTHRESHOLD)) {
					(*iter)->getNeighbors()->erase(n_iter);
					n_iter = (*iter)->getNeighbors()->begin();
					num_straight = 0;
					num_diagonal = 0;
				}
				else {
					num_opp_orientation++;
					n_iter++;
				}
			}
			/* same orientation */
			else {
				if ((normalized_angle < NORMALTHRESHOLD) || (normalized_angle > PI_OVER_TWO - NORMALTHRESHOLD)) {
					num_straight++;
					n_iter++;
				}
				else if ((normalized_angle > INVANGLE - INVTHRESHOLD) && (normalized_angle < INVANGLE + INVTHRESHOLD)) {
					num_diagonal++;
					n_iter++;
				}
			}
		}
		// If a cell has 2 or more neighbors that are either of the opposite orientation or straight up, down, left, or right of it
		// then we toss out any diagonal neighbors of the same orientation
		// Also if it is a fixed cell, we throw out diagonal neighbors.  HOPEFULLY ONLY A TEMPORARY FIX
		if ((((num_straight + num_opp_orientation) >= 2) && (num_diagonal >= 1)) || (*iter)->isFixed()) {
			for (CellNeighborSet::iterator n_iter = (*iter)->getNeighbors()->begin(); n_iter != (*iter)->getNeighbors()->end();) {
				float normalized_angle = (*n_iter)->angle;
				while (normalized_angle < 0) {
					normalized_angle += PI_OVER_TWO;
				}
				while (normalized_angle >= PI_OVER_TWO) {
					normalized_angle -= PI_OVER_TWO;
				}
				if (((*iter)->getOrientation() == (*n_iter)->neighbor->getOrientation()) && ((normalized_angle > INVANGLE - INVTHRESHOLD) && (normalized_angle < INVANGLE + INVTHRESHOLD))) {
					num_diagonal--;
					(*iter)->getNeighbors()->erase(n_iter);
					n_iter = (*iter)->getNeighbors()->begin();
				}
				else {
					n_iter++;
				}
			}
		}
		if ((num_straight == 4) && (num_diagonal == 0)) {
			(*iter)->setIsPotentialMaj(true);
			for (CellNeighborSet::iterator n_iter = (*iter)->getNeighbors()->begin(); n_iter != (*iter)->getNeighbors()->end(); n_iter++) {
				if ((*iter)->getOrientation() != (*n_iter)->neighbor->getOrientation()) {
					(*iter)->setIsPotentialMaj(false);
				}
			}
		}
	}
}

void qcaSim::calculateNumZones() {
	num_zones = 0;
	stack<Cell *> my_stack;
	map<Cell *, int> depths;

	for (CellSet::iterator iter = cells->begin(); iter != cells->end(); iter++) {
		depths[(*iter)] = 0;
	}

	for (CellSet::iterator iter = inputs->begin(); iter != inputs->end(); iter++) {
		my_stack.push(*iter);
		depths[*iter] = 1;
	}

	while (!my_stack.empty()) {
		Cell * my_cell = my_stack.top();
		my_stack.pop();
		for (CellNeighborSet::iterator iter = my_cell->getNeighbors()->begin(); iter != my_cell->getNeighbors()->end(); iter++) {
			int my_clock_zone = my_cell->getClockZone();
			int next_clock_zone = my_cell->getClockZone() - 1;
			if (next_clock_zone < 0) {
				next_clock_zone += 4;
			}
			if ((*iter)->neighbor->getClockZone() == my_clock_zone) {
				if (depths[(*iter)->neighbor] < depths[my_cell]) {
					depths[(*iter)->neighbor] = depths[my_cell];
					my_stack.push((*iter)->neighbor);
				}
			}
			else if ((*iter)->neighbor->getClockZone() == next_clock_zone) {
				if (depths[(*iter)->neighbor] <= depths[my_cell]) {
					depths[(*iter)->neighbor] = depths[my_cell] + 1;
					my_stack.push((*iter)->neighbor);
				}
			}
		}
	}
	for (CellSet::iterator iter = outputs->begin(); iter != outputs->end(); iter++) {
		num_zones = max(num_zones, depths[(*iter)]);
	}
}

void qcaSim::init() {

	current_input_combo = 0;
	sim_data = (simulation_data *)malloc(sizeof(simulation_data));

	int total_number_of_inputs = inputs->size();
	int total_number_of_outputs = outputs->size();

	sim_data->clock_data = (TRACEDATA *)malloc(sizeof (struct TRACEDATA) * 4); //  4 traces - one for each clock

	if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION) {
		sim_data->number_samples = pow(2,inputs->size()+2) + num_zones + EXTRACYCLES;
	}
	else if (SIMULATION_TYPE == VECTOR_TABLE) {
		sim_data->number_samples = 4*pvt->num_of_vectors + num_zones + EXTRACYCLES;
	}
#ifdef STAND_ALONE_VERSION
	else if (SIMULATION_TYPE == USER_INPUT) {
		sim_data->number_samples = 0;
	}
#endif

	int clock_width = 1;

	for (int i = 0; i < 4; i++) {
		sim_data->clock_data[i].data = (double *)malloc (sizeof (double) * clock_width * sim_data->number_samples);
	}

	for (int i = 0; i < clock_width * sim_data->number_samples; i++) {
		switch ((i/clock_width)%4) {
		case 0:
			sim_data->clock_data[0].data[i] = 0;
			sim_data->clock_data[1].data[i] = 2;
			sim_data->clock_data[2].data[i] = 2;
			sim_data->clock_data[3].data[i] = 0;
			break;
		case 1:
			sim_data->clock_data[0].data[i] = 2;
			sim_data->clock_data[1].data[i] = 2;
			sim_data->clock_data[2].data[i] = 0;
			sim_data->clock_data[3].data[i] = 0;
			break;
		case 2:
			sim_data->clock_data[0].data[i] = 2;
			sim_data->clock_data[1].data[i] = 0;
			sim_data->clock_data[2].data[i] = 0;
			sim_data->clock_data[3].data[i] = 2;
			break;
		case 3:
			sim_data->clock_data[0].data[i] = 0;
			sim_data->clock_data[1].data[i] = 0;
			sim_data->clock_data[2].data[i] = 2;
			sim_data->clock_data[3].data[i] = 2;
			break;
		}
	}


	for (int i = 0; i < 4; i++){
		sim_data->clock_data[i].data_labels = (char *)malloc (sizeof ("CLOCK 0   "));
		sprintf (sim_data->clock_data[i].data_labels, "CLOCK %d", i);
		sim_data->clock_data[i].drawtrace = 1;
		sim_data->clock_data[i].trace_color = RED;
	}

	sim_data->number_of_traces = total_number_of_inputs + total_number_of_outputs;
	sim_data->trace = (TRACEDATA *)malloc (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

	// create and initialize the inputs into the sim data structure //
	CellSet::iterator iter = inputs->begin();
	for (int i = 0; i < total_number_of_inputs; i++){
		sim_data->trace[i].data_labels = (char *)malloc (sizeof (char) * (strlen ((*iter)->getMyqcell()->label) + 1));
		strcpy (sim_data->trace[i].data_labels, (*iter)->getMyqcell()->label);
		sim_data->trace[i].drawtrace = TRUE;
		sim_data->trace[i].trace_color = BLUE;
		sim_data->trace[i].data = (double *)malloc (sizeof (double) * sim_data->number_samples);
		iter++;
	}

	// create and initialize the outputs into the sim data structure //
	iter = outputs->begin();
	for (int i = 0; i < total_number_of_outputs; i++){
		sim_data->trace[i + total_number_of_inputs].data_labels = (char *)malloc (sizeof (char) * (strlen ((*iter)->getMyqcell()->label) + 1));
		strcpy (sim_data->trace[i + total_number_of_inputs].data_labels, (*iter)->getMyqcell()->label);
		sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
		sim_data->trace[i + total_number_of_inputs].trace_color = YELLOW;
		sim_data->trace[i + total_number_of_inputs].data = (double *)malloc (sizeof (double) * sim_data->number_samples);
		iter++;
	}
}

// Set up the inputs at the beginning of each clock cycle
void qcaSim::preTic() {
	if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION) {
		int exp = 4; // change every fourth clock phase
		for (CellSet::iterator iter = inputs->begin(); iter != inputs->end(); iter++, exp *= 2) {
			if ((*iter)->getClockZone() == SWITCH) {
				(*iter)->setState(((current_input_combo / exp) % 2) ? HIGH : LOW);
			}
		}
		current_input_combo++;
	}
	else if (SIMULATION_TYPE == VECTOR_TABLE) {
		// Handles those extra cycles after all vector inputs are sent
		// and also if the vector table is messed up
		for (CellSet::iterator iter = inputs->begin(); iter != inputs->end(); iter++) {
			(*iter)->setState(LOW);
		}
		if (pvt->num_of_vectors > (cycle()/4)) {
			// each input that is not activated will be set to low
			// if the vector table size is messed up, all inputs will be set to low
			if (activated_inputs->size() != (unsigned int)CountActiveVTInputs (pvt)) {
				printf("ERROR: NUMBER OF ACTIVE INPUTS DOES NOT MATCH NUMBER OF BITS IN VECTOR TABLE\n");
				printf("DIGITAL SIMULATION CANNOT PROCEED\n");
				error = true;
				keepAlive() = 0; // Tells the Enkidu API to quit at the end of the cycle
				// The inputs have all been set to 0 so the API won't croak.
				return;
			}
			int i = 0, idx = 0;
			for (list<Cell *>::iterator iter = activated_inputs->begin(); iter != activated_inputs->end(); iter++, i++) {
				// Skip over inactive inputs and line up the indices in the vector table with those in the list
				while (!pvt->active_flag[idx++]) ;
				(*iter)->setState((pvt->vectors[(cycle()/4)][idx - 1] == 1) ? HIGH : LOW);
			}
		}
	}
#ifdef STAND_ALONE_VERSION
	else {
		for (CellSet::iterator iter = inputs->begin(); iter != inputs->end(); iter++) {
			if ((*iter)->getClockZone() == SWITCH) {
				char input;
				do {
					printf("Enter state (1 or 0) for input %s: ", (*iter)->name().c_str());
					scanf("%c", &input);
					fflush(stdin);
				} while ((input != '1') && (input != '0'));

				(*iter)->setState((input == '1') ? HIGH : LOW);
			}
		}
	}
#endif
}

// Set the sim_data at the end of each clock cycle.
void qcaSim::postTic() {
	if (!error) {
		if ((SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION) || (SIMULATION_TYPE == VECTOR_TABLE)) {
			CellSet::iterator iter = inputs->begin();
			for (unsigned int i = 0; i < inputs->size(); i++, iter++) {
				if ((*iter)->getState() == HIGH) {
					sim_data->trace[i].data[cycle()] = 1;
				}
				else if ((*iter)->getState() == LOW) {
					sim_data->trace[i].data[cycle()] = -1;
				}
				else {
					sim_data->trace[i].data[cycle()] = 0;
				}
			}

			iter = outputs->begin();
			int num_inputs = inputs->size();
			for (unsigned int i = 0; i < outputs->size(); i++, iter++) {
				if ((*iter)->getState() == HIGH) {
					sim_data->trace[num_inputs + i].data[cycle()] = 1;
				}
				else if ((*iter)->getState() == LOW) {
					sim_data->trace[num_inputs + i].data[cycle()] = -1;
				}
				else {
					sim_data->trace[num_inputs + i].data[cycle()] = 0;
				}
			}
		}
#ifdef STAND_ALONE_VERSION
		else if (SIMULATION_TYPE == USER_INPUT) {
			string * states = new string[HIGH-LOW+1];
			states[0] = "LOW";
			states[UND-LOW] = "X";
			states[HIGH-LOW] = "HIGH";
			string clocks[4] = {"SWITCH", "HOLD", "RELEASE", "RELAX"};
			for (CellSet::iterator iter = inputs->begin(); iter != inputs->end(); iter++) {
				printf(	"%9s : %6s  %7s\n",
					(*iter)->name().c_str(),
					states[(*iter)->getState() - LOW].c_str(),
					clocks[(*iter)->getClockZone()].c_str()
				);
			}
			for (CellSet::iterator iter = outputs->begin(); iter != outputs->end(); iter++) {
				printf(	"%9s : %6s  %7s\n",
					(*iter)->name().c_str(),
					states[(*iter)->getState() - LOW].c_str(),
					clocks[(*iter)->getClockZone()].c_str()
				);
			}
			delete states;
			printf("Press enter to continue or 'q' to quit: ");
			char input;
			scanf("%c", &input);
			if (input == 'q') {
				keepAlive() = 0; // Tells the API to end at the end of this cycle
			}
			fflush(stdin);
		}
#endif
	}
}

int CountActiveVTInputs (VectorTable *pvt)
	{
	int Nix, ic = 0 ;
	
	for (Nix = 0 ; Nix < pvt->num_of_inputs ; Nix++)
		if (pvt->active_flag[Nix])
			ic++ ;
	return ic ;
	}

// Why is there no standard definition of an integer to an integer power???
int pow(int b, int e) {
	int result = 1;
	while (e > 0) {
		result *= b;
		e--;
	}
	return result;
}
