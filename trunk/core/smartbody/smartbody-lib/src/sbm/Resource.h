#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <list>

class Resource
{
	public:
		Resource();
		virtual ~Resource();

		void addChild(Resource* resource);
		int getNumChildren();
		Resource* getChild(unsigned int num);
		Resource* getParent();
		void setParent(Resource* p);
		void setChildrenLimit(int l);

		virtual std::string dump() = 0;

	protected:
		std::list<Resource*> children;
		Resource* parent;
		int children_limit;		// limit of children number for a resource parant
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

		void setMediaPath(std::string mp);
		std::string getMediaPath();
		
		std::string dump();
	protected:
		std::string path;
		std::string type;
		std::string mediaPath;
};

class CmdResource : public Resource			// Cmd resources
{
	public:
		CmdResource();
		~CmdResource();

		int getCommandIndex();

		void setId(std::string str);
		std::string getId();

		void setCommand(std::string c);
		std::string getCommand();

		void setTime(double t);
		double getTime();
	
		std::string dump();
	
	protected:
		std::string id;
		std::string command;
		double time;
		int index;

		static int commandIndex;

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

class SkeletonResource : public Resource
{
	public:
		SkeletonResource();
		~SkeletonResource();

		void setSkeletonFile(std::string motion);
		std::string getSkeletonFile();

		void setType(std::string t);
		std::string getType();
		
		std::string dump();

	protected:
		std::string type;
		std::string skeletonFile;
};

class ControllerResource : public Resource		
{
	public:
		ControllerResource();
		~ControllerResource();

		void setControllerName(std::string cname);
		std::string getControllerName();

		void setType(std::string t);
		std::string getType();

		void setTime(double t);
		double getTime();

		std::string dump();

	protected:
		std::string controllerName;
		std::string type;
		double time;
};


#endif
