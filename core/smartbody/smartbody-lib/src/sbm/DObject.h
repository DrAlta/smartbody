#ifndef _DOBJECT_H_
#define _DOBJECT_H_

#include <map>
#include <string>
#include "DAttribute.h"
#include "DAttributeManager.h"
#include "DObserver.h"

class DObject : public DObserver
{
	public:
		DObject();
		~DObject();

		void setName(const std::string& name);
		const std::string& getName();
		DAttribute* getAttribute(std::string attrName);
		std::map<std::string, DAttribute*>& getAttributeList();
		DAttributeManager* getAttributeManager();
		void addAttribute(DAttribute* attr);
		void addAttribute(DAttribute* attr, std::string groupName);
		bool removeAttribute(std::string name);
		void clearAttributes();
		int getNumAttributes();
		BoolAttribute* createBoolAttribute(std::string name, bool value, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");
		IntAttribute* createIntAttribute(std::string name, int value, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");
		DoubleAttribute* createDoubleAttribute(std::string name, double value, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");
		Vec3Attribute* createVec3Attribute(std::string name, float val1, float val2, float val3, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");

		StringAttribute* createStringAttribute(std::string name, std::string value, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");
		MatrixAttribute* createMatrixAttribute(std::string name, SrMat& value, bool notifySelf, std::string groupName, int priority, 
													  bool isReadOnly, bool isLocked, bool isHidden, std::string description = "");

		void setBoolAttribute(std::string name, bool value);
		void setIntAttribute(std::string name, int value);
		void setDoubleAttribute(std::string name, double value);
		void setVec3Attribute(std::string name, float val1, float val2, float val3);
		void setStringAttribute(std::string name, std::string value);
		void setMatrixAttribute(std::string name, SrMat& value);

		bool getBoolAttribute(std::string name) ;
		int  getIntAttribute(std::string name) ;
		double getDoubleAttribute(std::string name) ;
		SrVec getVec3Attribute(std::string name) ;
		std::string getStringAttribute(std::string name) ;
		SrMat getMatrixAttribute(std::string name) ;


		virtual void notify(DSubject* subject);

	protected:
		std::string m_name;
		DAttributeManager* m_attributeManager;
		std::map<std::string, DAttribute*> m_attributeList;
		std::string m_emptyString;
};
#endif