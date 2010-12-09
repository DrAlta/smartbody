
#include "vhcl.h"

#include "smartbody-c-dll.h"
#include "smartbody-dll.h"


using std::string;


// Experimental interface

SimpleSmartbodyListener* SimpleSmartbodyListener::listener = NULL;

SimpleSmartbodyListener::SimpleSmartbodyListener()
{
	sbm = new Smartbody_dll();
	sbm->SetListener(this);
}

SimpleSmartbodyListener::~SimpleSmartbodyListener()
{
	delete sbm;
}

void SimpleSmartbodyListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	SimpleCharacter c;
	c.name = name;
	c.objectClass = objectClass;
	charactersCreated.push(c);

	// add an entry into the viseme map
	std::map<std::string, std::queue<SimpleViseme> >::iterator iter = visemes.find(name);
	if (iter == visemes.end())
	{
		visemes.insert( std::pair<std::string, std::queue<SimpleViseme> >(name, std::queue<SimpleViseme>()) );
	}
}

void SimpleSmartbodyListener::OnCharacterDelete( const std::string & name )
{
	SimpleCharacter c;
	c.name = name;
	charactersDeleted.push(c);
}

void SimpleSmartbodyListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{
	SimpleViseme v;
	v.name = name;
	v.visemeName = visemeName;
	v.weight = weight;
	v.blendTime = blendTime;

	std::map<std::string, std::queue<SimpleViseme> >::iterator iter = visemes.find(name);
	if (iter == visemes.end())
	{
		visemes.insert( std::pair<std::string, std::queue<SimpleViseme> >(name, std::queue<SimpleViseme>()) );
		iter = visemes.find(name);
	}

	(*iter).second.push(v);
}

bool HasCharacterCreated(std::string& name, std::string& objectClass)
{
	if (SimpleSmartbodyListener::listener)
	{
		if (SimpleSmartbodyListener::listener->charactersCreated.size() > 0)
		{
			SimpleCharacter& character = SimpleSmartbodyListener::listener->charactersCreated.front();
			name = character.name;
			objectClass = character.objectClass;
			SimpleSmartbodyListener::listener->charactersCreated.pop();
			return true;
		}
	}

	return false;
}

bool HasCharacterDeleted(std::string& name)
{
	if (SimpleSmartbodyListener::listener)
	{
		if (SimpleSmartbodyListener::listener->charactersDeleted.size() > 0)
		{
			SimpleCharacter& character = SimpleSmartbodyListener::listener->charactersDeleted.front();
			name = character.name;
			SimpleSmartbodyListener::listener->charactersDeleted.pop();
			return true;
		}
	}
	
	return false;
}

bool HasViseme(const std::string name, std::string& visemeName, float& weight, float& blendTime)
{
	if (SimpleSmartbodyListener::listener)
	{
		std::map<std::string, std::queue<SimpleViseme> >::iterator iter = SimpleSmartbodyListener::listener->visemes.find(name);
		if (iter != SimpleSmartbodyListener::listener->visemes.end())
		{		
			SimpleViseme& viseme = (*iter).second.front();
			visemeName = viseme.visemeName;
			weight = viseme.weight;
			blendTime = viseme.blendTime;
			(*iter).second.pop();
			return true;
		}
	}
	
	return false;
}

void SetSpeechAudiofileBasePath( const std::string & basePath )
{
	if (SimpleSmartbodyListener::listener)
	{
		SimpleSmartbodyListener::listener->sbm->SetSpeechAudiofileBasePath(basePath);
	}
}

void SetFacebone( const bool enabled )
{
	if (SimpleSmartbodyListener::listener)
	{
		SimpleSmartbodyListener::listener->sbm->SetFacebone(enabled);
	}
}

void SetProcessId( const std::string & processId )
{
	if (SimpleSmartbodyListener::listener)
	{
		SimpleSmartbodyListener::listener->sbm->SetProcessId(processId);
	}
}	

void Init()
{
	if (!SimpleSmartbodyListener::listener)
	{
		SimpleSmartbodyListener::listener = new SimpleSmartbodyListener();
	}

	SimpleSmartbodyListener::listener->sbm->Init();
}

void Shutdown()
{
	if (SimpleSmartbodyListener::listener)
	{
		SimpleSmartbodyListener::listener->sbm->Shutdown();
		delete SimpleSmartbodyListener::listener;
		SimpleSmartbodyListener::listener = NULL;
	}	
}

bool Update( const double timeInSeconds )
{
	if (SimpleSmartbodyListener::listener)
	{
		return SimpleSmartbodyListener::listener->sbm->Update(timeInSeconds);
	}
	else
	{
		return false;
	}
}

bool ProcessVHMsgs( const char * op, const char * args )
{
	if (SimpleSmartbodyListener::listener)
	{
		return SimpleSmartbodyListener::listener->sbm->ProcessVHMsgs(op, args);
	}
	else
	{
		return false;
	}
}

int GetNumberOfCharacters()
{
	if (SimpleSmartbodyListener::listener)
	{
		return SimpleSmartbodyListener::listener->sbm->GetNumberOfCharacters();
	}
	else
	{
		return 0;
	}
}

void GetCharacterInfo(std::string name, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz)
{
	if (SimpleSmartbodyListener::listener)
	{
			SmartbodyCharacter c = SimpleSmartbodyListener::listener->sbm->GetCharacter(name);
			name = c.m_name;
			x = c.x;
			y = c.y;
			z = c.z;
			rw = c.rw;
			rx = c.rx;
			ry = c.ry;
			rz = c.rz;
	}
}

void GetCharacterJointInfo(std::string name, int jointNum, std::string& jointName, float& x, float& y, float& z, float& rw, float& rx, float& ry, float& rz)
{
	if (SimpleSmartbodyListener::listener)
	{
		SmartbodyCharacter c = SimpleSmartbodyListener::listener->sbm->GetCharacter(name);
		if ((size_t) jointNum < c.m_joints.size())
		{
			SmartbodyJoint& j = c.m_joints[jointNum];
			jointName = j.m_name;
			x = j.x;
			y = j.y;
			z = j.z;
			rw = j.rw;
			rx = j.rx;
			ry = j.ry;
			rz = j.rz;
		}
	}
}

int GetNumJoints(std::string name)
{
	if (SimpleSmartbodyListener::listener)
	{
		return SimpleSmartbodyListener::listener->sbm->GetNumberOfCharacters();
	}
	else
	{
		return 0;
	}
}
