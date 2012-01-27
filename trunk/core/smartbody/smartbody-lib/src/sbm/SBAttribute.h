#ifndef _SBATTRIBUTE_H
#define _SBATTRIBUTE_H

#include "SBSubject.h"
#include <string>
#include <vector>
#include <limits>
#include <sr/sr_vec.h>
#include <sr/sr_mat.h>

namespace SmartBody {

class SBObject;
class SBAttributeInfo;

class SBAttribute : public SBSubject
{
	public:
		SBAttribute(const std::string& name);	
		SBAttribute();
		virtual ~SBAttribute();
		void setName(const std::string& name);
		const std::string& getName();
		virtual std::string write();
		virtual void read();
		SBAttributeInfo* getAttributeInfo();

		void setObject(SBObject* object);
		SBObject* getObject();

		virtual SBAttribute* copy();


	protected:
		std::string m_name;
		SBAttributeInfo* m_info;
		SBObject* m_object;
};

class SBAttributeGroup 
{
	public:
		SBAttributeGroup(const std::string& name);
		~SBAttributeGroup();

		const std::string& getName();
		void setPriority(int val);
		int getPriority();

	protected:
		std::string m_name;
		int m_priority;
};


class SBAttributeInfo
{
	public:
		SBAttributeInfo();
		~SBAttributeInfo();

		void setPriority(int val);
		int getPriority();

		void setReadOnly(bool val);
		bool getReadOnly();
		void setLocked(bool val);
		bool getLocked();
		void setHidden(bool val);
		bool getHidden();
		void setGroup(SBAttributeGroup* group);
		SBAttributeGroup* getGroup();
		void setGroup(const std::string& groupName);
		void setDescription(const std::string& description);
		std::string getDescription();

		std::string write();

		void setAttribute(SBAttribute* attr);
		SBAttribute* getAttribute();

	protected:
		bool m_readOnly;
		bool m_locked;
		int m_priority;
		bool m_hidden;
		SBAttribute* m_attr;
		SBAttributeGroup* m_group;
		std::string m_description;
};

class BoolAttribute : public SBAttribute
{
	public:
		BoolAttribute();
		BoolAttribute(const std::string& name, bool val = true);
		~BoolAttribute();

		const bool& getValue();
		void setValue(const bool& val);
		void setDefaultValue(const bool& defaultVal);
		const bool& getDefaultValue();
		void setValueFast(const bool& val);

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		bool m_value;
		bool m_defaultValue;
};

class IntAttribute : public SBAttribute
{
	public:
		IntAttribute();
		IntAttribute(const std::string& name, int val = 0, int min = -std::numeric_limits<int>::max(), int max = std::numeric_limits<int>::max());
		~IntAttribute();

		const int& getValue();
		void setValue(const int& val);
		void setDefaultValue(const int& defaultVal);
		const int& getDefaultValue();
		void setValueFast(const int& val);
		int getMin();
		int getMax();
		void setMin(int val);
		void setMax(int val);
		
		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		int m_value;
		int m_defaultValue;
		int m_min;
		int m_max;
};

class DoubleAttribute : public SBAttribute
{
	public:
		DoubleAttribute();
		DoubleAttribute(const std::string& name, double val = 0, double min = -std::numeric_limits<double>::max(), double max = std::numeric_limits<double>::max());
		~DoubleAttribute();

		const double& getValue();
		void setValue(const double& val);
		void setDefaultValue(const double& defaultVal);
		const double& getDefaultValue();
		void setValueFast(const double& val);
		double getMin();
		double getMax();
		void setMin(double val);
		void setMax(double val);

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		double m_value;
		double m_defaultValue;
		double m_min;
		double m_max;
};

class StringAttribute : public SBAttribute
{
	public:
		StringAttribute();
		StringAttribute(const std::string& name, std::string value = "");
		~StringAttribute();

		const std::string& getValue();
		void setValue(const std::string& val);
		void setDefaultValue(const std::string& defaultVal);
		const std::string& getDefaultValue();
		void setValidValues(const std::vector<std::string>& values);
		const std::vector<std::string>& getValidValues();
		void setValueFast(const std::string& val);

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		std::string m_value;
		std::string m_defaultValue;
		std::vector<std::string> m_validValues;
};

class Vec3Attribute : public SBAttribute
{
	public:
		Vec3Attribute();
		Vec3Attribute(const std::string& name);
		~Vec3Attribute();

		const SrVec& getValue();
		void setValue(const SrVec& val);
		void setDefaultValue(const SrVec& defaultVal);
		const SrVec& getDefaultValue();
		void setValueFast(const SrVec& val);

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		SrVec m_value;
		SrVec m_defaultValue;
};

class MatrixAttribute : public SBAttribute
{
	public:
		MatrixAttribute();
		MatrixAttribute(const std::string& name);
		~MatrixAttribute();

		const SrMat& getValue();
		void setValue(const SrMat& matrix);
		void setDefaultValue(const SrMat& matrix);
		const SrMat& getDefaultValue();
		void setValueFast(const SrMat& val);

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();

	private:
		SrMat m_value;
		SrMat m_defaultValue;
};

class ActionAttribute : public SBAttribute
{
	public:
		ActionAttribute();
		ActionAttribute(const std::string& name);
		~ActionAttribute();

		void setValue();

		virtual std::string write();
		virtual void read();
		virtual SBAttribute* copy();		
};

};


#endif