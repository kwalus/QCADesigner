#ifndef _CELL_INFO_H_
#define _CELL_INFO_H_

class cellInfo {
public:
	cellInfo(float distance = 0, float angle = 0, int o_s = 0, int n_s = 0);
	float getDistance() const;
	float getAngle() const;
	int getOldState() const;
	int getNewState() const;
	int getOrientation() const;
	void setDistance(const float );
	void setAngle(const float );
	void setOldState(const int );
	void setNewState(const int );
	void setOrientation(const int );
private:
	float distance;
	float angle;
	int old_state, new_state;
	int orientation;
};

#endif
