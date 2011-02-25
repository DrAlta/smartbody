/*
 *  ParserOpenCOLLADA.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Yuyu Xu, USC
 */

#include "ParserOpenCOLLADA.h"
#include "sr/sr_euler.h"


bool ParserOpenCOLLADA::parse(SkSkeleton& skeleton, SkMotion& motion, std::string fileName, float scale)
{
	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Error during initialization! :\n" << message << "\n";
		XMLString::release(&message);
		return false;
	}

	XercesDOMParser* parser = new XercesDOMParser();
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true);    // optional

	ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
	parser->setErrorHandler(errHandler);

	try 
	{
		parser->parse(fileName.c_str());
		xercesc_3_0::DOMDocument* doc = parser->getDocument();
		parseNode(skeleton, motion, doc, scale);
	}
	catch (const XMLException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return false;
	}
	catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return false;
	}
		catch (...) {
		std::cout << "Unexpected Exception \n" ;
		return false;
	}

	delete parser;
	delete errHandler;
	return true;
}

void ParserOpenCOLLADA::parseNode(SkSkeleton& skeleton, SkMotion& motion, xercesc_3_0::DOMNode* node, float scale)
{
	int type = node->getNodeType();
	std::string name = getString(node->getNodeName());
	std::string value = getString(node->getNodeValue());
	if (name == "library_visual_scenes" && node->getNodeType() ==  xercesc_3_0::DOMNode::ELEMENT_NODE)
	{
		parseLibraryVisualScenes(node, skeleton, motion, scale);
		skeleton.make_active_channels();
		skeleton.compress();
	}

	if (name == "library_animations" && node->getNodeType() ==  xercesc_3_0::DOMNode::ELEMENT_NODE)
		parseLibraryAnimations(node, skeleton, motion, scale);

	if (node->hasChildNodes())
	{
		  const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
		  for (unsigned int c = 0; c < list->getLength(); c++)
				parseNode(skeleton, motion, list->item(c), scale);
	}	
}

void ParserOpenCOLLADA::parseLibraryVisualScenes(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale)
{
	const xercesc_3_0::DOMNodeList* list1 = node->getChildNodes();
	for (unsigned int c = 0; c < list1->getLength(); c++)
	{
		xercesc_3_0::DOMNode* node1 = list1->item(c);
		std::string nodeName = getString(node1->getNodeName());
		if (nodeName == "visual_scene")
			parseJoints(node1, skeleton, motion, scale, NULL);
	}
}

void ParserOpenCOLLADA::parseJoints(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, SkJoint* parent)
{
	const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		xercesc_3_0::DOMNode* childNode = list->item(i);
		std::string nodeName = getString(childNode->getNodeName());
		if (nodeName == "node")
		{
			xercesc_3_0::DOMNamedNodeMap* childAttr = childNode->getAttributes();
			xercesc_3_0::DOMNode* typeNode = childAttr->getNamedItem(XMLString::transcode("type"));
			std::string typeAttr = getString(typeNode->getNodeValue());
			xercesc_3_0::DOMNode* nameNode = childAttr->getNamedItem(XMLString::transcode("name"));
			std::string nameAttr = getString(nameNode->getNodeValue());
			if (typeAttr == "JOINT")
			{
				int index = -1;
				if (parent != NULL)	
					index = parent->index();
				SkJoint* joint = skeleton.add_joint(SkJoint::TypeQuat, index);
				joint->quat()->activate();
				joint->name(SkJointName(nameAttr.c_str()));

				skeleton.channels().add(joint->name(), SkChannel::XPos);
				skeleton.channels().add(joint->name(), SkChannel::YPos);
				skeleton.channels().add(joint->name(), SkChannel::ZPos);
				joint->pos()->limits(SkVecLimits::X, false);
				joint->pos()->limits(SkVecLimits::Y, false);
				joint->pos()->limits(SkVecLimits::Z, false);

				if (parent == NULL)
					skeleton.root(joint);

				const xercesc_3_0::DOMNodeList* infoList = childNode->getChildNodes();
				for (unsigned int j = 0; j < infoList->getLength(); j++)
				{
					xercesc_3_0::DOMNode* infoNode = infoList->item(j);
					std::string infoNodeName = getString(infoNode->getNodeName());
					if (infoNodeName == "translate")
					{
						std::string offsetString = getString(infoNode->getTextContent());
						SrVec offset;
						offset.x = (float)atof(tokenize(offsetString).c_str()) * scale;
						offset.y = (float)atof(tokenize(offsetString).c_str()) * scale;
						offset.z = (float)atof(tokenize(offsetString).c_str()) * scale;
						joint->offset(offset);
					}
				}
				parseJoints(list->item(i), skeleton, motion, scale, joint);
			}
			else
				parseJoints(list->item(i), skeleton, motion, scale, parent);
		}
	}
}


void ParserOpenCOLLADA::parseLibraryAnimations(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale)
{
	std::vector<std::string> timingString;
	std::vector<std::string> dataEntryName;
	std::vector<std::string> frameDataString;
	std::map<std::string, int> idMap;

	const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		xercesc_3_0::DOMNode* node1 = list->item(i);
		std::string node1Name = getString(node1->getNodeName());
		if (node1Name == "animation")
		{
			xercesc_3_0::DOMNamedNodeMap* animationAttr = node1->getAttributes();
			xercesc_3_0::DOMNode* idNode = animationAttr->getNamedItem(XMLString::transcode("id"));
			std::string idAttr = getString(idNode->getNodeValue());
			std::string jointName = tokenize(idAttr, ".");
			std::string channelType = tokenize(idAttr, "_");
			
			const xercesc_3_0::DOMNodeList* list1 = node1->getChildNodes();
			for (unsigned int j = 0; j < list1->getLength(); j++)
			{
				xercesc_3_0::DOMNode* node2 = list1->item(j);
				std::string node2Name = getString(node2->getNodeName());
				if (node2Name == "source")
				{
					xercesc_3_0::DOMNamedNodeMap* sourceAttr = node2->getAttributes();
					xercesc_3_0::DOMNode* sourceIdNode = sourceAttr->getNamedItem(XMLString::transcode("id"));
					std::string sourceIdAttr = getString(sourceIdNode->getNodeValue());
					std::string dataName = sourceIdAttr;
					std::string op = tokenize(sourceIdAttr, "-");
					const xercesc_3_0::DOMNodeList* list2 = node2->getChildNodes();
					for (unsigned int k = 0; k < list2->getLength(); k++)
					{
						xercesc_3_0::DOMNode* node3 = list2->item(k);
						std::string node3Name = getString(node3->getNodeName());
						if (node3Name == "float_array")
						{
							xercesc_3_0::DOMNamedNodeMap* arrayAttr = node3->getAttributes();
							xercesc_3_0::DOMNode* arrayCountNode = arrayAttr->getNamedItem(XMLString::transcode("count"));
							int counter = atoi(getString(arrayCountNode->getNodeValue()).c_str());
							std::string arrayString = getString(node3->getTextContent());
							
							if (sourceIdAttr == "input")
							{
								timingString.push_back(arrayString);
							}
							if (sourceIdAttr == "output")
							{
								dataEntryName.push_back(dataName);
								frameDataString.push_back(arrayString);
							}
						}
					}
					
				}
			}
		}
	}
	if (timingString.size() != dataEntryName.size() || dataEntryName.size() != frameDataString.size())
		return;

	// first step, handle the ata
	std::vector<std::vector<float>> timing;
	std::vector<std::vector<float>> frameData;
	std::vector<std::vector<int>>	frameId;

	for (size_t i = 0; i < timingString.size(); i++)
	{
		std::vector<float> timingData;
		std::string value = tokenize(timingString[i]);
		while (value != "")
		{
			timingData.push_back(float(atof(value.c_str())));
			value = tokenize(timingString[i]);
		}
		timing.push_back(timingData);
	}

	for (size_t i = 0; i < frameDataString.size(); i++)
	{
		std::vector<float> data;
		std::string value = tokenize(frameDataString[i]);
		while (value != "")
		{
			data.push_back(float(atof(value.c_str())));
			value = tokenize(frameDataString[i]);
		}
		frameData.push_back(data);
	}

	// add channels
	int globalId = 0;
	SkChannelArray motionChannels;
	for (size_t i = 0; i < dataEntryName.size(); i++)
	{
		std::string fullName = dataEntryName[i];
		std::string jointName = tokenize(fullName, ".");
		size_t found = fullName.find_last_of(".");
		std::string name = fullName.substr(found + 1);
		std::string type = tokenize(name, "_");
		if (type == "translate" && jointName == "base")
		{
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::XPos);
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::YPos);
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::ZPos);
			idMap.insert(std::make_pair(dataEntryName[i], globalId));
			globalId += 3;
		}
/*		if (type == "translateX")
		{
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::XPos);
			idMap.insert(std::make_pair(dataEntryName[i], globalId));
			globalId += 1;
		}
		if (type == "translateY")
		{
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::YPos);
			idMap.insert(std::make_pair(dataEntryName[i], globalId));
			globalId += 1;
		}
		if (type == "translateZ")
		{
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::ZPos);
			idMap.insert(std::make_pair(dataEntryName[i], globalId));
			globalId += 1;
		}
*/
		if (type == "rotateX")
		{
			motionChannels.add(SkJointName(jointName.c_str()), SkChannel::Quat);
			motionChannels.size();
			idMap.insert(std::make_pair(dataEntryName[i], globalId));
			globalId += 4;
		}
	}
	motion.init(motionChannels);

	// add frames
	int numberFrame = 0;
	std::vector<float> fullTiming;
	for (size_t i = 0; i < timing.size(); i++)
		if (numberFrame < (int)timing[i].size())	
		{
			numberFrame = timing[i].size();
			fullTiming = timing[i];
		}

	for (size_t i = 0; i < fullTiming.size(); i++)
		motion.insert_frame(i, fullTiming[i]);

	for (int i = 0; i < motion.frames(); i++)
		for (int j = 0; j < motion.posture_size(); j++)
			motion.posture(i)[j] = 0.0;

	// add data
	std::vector<SrVec> euler;
	for (int i = 0; i < numberFrame; i++)
	{
		SrVec angle;
		euler.push_back(angle);
	}
	for (size_t i = 0; i < dataEntryName.size(); i++)
	{
		std::string fullName = dataEntryName[i];
		std::string jointName = tokenize(fullName, ".");
		size_t found = fullName.find_last_of(".");
		std::string name = fullName.substr(found + 1);
		std::string type = tokenize(name, "_");
		int size = timing[i].size();
		if (type == "translate" && jointName == "base")
		{
			int index = idMap[dataEntryName[i]];
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				motion.posture(frame)[index + 0] = frameData[i][j * 3 + 0] * scale;
				motion.posture(frame)[index + 1] = frameData[i][j * 3 + 1] * scale;
				motion.posture(frame)[index + 2] = frameData[i][j * 3 + 2] * scale;
			}
		}
/*		if (type == "translateX")
		{
			int index = idMap[dataEntryName[i]];
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				motion.posture(frame)[index] = frameData[i][j];
			}
		}
		if (type == "translateY")
		{
			int index = idMap[dataEntryName[i]];
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				motion.posture(frame)[index] = frameData[i][j];
			}
		}
		if (type == "translateZ")
		{
			int index = idMap[dataEntryName[i]];
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				motion.posture(frame)[index] = frameData[i][j];
			}
		}
*/
		if (type == "rotateZ")
		{
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				euler[frame].z = float(frameData[i][j] * 0.017444444);
			}
		}
		if (type == "rotateY")
		{
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				euler[frame].y = float(frameData[i][j] * 0.017444444);
			}
		}
		if (type == "rotateX")
		{
			int index = idMap[dataEntryName[i]];
			for (int j = 0; j < size; j++)
			{
				int frame = getFrameNumber(fullTiming, timing[i][j]);
				euler[frame].x = float(frameData[i][j] * 0.017444444);

				SrMat xrot;
				xrot.rotx(euler[frame].x);
				SrMat yrot;
				yrot.roty(euler[frame].y);
				SrMat zrot;
				zrot.rotz(euler[frame].z);

				SrMat matrix;
				matrix = xrot * yrot * zrot;

				SrQuat quat = SrQuat(matrix);
				motion.posture(frame)[index + 0] = quat.w;
				motion.posture(frame)[index + 1] = quat.x;
				motion.posture(frame)[index + 2] = quat.y;
				motion.posture(frame)[index + 3] = quat.z;

			}
			for (int j = 0; j < numberFrame; j++)
			{
				SrVec emptyVec;
				euler[j] = emptyVec;
			}
		}
	}

	double duration = double(motion.duration());
	motion.synch_points.set_time(0.0, duration / 3.0, duration / 2.0, duration * 2.0/3.0, duration);
	motion.compress();
}

int ParserOpenCOLLADA::getFrameNumber(std::vector<float>& timing, float currentTime)
{
	for (size_t i = 0; i < timing.size(); i++)
	{
		float diff = fabs(currentTime - timing[i]);
		if (diff < 0.00001)
			return int(i);
	}
	return -1;
}

std::string ParserOpenCOLLADA::getString(const XMLCh* s)
{
	if (!s)	return "";
	std::string str = XMLString::transcode(s);
	return str;
}


std::string ParserOpenCOLLADA::tokenize(std::string& str, const std::string& delimiters, int mode)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	if (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		std::string return_string = str.substr(lastPos, pos - lastPos);
		std::string::size_type lastPos = str.find_first_not_of(delimiters, pos);
		if (mode == 1)
		{
			if (std::string::npos == lastPos)	str = "";
			else								str = str.substr(lastPos, str.size() - lastPos);
		}
		return return_string;
	}
	else
		return "";
}