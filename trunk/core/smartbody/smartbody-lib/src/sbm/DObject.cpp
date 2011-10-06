#include "DObject.h"
#include <sr/sr_vec.h>

bool DObject::defaultBool = true;
int DObject::defaultInt = 0;
double DObject::defaultDouble = 0.0;
std::string DObject::defaultString = "";
SrVec DObject::defaultVec = SrVec();
SrMat DObject::defaultMatrix = SrMat();

DObject::DObject() : DSubject(), DObserver()
{
	m_attributeManager = new DAttributeManager();
}

DObject::~DObject()
{
	for (std::map<std::string, DAttribute*>::iterator iter = m_attributeList.begin();
		 iter != m_attributeList.end();
		 iter++)
	{
		delete (*iter).second;
	}
	delete m_attributeManager;

}

void DObject::setName(const std::string& name)
{
	m_name = name;
}


const std::string& DObject::getName()
{
	return m_name;
}


void DObject::addAttribute(DAttribute* attr)
{
	 // check for the existence of the attribute
	std::map<std::string, DAttribute*>::iterator iter = m_attributeList.find(attr->getName());
	if (iter != m_attributeList.end()) // attribute exists, remove the old attribute 
	{
		DAttribute* attr = iter->second;

		// notify the attribute manager of the change
		this->getAttributeManager()->notifyRemoveAttribute(attr);

		m_attributeList.erase(iter);
		delete attr;
	}

	m_attributeList[attr->getName()] = attr;
	attr->setObject(this);
	// notify the attribute manager of the change
	this->getAttributeManager()->notifyCreateAttribute(attr);
}

void DObject::addAttribute( DAttribute* attr, const std::string& groupName )
{
	// check for the existence of the attribute
	std::map<std::string, DAttribute*>::iterator iter = m_attributeList.find(attr->getName());
	if (iter != m_attributeList.end()) // attribute exists, remove the old attribute 
	{
		DAttribute* attr = iter->second;

		// notify the attribute manager of the change
		this->getAttributeManager()->notifyRemoveAttribute(attr);

		m_attributeList.erase(iter);
		delete attr;
	}

	m_attributeList[attr->getName()] = attr;
	DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	attr->getAttributeInfo()->setGroup(group);
	attr->setObject(this);
	// notify the attribute manager of the change
	this->getAttributeManager()->notifyCreateAttribute(attr);
}


 DAttribute* DObject::getAttribute(const std::string& name)
 {
	std::map<std::string, DAttribute*>::iterator iter = m_attributeList.find(name);
	if (iter != m_attributeList.end()) // attribute exists, remove the old attribute 
	{
		return iter->second;
	}
	else
	{
		return NULL;
	}
 }

 void DObject::clearAttributes()
 {
	 std::map<std::string, DAttribute*>::iterator iter = m_attributeList.begin();
	 std::vector<std::string> attrNameList;
	 for ( iter  = m_attributeList.begin();
		   iter != m_attributeList.end();
		   iter++)
	 {
		 attrNameList.push_back(iter->first);
	 }		 
	 for (unsigned int i=0;i<attrNameList.size();i++)
		 removeAttribute(attrNameList[i]);
 }
 
int DObject::getNumAttributes()
{
	return m_attributeList.size();
}


 bool DObject::removeAttribute(const std::string& name)
 {
	// check for the existence of the attribute
	std::map<std::string, DAttribute*>::iterator iter = m_attributeList.find(name);
	if (iter != m_attributeList.end()) // attribute exists, remove the old attribute 
	{

		DAttribute* attr = iter->second;
	
		// notify the attribute manager of the change
		this->getAttributeManager()->notifyRemoveAttribute(attr);
		
		m_attributeList.erase(iter);
		delete attr;
		
		return true;
	}

	return false;
 }

  BoolAttribute* DObject::createBoolAttribute(const std::string& name, bool value, bool notifySelf, const std::string& groupName, int priority, 
											  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 BoolAttribute* boolAttr = new BoolAttribute();
	 boolAttr->setName(name);
	 boolAttr->setValue(value);
	 boolAttr->setDefaultValue(value);
	 boolAttr->getAttributeInfo()->setPriority(priority);
	 boolAttr->getAttributeInfo()->setReadOnly(isReadOnly);
	 boolAttr->getAttributeInfo()->setLocked(isLocked);
	 boolAttr->getAttributeInfo()->setHidden(isHidden);
	 boolAttr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(boolAttr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 boolAttr->getAttributeInfo()->setGroup(group);

	 if (notifySelf)
		boolAttr->registerObserver(this);

	 return boolAttr;
}

IntAttribute* DObject::createIntAttribute(const std::string& name, int value, bool notifySelf, const std::string& groupName, int priority, 
												  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 IntAttribute* intAttr = new IntAttribute();
	 intAttr->setName(name);
	 intAttr->setValue(value);
	 intAttr->setDefaultValue(value);
	 intAttr->getAttributeInfo()->setPriority(priority);
	 intAttr->getAttributeInfo()->setReadOnly(isReadOnly);
	 intAttr->getAttributeInfo()->setLocked(isLocked);
	 intAttr->getAttributeInfo()->setHidden(isHidden);
	 intAttr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(intAttr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 intAttr->getAttributeInfo()->setGroup(group);

	 if (notifySelf)
		intAttr->registerObserver(this);

	  return intAttr;
}

 DoubleAttribute* DObject::createDoubleAttribute(const std::string& name, double value, bool notifySelf, const std::string& groupName, int priority, 
												  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 DoubleAttribute* doubleAttr = new DoubleAttribute();
	 doubleAttr->setName(name);
	 doubleAttr->setValue(value);
	 doubleAttr->setDefaultValue(value);
	 doubleAttr->getAttributeInfo()->setPriority(priority);
	 doubleAttr->getAttributeInfo()->setReadOnly(isReadOnly);
	 doubleAttr->getAttributeInfo()->setLocked(isLocked);
	 doubleAttr->getAttributeInfo()->setHidden(isHidden);
	 doubleAttr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(doubleAttr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 doubleAttr->getAttributeInfo()->setGroup(group);

	 if (notifySelf)
		doubleAttr->registerObserver(this);

	  return doubleAttr;
}

 Vec3Attribute* DObject::createVec3Attribute(const std::string& name, float val1, float val2, float val3, bool notifySelf, const std::string& groupName, int priority, 
												  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 Vec3Attribute* vec3Attr = new Vec3Attribute();
	 vec3Attr->setName(name);
	 SrVec vec(val1, val2, val3);
	 vec3Attr->setValue(vec);
	 vec3Attr->setDefaultValue(vec);
	 vec3Attr->getAttributeInfo()->setPriority(priority);
	 vec3Attr->getAttributeInfo()->setReadOnly(isReadOnly);
	 vec3Attr->getAttributeInfo()->setLocked(isLocked);
	 vec3Attr->getAttributeInfo()->setHidden(isHidden);
	 vec3Attr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(vec3Attr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 vec3Attr->getAttributeInfo()->setGroup(group);

	 if (notifySelf)
		vec3Attr->registerObserver(this);

	  return vec3Attr;
}


 StringAttribute* DObject::createStringAttribute(const std::string& name, const std::string& value, bool notifySelf, const std::string& groupName, int priority, 
												  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 StringAttribute* strAttr = new StringAttribute();
	 strAttr->setName(name);
	 strAttr->setValue(value);
	 strAttr->setDefaultValue(value);
	 strAttr->getAttributeInfo()->setPriority(priority);
	 strAttr->getAttributeInfo()->setReadOnly(isReadOnly);
	 strAttr->getAttributeInfo()->setLocked(isLocked);
	 strAttr->getAttributeInfo()->setHidden(isHidden);
	 strAttr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(strAttr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 strAttr->getAttributeInfo()->setGroup(group);
	
	 if (notifySelf)
		strAttr->registerObserver(this);

	 return strAttr;
}

 MatrixAttribute* DObject::createMatrixAttribute(const std::string& name, SrMat& value, bool notifySelf, const std::string& groupName, int priority, 
												  bool isReadOnly, bool isLocked, bool isHidden, const std::string& description)
 {
	 MatrixAttribute* matrixAttr = new MatrixAttribute();
	 matrixAttr->setName(name);
	 matrixAttr->setValue(value);
	 matrixAttr->setDefaultValue(value);
	 matrixAttr->getAttributeInfo()->setPriority(priority);
	 matrixAttr->getAttributeInfo()->setReadOnly(isReadOnly);
	 matrixAttr->getAttributeInfo()->setLocked(isLocked);
	 matrixAttr->getAttributeInfo()->setHidden(isHidden);
	 matrixAttr->getAttributeInfo()->setDescription(description);

	 this->addAttribute(matrixAttr);

	 DAttributeGroup* group = this->getAttributeManager()->getGroup(groupName, true);
	 matrixAttr->getAttributeInfo()->setGroup(group);

	 if (notifySelf)
		matrixAttr->registerObserver(this);

	  return matrixAttr;
}

  std::map<std::string, DAttribute*>& DObject::getAttributeList()
 {
	return m_attributeList;
 }


  DAttributeManager* DObject::getAttributeManager()
{
	return m_attributeManager;
}

  void DObject::notify(DSubject* subject)
{
	
  }

  void DObject::setBoolAttribute( const std::string& name, bool value )
  {
	  DAttribute* attr = getAttribute(name);
	  BoolAttribute* battr = dynamic_cast<BoolAttribute*>(attr);
	  if (battr)
	  {
		  battr->setValue(value);
	  }
  }

  void DObject::setIntAttribute( const std::string& name, int value )
  {
	  DAttribute* attr = getAttribute(name);
	  IntAttribute* iattr = dynamic_cast<IntAttribute*>(attr);
	  if (iattr)
	  {
		  iattr->setValue(value);
	  }
  }

  void DObject::setDoubleAttribute( const std::string& name, double value )
  {
	  DAttribute* attr = getAttribute(name);
	  DoubleAttribute* dattr = dynamic_cast<DoubleAttribute*>(attr);
	  if (dattr)
	  {
		  dattr->setValue(value);
	  }
  }

  void DObject::setVec3Attribute( const std::string& name, float val1, float val2, float val3 )
  {
	  DAttribute* attr = getAttribute(name);
	  Vec3Attribute* vattr = dynamic_cast<Vec3Attribute*>(attr);
	  if (vattr)
	  {
		  vattr->setValue(SrVec(val1,val2,val3));
	  }
  }

  void DObject::setStringAttribute( const std::string& name, const std::string value )
  {
	  DAttribute* attr = getAttribute(name);
	  StringAttribute* sattr = dynamic_cast<StringAttribute*>(attr);
	  if (sattr)
	  {
		  sattr->setValue(value);
	  }
  }

  void DObject::setMatrixAttribute( const std::string& name, SrMat& value )
  {
	  DAttribute* attr = getAttribute(name);
	  MatrixAttribute* mattr = dynamic_cast<MatrixAttribute*>(attr);
	  if (mattr)
	  {
		  mattr->setValue(value);
	  }
  }

  const bool& DObject::getBoolAttribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  BoolAttribute* battr = dynamic_cast<BoolAttribute*>(attr);
	  if (battr)
	  {
		  return battr->getValue();
	  }	  
	  return defaultBool;
  }

  const int& DObject::getIntAttribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  IntAttribute* iattr = dynamic_cast<IntAttribute*>(attr);
	  if (iattr)
	  {
		  return iattr->getValue();
	  }
	  return defaultInt;
  }

  const double& DObject::getDoubleAttribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  DoubleAttribute* dattr = dynamic_cast<DoubleAttribute*>(attr);
	  if (dattr)
	  {
		 return dattr->getValue();
	  }
	  return defaultDouble;
  }

  const SrVec& DObject::getVec3Attribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  Vec3Attribute* vattr = dynamic_cast<Vec3Attribute*>(attr);
	  if (vattr)
	  {
		  return vattr->getValue();
	  }
	  return defaultVec;
  }

  const std::string& DObject::getStringAttribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  StringAttribute* sattr = dynamic_cast<StringAttribute*>(attr);
	  if (sattr)
	  {
		  return sattr->getValue();
	  }
	  return defaultString;
  }

  const SrMat& DObject::getMatrixAttribute( const std::string& name )
  {
	  DAttribute* attr = getAttribute(name);
	  MatrixAttribute* mattr = dynamic_cast<MatrixAttribute*>(attr);
	  if (mattr)
	  {
		  return mattr->getValue();
	  }
	  return defaultMatrix;
  }

  