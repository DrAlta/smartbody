#include "BMLSpeechObject.h"
#include <sstream>

BMLSpeechObject::BMLSpeechObject() : BMLObject()
{
	setName("speech");

	typeAttr = createStringAttribute("type", "text/plain", "", "Basic", 10, false, false, false, "Format of speech content.");
#ifndef __APPLE__
	std::vector<std::string> types;
	types.push_back("text/plain");
	types.push_back("application/ssml+xml");
	typeAttr->setValidValues(types);
#endif

	refAttr = createStringAttribute("ref", "", "", "Basic", 100, false, false, false, "Reference .xml file that contains speech information. Either set this attribute, or enter the content.");
	contentAttr = createStringAttribute("content", "", "", "Basic", 110, false, false, false, "Content of speech, such as 'hello, my name is John'. When using ssml, tags (such as <emphasis>) can be used.");
}

BMLSpeechObject::~BMLSpeechObject()
{
}

std::string BMLSpeechObject::getBML()
{
	std::stringstream strstr;

	strstr << "<" << getName();
	strstr << " type=\"" << typeAttr->getValue() << "\"";
	if (refAttr->getValue() != "")
	{
		strstr << " ref=\"" << refAttr->getValue() << "\"";
	}
	strstr << ">";
	if (contentAttr->getValue() != "")
	{
		strstr << contentAttr->getValue();
	}
	strstr << "</" << getName() << ">";

	return strstr.str();
	
}

void BMLSpeechObject::notify(SBSubject* subject)
{
	BMLObject::notify(subject);
}
