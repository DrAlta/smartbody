#include "BMLObject.h"
#include <sstream>

BMLObject::BMLObject()
{
	createStringAttribute("id", "", true, "Basic", 1000, false, false, false, 
		"Id of this BML request");
}

BMLObject::~BMLObject()
{
}

void BMLObject::constructBML()
{
	std::stringstream strstr;

	strstr << "<" << getName();

	int numUsedElements = 0;
	std::map<std::string, DAttribute*>& attributes = getAttributeList();
	for (std::map<std::string, DAttribute*>::iterator iter = attributes.begin();
		 iter != attributes.end();
		 iter++)
	{
		DAttribute* attribute = (*iter).second;

		BoolAttribute* boolAttribute = dynamic_cast<BoolAttribute*>(attribute);
		if (boolAttribute)
		{
			if (boolAttribute->getValue() != boolAttribute->getDefaultValue())
			{
				strstr << " " << boolAttribute->getName() << "=\"" << (boolAttribute->getValue()? "true" : "false") << "\"";
				numUsedElements++;
			}
		}
		
		IntAttribute* intAttribute = dynamic_cast<IntAttribute*>(attribute);
		if (intAttribute)
		{
			if (intAttribute->getValue() != intAttribute->getDefaultValue())
			{
				strstr << " " << intAttribute->getName() << "=\"" << intAttribute->getValue() << "\"";
				numUsedElements++;
			}
		}

		DoubleAttribute* doubleAttribute = dynamic_cast<DoubleAttribute*>(attribute);
		if (doubleAttribute)
		{
			if (doubleAttribute->getValue() != doubleAttribute->getDefaultValue())
			{
				strstr << " " << doubleAttribute->getName() << "=\"" << doubleAttribute->getValue()<< "\"";
				numUsedElements++;
			}
		}

		Vec3Attribute* vecAttribute = dynamic_cast<Vec3Attribute*>(attribute);
		if (vecAttribute)
		{
			if (vecAttribute->getValue() != vecAttribute->getDefaultValue())
			{
				strstr << " " << doubleAttribute->getName() << "=\"" << vecAttribute->getValue()[0] << " " << vecAttribute->getValue()[1] << " " << vecAttribute->getValue()[1] << "\"";
				numUsedElements++;
			}
		}

		StringAttribute* stringAttribute = dynamic_cast<StringAttribute*>(attribute);
		if (stringAttribute)
		{
			if (stringAttribute->getValue() != stringAttribute->getDefaultValue())
			{
				strstr << " " << stringAttribute->getName() << "=\"" << stringAttribute->getValue()<< "\"";
				numUsedElements++;
			}
		}
	}

	strstr << "/>";
	if (numUsedElements > 0)
		_bml = strstr.str();
	else
		_bml = "";
}

std::string BMLObject::getBML()
{
	return _bml;
}

void BMLObject::notify(DSubject* subject)
{
	DAttribute* attribute = dynamic_cast<DAttribute*>(subject);
	if (attribute)
	{
		constructBML();
		notifyObservers();
	}
}
