#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

class Resource
{
	public:
		Resource();
		~Resource();

		virtual std::string dump() = 0;
};

class FileResource : public Resource
{
	public:
		FileResource();
		~FileResource();

		void setFilePath(std::string path);
		std::string getFilePath();
		
		std::string dump();
	protected:
		std::string m_filePath;
};

class SeqResource : public FileResource
{
	public:
		SeqResource();

		std::string dump();
};

class PathResource : public Resource
{
	public:
		PathResource();
		~PathResource();

		void setType(std::string type);
		std::string getType();

		void setPath(std::string p);
		std::string getPath();
		
		std::string dump();
	protected:
		std::string path;
		std::string type;
};




#endif
