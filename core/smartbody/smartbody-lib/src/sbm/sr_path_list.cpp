#include "sr_path_list.h"
#include <sstream>
#include <boost/filesystem.hpp>


srPathList::srPathList()
{
	_curIndex = 0;
}

srPathList::~srPathList()
{
}

void srPathList::setPathPrefix(std::string pre)
{
	_prefix = pre;
}

std::string srPathList::getPathPrefix()
{
	return _prefix;
}
		
bool srPathList::insert(std::string path)
{
	for (size_t x = 0; x < _paths.size(); x++)
	{
		if (_paths[x] == path)
			return false;
	}

	_paths.push_back(path);
	return true;	
}

bool srPathList::remove(std::string path)
{
	for (std::vector<std::string>::iterator iter = _paths.begin();
		iter != _paths.end();
		iter++)
	{
		if ((*iter) == path)
		{
			_paths.erase(iter);
			return true;
		}
	}

	return false;	
}
		
void srPathList::reset()
{	
	_curIndex = 0;
}
		
std::string srPathList::next_path( void )
{
	if (size_t(_curIndex) >= _paths.size())
		return "";

	std::stringstream strstr;
	// if the path is an absolute path, don't prepend the media path
	const boost::filesystem::path p = _paths[_curIndex];
	std::string mediaPath = getPathPrefix();
	if (mediaPath.size() > 0 && p.has_relative_path())
		strstr << getPathPrefix() << "\\";
	strstr << p.string();

	boost::filesystem::path finalPath(strstr.str());
	finalPath.canonize();

	_curIndex++;
	
	return finalPath.string();
}

std::string srPathList::next_filename(char *buffer, const char *name)
{
	std::string path = next_path();
	if( path.size() > 0 )	{
		std::stringstream strstr;
		strstr << path << "/" << name;
		return( strstr.str() );
	}
	return( "" );
}
