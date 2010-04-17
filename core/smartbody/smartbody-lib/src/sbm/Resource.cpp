#include "Resource.h"

#include <sstream>

Resource::Resource()
{
}

Resource::~Resource()
{
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
	return stream.str();
}

SeqResource::SeqResource()
{
}

std::string SeqResource::dump()
{
	std::stringstream stream;
	stream << "Seq: " << m_filePath;
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
	return stream.str();
}


