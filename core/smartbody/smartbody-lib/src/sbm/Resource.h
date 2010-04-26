#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <vector>

class Resource
{
	public:
		Resource();
		~Resource();

		void addChild(Resource* resource);
		int getNumChildren();
		Resource* getChild(unsigned int num);
		Resource* getParent();
		void setParent(Resource* p);

		virtual std::string dump() = 0;

	protected:
		std::vector<Resource*> children;
		Resource* parent;
};

class FileResource : public Resource		// Seq file resources
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

class PathResource : public Resource		// Path resources with type [ME/SEQ] specified
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

class CmdResource : public Resource			// Cmd resources
{
	public:
		CmdResource();
		~CmdResource();

		void setId(std::string str);
		std::string getId();

		void setCommand(std::string c);
		std::string getCommand();

	
		std::string dump();
	
	protected:
		std::string id;
		std::string command;

};

class MotionResource : public Resource		// General motion file resources with [SKM/SKP] type specified
{
	public:
		MotionResource();
		~MotionResource();

		void setMotionFile(std::string motion);
		std::string getMotionFile();

		void setType(std::string t);
		std::string getType();
		
		std::string dump();

	protected:
		std::string type;
		std::string motionFile;
};


#endif
