#ifndef DATTRIBUTE_H
#define DATTRIBUTE_H

#include "DSubject.h"
#include <string>
#include <vector>
#include <limits>
#include <sr/sr_vec.h>
#include <sr/sr_mat.h>

class DObject;
class DAttributeInfo;

class DAttribute : public DSubject
{
	public:
		DAttribute(std::string name);	
		DAttribute();
		virtual ~DAttribute();
		void setName(const std::string& name);
		const std::string& getName();
		virtual std::string write() = 0;
		virtual void read() = 0;
		DAttributeInfo* getAttributeInfo();

		void setObject(DObject* object);
		DObject* getObject();

		virtual DAttribute* copy() = 0;


	protected:
		std::string m_name;
		DAttributeInfo* m_info;
		DObject* m_object;
};

class DAttributeGroup 
{
	public:
		DAttributeGroup(std::string name);
		~DAttributeGroup();

		const std::string& getName();
		void setPriority(int val);
		int getPriority();

	protected:
		std::string m_name;
		int m_priority;
};


class DAttributeInfo
{
	public:
		DAttributeInfo();
		~DAttributeInfo();

		void setPriority(int val);
		int getPriority();

		void setReadOnly(bool val);
		bool getReadOnly();
		void setLocked(bool val);
		bool getLocked();
		void setHidden(bool val);
		bool getHidden();
		void setGroup(DAttributeGroup* group);
		DAttributeGroup* getGroup();
		void setGroup(std::string groupName);
		void setDescription(std::string description);
		std::string getDescription();

		std::string write();

		void setAttribute(DAttribute* attr);
		DAttribute* getAttribute();

	protected:
		bool m_readOnly;
		bool m_locked;
		int m_priority;
		bool m_hidden;
		DAttribute* m_attr;
		DAttributeGroup* m_group;
		std::string m_description;
};

class BoolAttribute : public DAttribute
{
	public:
		BoolAttribute();
		BoolAttribute(std::string name, bool val = true);
		~BoolAttribute();

		bool getValue(float time = -1.0);
		void setValue(bool val, float time = -1.0);
		void setDefaultValue(bool defaultVal);
		bool getDefaultValue();

		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		bool m_value;
		bool m_defaultValue;
};

class IntAttribute : public DAttribute
{
	public:
		IntAttribute();
		IntAttribute(std::string name, int val = 0, int min = -std::numeric_limits<int>::max(), int max = std::numeric_limits<int>::max());
		~IntAttribute();

		int getValue(float time = -1.0);
		void setValue(int val, float time = -1.0);
		void setDefaultValue(int defaultVal);
		int getDefaultValue();
		int getMin();
		int getMax();
		void setMin(int val);
		void setMax(int val);
		
		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		int m_value;
		int m_defaultValue;
		int m_min;
		int m_max;
};

class DoubleAttribute : public DAttribute
{
	public:
		DoubleAttribute();
		DoubleAttribute(std::string name, double val = 0, double min = -std::numeric_limits<double>::max(), double max = std::numeric_limits<double>::max());
		~DoubleAttribute();

		double getValue(float time = -1.0);
		void setValue(double val, float time = -1.0);
		void setDefaultValue(double defaultVal);
		double getDefaultValue();
		double getMin();
		double getMax();
		void setMin(double val);
		void setMax(double val);

		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		double m_value;
		double m_defaultValue;
		double m_min;
		double m_max;
};

class StringAttribute : public DAttribute
{
	public:
		StringAttribute();
		StringAttribute(std::string name, std::string value = "");
		~StringAttribute();

		const std::string& getValue(float time = -1.0);
		void setValue(const std::string& val, float time = -1.0);
		void setDefaultValue(const std::string& defaultVal);
		const std::string& getDefaultValue();
		void setValidValues(std::vector<std::string> values);
		std::vector<std::string>& getValidValues();

		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		std::string m_value;
		std::string m_defaultValue;
		std::vector<std::string> m_validValues;
};

class Vec3Attribute : public DAttribute
{
	public:
		Vec3Attribute();
		Vec3Attribute(std::string name);
		~Vec3Attribute();

		SrVec getValue(float time = -1.0);
		void setValue(SrVec val, float time = -1.0);
		void setDefaultValue(SrVec defaultVal);
		SrVec getDefaultValue();

		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		SrVec m_value;
		SrVec m_defaultValue;
};

class MatrixAttribute : public DAttribute
{
	public:
		MatrixAttribute();
		MatrixAttribute(std::string name);
		~MatrixAttribute();

		SrMat& getValue(float time = -1.0);
		void setValue(SrMat& matrix, float time = -1.0);
		void setDefaultValue(SrMat& matrix);
		SrMat& getDefaultValue();

		virtual std::string write();
		virtual void read();
		virtual DAttribute* copy();

	private:
		SrMat m_value;
		SrMat m_defaultValue;
};


#endif