#ifndef _DOBJECT_H_
#define _DOBJECT_H_

#include <map>
#include <string>
#include "DAttribute.h"
#include "DAttributeManager.h"
#include "DObserver.h"

class DObject : public DObserver, public DSubject
{
	public:
		DObject();
		~DObject();

		void setName(const std::string& name);
		const std::string& getName();
		DAttribute* getAttribute(const std::string& attrName);
		std::map<std::string, DAttribute*>& getAttributeList();
		DAttributeManager* getAttributeManager();
		void addAttribute(DAttribute* attr);
		void addAttribute(DAttribute* attr, const std::string& groupName);
		bool removeAttribute(const std::string& name);
		void clearAttributes();
		int getNumAttributes();
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

		void setBoolAttribute(const std::string& name, bool value);
		void setIntAttribute(const std::string& name, int value);
		void setDoubleAttribute(const std::string& name, double value);
		void setVec3Attribute(const std::string& name, float val1, float val2, float val3);
		void setStringAttribute(const std::string& name, std::string value);
		void setMatrixAttribute(const std::string& name, SrMat& value);

		const bool& getBoolAttribute(const std::string& name) ;
		const int&  getIntAttribute(const std::string& name) ;
		const double& getDoubleAttribute(const std::string& name) ;
		const SrVec& getVec3Attribute(const std::string& name) ;
		const std::string& getStringAttribute(const std::string& name) ;
		const SrMat& getMatrixAttribute(const std::string& name) ;

		virtual void notify(DSubject* subject);


	protected:
		std::string m_name;
		DAttributeManager* m_attributeManager;
		std::map<std::string, DAttribute*> m_attributeList;
		std::string m_emptyString;

		static bool defaultBool;
		static int defaultInt;
		static double defaultDouble;
		static SrVec defaultVec;
		static std::string defaultString;
		static SrMat defaultMatrix;
};
#endif