#ifndef DEFAULT_ATTRIBUTE_TABLE_H
#define DEFAULT_ATTRIBUTE_TABLE_H

#include <sbm/SBAttribute.h>
#include <vector>
#include <map>
#include <string>

class VariablePointer
{
public:
	enum { TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_DOUBLE, TYPE_STRING, TYPE_VEC3, TYPE_MATRIX };
	int varType;
	void* varPtr;
public:
	VariablePointer& operator= (const VariablePointer& rt);
	void updateVariableFromAttribute(SmartBody::SBAttribute* attr);
};

typedef std::pair<SmartBody::SBAttribute*,VariablePointer> AttributeVarPair;

class DefaultAttributeTable
{
protected:
	std::vector<AttributeVarPair> _defaultAttributes;	
public:
	DefaultAttributeTable(void);
	~DefaultAttributeTable(void);

	void addDefaultAttributeDouble(const std::string& name, double defaultValue, double* varPtr = NULL);
	void addDefaultAttributeFloat(const std::string& name, float defaultValue, float* varPtr = NULL);
	void addDefaultAttributeInt(const std::string& name, int defaultValue, int* varPtr = NULL);
	void addDefaultAttributeBool(const std::string& name, bool defaultValue, bool* varPtr = NULL);
	void addDefaultAttributeString(const std::string& name, const std::string& defaultValue, std::string* varPtr = NULL);
	void addDefaultAttributeVec3(const std::string&name, SrVec& defaultValue, SrVec* varPtr = NULL);
	void addDefaultAttributeMatrix(const std::string& name, SrMat& defaultValue, SrMat* varPtr = NULL);

	std::vector<AttributeVarPair>& getDefaultAttributes();	
};

#endif
