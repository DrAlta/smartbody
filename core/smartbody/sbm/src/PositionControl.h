#pragma once
#include <SR/sr_vec.h>
#include <SR/sr_vec2.h>
#include <SR/sr_camera.h>

class PositionControl
{
protected:
	bool active;	
	int opdir;
	float base,r,len,s_len,ss_len;
	SrVec colors[4];
	SrVec worldPt;
public:
	bool dragging;
public:
	PositionControl(void);
	~PositionControl(void);

	virtual SrVec getWorldPt(); // get world position ( from the attached object ? )
	virtual void setWorldPt(SrVec& newPt);
	virtual void draw(); 
	
	void setColor(const SrVec &color);
	bool drag(SrCamera& cam, float  fx, float fy, float tx, float ty);
	void hitOPS();
	void hitTest();

	static void drawSphere(SrVec& pos, float fRadius = 1.0);

protected:
	void drawCenter();		
};

