#ifndef DEFAULT_ATTRIBUTE_TABLE_H
#define DEFAULT_ATTRIBUTE_TABLE_H

#include <sbm/DAttribute.h>
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
	void updateVariableFromAttribute(DAttribute* attr);
};

typedef std::pair<DAttribute*,VariablePointer> AttributeVarPair;

class DefaultAttributeTable
{
protected:
	std::vector<AttributeVarPair> _defaultAttributes;	
public:
	DefaultAttributeTable(void);
	~DefaultAttributeTable(void);

	void addDefaultAttributeDouble(std::string name, double defaultValue, double* varPtr = NULL);
	void addDefaultAttributeFloat(std::string name, float defaultValue, float* varPtr = NULL);
	void addDefaultAttributeInt(std::string name, int defaultValue, int* varPtr = NULL);
	void addDefaultAttributeBool(std::string name, bool defaultValue, bool* varPtr = NULL);
	void addDefaultAttributeString(std::string name, std::string defaultValue, std::string* varPtr = NULL);
	void addDefaultAttributeVec3(std::string name, SrVec& defaultValue, SrVec* varPtr = NULL);
	void addDefaultAttributeMatrix(std::string name, SrMat& defaultValue, SrMat* varPtr = NULL);

	std::vector<AttributeVarPair>& getDefaultAttributes();	
};

#endif
