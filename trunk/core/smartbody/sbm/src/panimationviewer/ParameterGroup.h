#ifndef _PARAMETERGROUP_H_
#define _PARAMETERGROUP_H_

#include <FL/Fl_Group.H>
#include <sbm/me_ct_param_animation_data.h>
#include "PanimationWindow.h"

class ParameterVisualization;
class Parameter3DVisualization;

namespace SmartBody {
	class SBCharacter;
}
class ParameterGroup : public Fl_Group
{
	public:
		ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex = false);
		~ParameterGroup();

		virtual void resize(int x, int y, int w, int h);
		
		static void updateXAxisValue(Fl_Widget* widget, void* data);
		static void updateXYAxisValue(Fl_Widget* widget, void* data);
		static void updateXYZAxisValue(Fl_Widget* widget, void* data);
		void updateWeight();
		PAStateData* getCurrentPAStateData();
		SmartBody::SBCharacter* getCurrentCharacter();

	public:
		PanimationWindow* paWindow;
		PAStateData* stateData;
		ParameterVisualization* paramVisualization;
		Parameter3DVisualization* param3DVisualization;
		bool exec;
		Fl_Value_Slider* xAxis;
		Fl_Value_Slider* yAxis;
		Fl_Value_Slider* zAxis;
};

#endif