#include "Resource.h"

#include <sstream>

int CmdResource::commandIndex = 0;

Resource::Resource()
{
	children_limit = 1000;
	parent = NULL;
}

Resource::~Resource()
{
	for (std::list<Resource*>::iterator iter = children.begin(); 
		 iter != children.end();
		 iter++)
	{
		delete (*iter);
	}
}


void Resource::setChildrenLimit(int l)
{
	children_limit = l;
	while(l < (int)children.size())
		children.pop_front();
}

void Resource::addChild(Resource* resource)
{
	while((int)children.size() >= children_limit)
		children.pop_front();
	children.push_back(resource);
	resource->setParent(this);
}

Resource* Resource::getChild(unsigned int num)
{
	std::list<Resource *>::iterator iter = children.begin();
	for(unsigned int i = 0 ; i < num; i++)
	{
		iter++;
		if(iter == children.end())	return NULL;
	}
	return *iter;

//	if (children.size() > num)
//		return children[num];
//	else
//		return NULL;
}

Resource* Resource::getParent()
{
	return parent;
}

void Resource::setParent(Resource* p)
{
	parent = p;
}

int Resource::getNumChildren()
{
	return children.size();
}

std::string Resource::dump()
{
	std::stringstream stream;
	if (children.size() > 0)
		stream << std::endl;

	// determine the depth
	Resource* curParent = getParent();
	int depth = 1;
	while (curParent)
	{
		curParent = curParent->getParent();
		depth++;
	}

	std::list<Resource *>::iterator iter = children.begin();
	for (unsigned int c = 0; c < children.size(); c++)
	{
		if(iter == children.end())	return "";
		for (int d = 0; d < depth; d++)
			stream << "\t";
//		stream << children[c]->dump() << std::endl;
		stream << (*iter)->dump() << std::endl;
		iter ++;
	}
	return stream.str();
}

FileResource::FileResource()
{
	m_filePath = "";
}

FileResource::~FileResource()
{
}

void FileResource::setFilePath(std::string path)
{
	m_filePath = path;
}


std::string FileResource::getFilePath()
{
	return m_filePath;
}

std::string FileResource::dump()
{
	std::stringstream stream;
	stream << "File: " << m_filePath;
	stream << Resource::dump();
	return stream.str();
}

PathResource::PathResource()
{
	path = "";
}

PathResource::~PathResource()
{
}

void PathResource::setPath(std::string p)
{
	path = p;
}

std::string PathResource::getPath()
{
	return path;
}

void PathResource::setType(std::string t)
{
	type = t;
}

std::string PathResource::getType()
{
	return type;
}

std::string PathResource::dump()
{
	std::stringstream stream;
	stream << "Path: [" << type << "] " << path;
	stream << Resource::dump();
	return stream.str();
}


CmdResource::CmdResource()
{
	id = "";
	command = "";
	time = -1;

	commandIndex++; // increment the counter
	index = commandIndex;
}

CmdResource::~CmdResource()
{
}


int CmdResource::getCommandIndex()
{
	

	return commandIndex;
}

void CmdResource::setId(std::string str)
{
	id = str;
}

std::string CmdResource::getId()
{
	return id;
}


void CmdResource::setCommand(std::string c)
{
	command = c;
}

std::string CmdResource::getCommand()
{
	return command;
}

void CmdResource::setTime(double t)
{
	time = t;
}

double CmdResource::getTime()
{
	return time;
}

std::string CmdResource::dump()
{
	std::stringstream stream;
	stream << "* " << index << " " << time << " " << command;

	stream << Resource::dump();
	return stream.str();
}

MotionResource::MotionResource()
{
}

MotionResource::~MotionResource()
{
}

void MotionResource::setMotionFile(std::string motion)
{
	motionFile = motion;
}
		
std::string MotionResource::getMotionFile()
{
	return motionFile;
}

void MotionResource::setType(std::string t)
{
	type = t;
}
		
std::string MotionResource::getType()
{
	return type;
}
				
std::string MotionResource::dump()
{
	std::stringstream stream;
	stream << "MotionFile ["<<type<<"]: " << motionFile;
	stream << Resource::dump();
	return stream.str();	
}


ControllerResource::ControllerResource()
{
	time = -1;
	type = "";
}

ControllerResource::~ControllerResource()
{
}

void ControllerResource::setControllerName(std::string cname)
{
	controllerName = cname;
}

std::string ControllerResource::getControllerName()
{
	return controllerName;
}

void ControllerResource::setType(std::string t)
{
	type = t;
}

void ControllerResource::setTime(double t)
{
	time = t;
}

double ControllerResource::getTime()
{
	return time;
}
		
std::string ControllerResource::getType()
{
	return type;
}

std::string ControllerResource::dump()
{
	std::stringstream stream;
	stream << "Controller [" << controllerName << "]: " << type << " " << time;
	stream << Resource::dump();
	return stream.str();	
}
