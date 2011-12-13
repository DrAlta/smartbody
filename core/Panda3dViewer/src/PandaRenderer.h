#ifndef _PANDARENDERER_
#define _PANDARENDERER_

#include "bonebus.h"
#include "vhmsg-tt.h"

#include "SBListener.h"
#include "asyncTask.h"
#include "pandaFramework.h"
#include "pandaSystem.h"

class PandaRenderer : public AsyncTask
{
	public:
		PandaRenderer(PandaFramework* framework, WindowFramework* window);
		~PandaRenderer();

		PandaFramework* getPandaFramework();
		WindowFramework* getWindowFramework();
		std::map<std::string, NodePath>& getCharacters();

		virtual AsyncTask::DoneStatus do_task();
		static void tt_client_callback( const char * op, const char * args, void * user_data );

	protected:
		SBListener* m_sbListener;
		Smartbody_dll* m_sbm;
		std::map<std::string, NodePath> m_characters;
		WindowFramework* m_window;
		PandaFramework* m_framework;
};


#endif