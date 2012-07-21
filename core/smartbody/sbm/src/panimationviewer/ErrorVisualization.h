#ifndef _ERRORVISUALIZATION_H_
#define _ERRORVISUALIZATION_H_

#include <FL/Fl_Group.H>
#include <FL/Fl_Gl_Window.H>
#include <controllers/me_ct_param_animation_data.h>
#include <sr/sr_camera.h>
#include "Parameter3DVisualization.h"
#include <sb/SBAnimationState.h>

class ErrorVisualization : public VisualizationBase
{
public:
	ErrorVisualization(int x, int y, int w, int h, char* name);
	~ErrorVisualization();

	virtual void draw();	
	void setAnimationState(SmartBody::SBAnimationBlend* animBlend);
	void setDrawType(const std::string& type);
private:
	SmartBody::SBAnimationBlend* curBlend;	
	std::string drawType;
};

#endif
