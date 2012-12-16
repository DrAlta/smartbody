#include <vhcl.h>
#include "PandaRenderer.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
 
#include "genericAsyncTask.h"
#include "asyncTaskManager.h"


PandaRenderer::PandaRenderer(PandaFramework* framework, WindowFramework *window)
{
	m_framework = framework;
	m_window = window;

	m_sbListener = new SBListener(this);
	m_sbm = new Smartbody_dll;
	m_sbm->Init("../../../Python26/Lib", true);
	LOG("SmartBody initialized...");
	vhcl::Log::g_log.AddListener(new vhcl::Log::StdoutListener());
	
	m_sbm->SetListener(m_sbListener);

	vhmsg::ttu_set_client_callback( &PandaRenderer::tt_client_callback, this );
	int err = vhmsg::ttu_open();
	if ( err != vhmsg::TTU_SUCCESS )
	{
		printf("%s", "ttu_open failed!\n" );
	}
	else
	{
		printf("%s", "ttu_open success!\n" );
	}

	vhmsg::ttu_notify2( "vrComponent", "renderer all" );

	// sbm related vhmsgs
    vhmsg::ttu_register( "vrAllCall" );
    vhmsg::ttu_register( "vrKillComponent" );
    vhmsg::ttu_register( "sbm" );
    vhmsg::ttu_register( "vrAgentBML" );
    vhmsg::ttu_register( "vrSpeak" );
    vhmsg::ttu_register( "vrExpress" );
    vhmsg::ttu_register( "vrSpoke" );
    vhmsg::ttu_register( "RemoteSpeechReply" );
    vhmsg::ttu_register( "PlaySound" );
    vhmsg::ttu_register( "StopSound" );
    vhmsg::ttu_register( "CommAPI" );
    vhmsg::ttu_register( "object-data" );
    vhmsg::ttu_register( "wsp" );

	

	
}

PandaFramework* PandaRenderer::getPandaFramework()
{
	return m_framework;
}
	

WindowFramework* PandaRenderer::getWindowFramework()
{
	return m_window;
}

PandaRenderer::~PandaRenderer()
{
}

AsyncTask::DoneStatus PandaRenderer::do_task()
{
	// update SmartBody
	PT(ClockObject) globalClock  = ClockObject::get_global_clock();
	
	vhmsg::ttu_poll();
	
	bool updateOk = m_sbm->Update(globalClock->get_real_time());
	
	
	
	// update the characters
	for (std::map<std::string, NodePath>::iterator iter = m_characters.begin();
		 iter != m_characters.end();
		 iter++)
	{
		SmartbodyCharacter& character = m_sbm->GetCharacter((*iter).first);

		NodePath nodePath = (*iter).second;
		
		nodePath.set_pos(character.x, character.z, character.y);

		// update the joints
		size_t numJoints = character.m_joints.size();
		for (int j = 0; j < numJoints; j++)
		{
			std::string& jointName = character.m_joints[j].m_name;
			// find the corresponding joint in the actor and set the matrix accordingly
			// ...
			// ...			
		}

	}
	
	return AsyncTask::DS_cont;
}

void PandaRenderer::tt_client_callback( const char * op, const char * args, void * user_data )
{
   PandaRenderer* app = (PandaRenderer*)user_data;

   //NILOG( "ActiveMQ message received: '%s %s'\n", op, args );

   std::string sOp = op;
   std::string sArgs = args;
   std::vector< std::string > splitArgs;
   vhcl::Tokenize( sArgs, splitArgs );

   if ( sOp == "vrAllCall" )
   {
	  vhmsg::ttu_notify2( "vrComponent", "renderer Ogre" );
   }
   else if ( sOp == "vrKillComponent" )
   {
	  if ( splitArgs.size() > 0 )
	  {
		 if ( splitArgs[ 0 ] == "renderer" ||
			  splitArgs[ 0 ] == "all" )
		 {
			 // quit application...
		 }
	  }
   }

    app->m_sbm->ProcessVHMsgs(op, args);
}

std::map<std::string, NodePath>& PandaRenderer::getCharacters()
{
	return m_characters;
}
