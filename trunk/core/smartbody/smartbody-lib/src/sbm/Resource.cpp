#include "Resource.h"

#include <sstream>

Resource::Resource()
{
	parent = NULL;
}

Resource::~Resource()
{
	for (unsigned int c = 0; c < children.size(); c++)
	{
		delete children[c];
	}
}

void Resource::addChild(Resource* resource)
{
	children.push_back(resource);
	resource->setParent(this);
}

Resource* Resource::getChild(unsigned int num)
{
	if (children.size() > num)
		return children[num];
	else
		return NULL;
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

	for (unsigned int c = 0; c < children.size(); c++)
	{
		for (int d = 0; d < depth; d++)
			stream << "\t";
		stream << children[c]->dump() << std::endl;
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
}

CmdResource::~CmdResource()
{
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



std::string CmdResource::dump()
{
	std::stringstream stream;
	stream << "* " << command;

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
	stream << "MotionFile["<<type<<"]: " << motionFile;
	stream << Resource::dump();
	return stream.str();	
}