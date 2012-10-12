#ifndef _SBOBJECT_H_
#define _SBOBJECT_H_

#include <map>
#include <string>
#include <vector>
#include "SBObserver.h"
#include "SBSubject.h"
#include <sr/sr_mat.h>

namespace SmartBody {

class SBAttribute;
class ActionAttribute;
class BoolAttribute;
class IntAttribute;
class StringAttribute;
class DoubleAttribute;
class Vec3Attribute;
class MatrixAttribute;
class SBAttributeManager;

class SBObject : public SBObserver, public SBSubject
{
	public:
		SBObject();
		~SBObject();

		virtual void start();
		virtual void beforeUpdate(double time);
		virtual void update(double time);
		virtual void afterUpdate(double time);
		virtual void stop();

		virtual void setName(const std::string& name);
		virtual const std::string& getName();
		bool hasAttribute(const std::string& attrName);
		SBAttribute* getAttribute(const std::string& attrName);
		std::map<std::string, SBAttribute*>& getAttributeList();
		SBAttributeManager* getAttributeManager();
		void addAttribute(SBAttribute* attr);
		void addAttribute(SBAttribute* attr, const std::string& groupName);
		bool removeAttribute(const std::string& name);
		void clearAttributes();
		int getNumAttributes();

		std::vector<std::string> getAttributeNames();

		BoolAttribute* createBoolAttribute(const std::string& name, bool value, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");
		IntAttribute* createIntAttribute(const std::string& name, int value, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");
		DoubleAttribute* createDoubleAttribute(const std::string& name, double value, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");
		Vec3Attribute* createVec3Attribute(const std::string& name, float val1, float val2, float val3, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");

		StringAttribute* createStringAttribute(const std::string& name, const std::string& value, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");
		MatrixAttribute* createMatrixAttribute(const std::string& name, SrMat& value, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");
		ActionAttribute* createActionAttribute(const std::string& name, bool notifySelf, const std::string& groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description = "");

		void setBoolAttribute(const std::string& name, bool value);
		void setIntAttribute(const std::string& name, int value);
		void setDoubleAttribute(const std::string& name, double value);
		void setVec3Attribute(const std::string& name, float val1, float val2, float val3);
		void setStringAttribute(const std::string& name, std::string value);
		void setMatrixAttribute(const std::string& name, SrMat& value);
		void setActionAttribute(const std::string& name);

		const bool& getBoolAttribute(const std::string& name) ;
		const int&  getIntAttribute(const std::string& name) ;
		const double& getDoubleAttribute(const std::string& name) ;
		const SrVec& getVec3Attribute(const std::string& name) ;
		const std::string& getStringAttribute(const std::string& name) ;
		const SrMat& getMatrixAttribute(const std::string& name) ;

		virtual void notify(SBSubject* subject);


	protected:
		std::string m_name;
		SBAttributeManager* m_attributeManager;
		std::map<std::string, SBAttribute*> m_attributeList;
		std::string m_emptyString;

		static bool defaultBool;
		static int defaultInt;
		static double defaultDouble;
		static SrVec defaultVec;
		static std::string defaultString;
		static SrMat defaultMatrix;
};

};

#endif