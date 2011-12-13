#include "SBListener.h"
#include "PandaRenderer.h"
#include <vhcl.h>

SBListener::SBListener(PandaRenderer* app)
{
	m_app = app;
}

void SBListener::OnCharacterCreate( const  std::string & name )
{	   
	OnCharacterCreate(name, "");
}


void SBListener::OnCharacterCreate( const  std::string & name, const  std::string & objectClass )
{	
	std::map<std::string, NodePath>& characters = m_app->getCharacters();
	std::map<std::string, NodePath>::iterator iter = characters.find(name);
	if (iter != characters.end())
	{
		LOG("Character %s already exists, ignoring...", name.c_str());
		return;
	}
	WindowFramework* window = m_app->getWindowFramework();
	PandaFramework* framework = m_app->getPandaFramework();
	NodePath actor = window->load_model(framework->get_models(), "panda-model");
    actor.set_scale(0.005);
    actor.reparent_to(window->get_render());

	characters.insert(std::pair<std::string, NodePath>(name, actor));
}

void SBListener::OnCharacterDelete( const  std::string & name )
{
	
}

void SBListener::OnCharacterChange( const  std::string & name )
{
	
}
