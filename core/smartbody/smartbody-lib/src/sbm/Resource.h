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

		void setFilePath(const std::string& path);
		const std::string& getFilePath();
		
		std::string dump();
	protected:
		std::string m_filePath;
};

class PathResource : public Resource		// Path resources with type [ME/SEQ] specified
{
	public:
		PathResource();
		~PathResource();

		void setType(const std::string& type);
		const std::string& getType();

		void setPath(const std::string& p);
		const std::string& getPath();
		
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

		void setMotionFile(const std::string& motion);
		const std::string& getMotionFile();

		void setType(const std::string& t);
		const std::string& getType();
		
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

		void setSkeletonFile(const std::string& motion);
		const std::string& getSkeletonFile();

		void setType(const std::string& t);
		const std::string&getType();
		
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

		void setControllerName(const std::string& cname);
		const std::string& getControllerName();

		void setType(const std::string& t);
		const std::string& getType();

		void setTime(double t);
		double getTime();

		std::string dump();

	protected:
		std::string controllerName;
		std::string type;
		double time;
};


#endif
