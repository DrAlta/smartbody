#include "DefaultAttributeTable.h"

DefaultAttributeTable::DefaultAttributeTable(void)
{
}

DefaultAttributeTable::~DefaultAttributeTable(void)
{
}

void DefaultAttributeTable::addDefaultAttributeDouble( std::string name, double defaultValue, double* varPtr )
{
	DoubleAttribute* hf = new DoubleAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_DOUBLE;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeFloat( std::string name, float defaultValue, float* varPtr )
{
	DoubleAttribute* hf = new DoubleAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_FLOAT;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeInt( std::string name, int defaultValue, int* varPtr )
{
	IntAttribute* hf = new IntAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_INT;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeBool( std::string name, bool defaultValue, bool* varPtr )
{
	BoolAttribute* hf = new BoolAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_BOOL;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeString( std::string name, std::string defaultValue, std::string* varPtr )
{
	StringAttribute* hf = new StringAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_STRING;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeVec3( std::string name, SrVec& defaultValue, SrVec* varPtr )
{
	Vec3Attribute* hf = new Vec3Attribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_VEC3;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

void DefaultAttributeTable::addDefaultAttributeMatrix( std::string name, SrMat& defaultValue, SrMat* varPtr )
{
	MatrixAttribute* hf = new MatrixAttribute(name);
	hf->setDefaultValue(defaultValue);
	hf->setValue(defaultValue);
	VariablePointer var;
	var.varType = VariablePointer::TYPE_MATRIX;
	var.varPtr  = varPtr;
	_defaultAttributes.push_back(AttributeVarPair(hf,var));	
}

std::vector<AttributeVarPair>& DefaultAttributeTable::getDefaultAttributes()
{
		return _defaultAttributes;
}

VariablePointer& VariablePointer::operator=( const VariablePointer& rt )
{
	varType = rt.varType;
	varPtr  = rt.varPtr;
	return *this;
}

void VariablePointer::updateVariableFromAttribute( DAttribute* attr )
{
	if (!varPtr) return;

	if (varType == TYPE_BOOL)
	{
		BoolAttribute* boolAttr = dynamic_cast<BoolAttribute*>(attr);
		if (!boolAttr) return; 
		bool* boolPtr = (bool*)varPtr; 
		*boolPtr = boolAttr->getValue();
	}
	else if (varType == TYPE_INT)
	{
		IntAttribute* intAttr = dynamic_cast<IntAttribute*>(attr);
		if (!intAttr) return; 
		int* intPtr = (int*)varPtr; 
		*intPtr = intAttr->getValue();
	}
	else if (varType == TYPE_FLOAT)
	{
		DoubleAttribute* doubleAttr = dynamic_cast<DoubleAttribute*>(attr);
		if (!doubleAttr) return; 
		float* floatPtr = (float*)varPtr; 
		*floatPtr = (float)doubleAttr->getValue();
	}	
	else if (varType == TYPE_DOUBLE)
	{
		DoubleAttribute* doubleAttr = dynamic_cast<DoubleAttribute*>(attr);
		if (!doubleAttr) return; 
		double* doublePtr = (double*)varPtr; 
		*doublePtr = doubleAttr->getValue();
	}
	else if (varType == TYPE_STRING)
	{
		StringAttribute* stringAttr = dynamic_cast<StringAttribute*>(attr);
		if (!stringAttr) return; 
		std::string* stringPtr = (std::string*)varPtr; 
		*stringPtr = stringAttr->getValue();
	}
	else if (varType == TYPE_VEC3)
	{
		Vec3Attribute* vec3Attr = dynamic_cast<Vec3Attribute*>(attr);
		if (!vec3Attr) return; 
		SrVec* vec3Ptr = (SrVec*)varPtr; 
		*vec3Ptr = vec3Attr->getValue();
	}
	else if (varType == TYPE_MATRIX)
	{
		MatrixAttribute* matrixAttr = dynamic_cast<MatrixAttribute*>(attr);
		if (!matrixAttr) return; 
		SrMat* matPtr = (SrMat*)varPtr; 
		*matPtr = matrixAttr->getValue();
	}	
}