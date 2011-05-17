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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>


bool ParserOpenCOLLADA::parse(SkSkeleton& skeleton, SkMotion& motion, std::string pathName, float scale)
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
		int order;
		std::string filebasename = boost::filesystem::basename(pathName);
		motion.name(filebasename.c_str());
		skeleton.name(filebasename.c_str());
		parser->parse(pathName.c_str());
		xercesc_3_0::DOMDocument* doc = parser->getDocument();
		xercesc_3_0::DOMNode* skNode = getNode("library_visual_scenes", doc);
		if (!skNode)
		{
			LOG("ParserOpenCOLLADA::parse ERR: no skeleton info contained in this file");
			return false;
		}
		parseLibraryVisualScenes(skNode, skeleton, motion, scale, order);
		xercesc_3_0::DOMNode* skmNode = getNode("library_animations", doc);
		if (!skmNode)
		{
		//	LOG("ParserOpenCOLLADA::parse WARNING: no motion info contained in this file");
			return true;
		}
		parseLibraryAnimations(skmNode, skeleton, motion, scale, order);
	//	animationPostProcess(skeleton, motion);
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

xercesc_3_0::DOMNode* ParserOpenCOLLADA::getNode(std::string nodeName, xercesc_3_0::DOMNode* node)
{
	int type = node->getNodeType();
	std::string name = getString(node->getNodeName());
	std::string value = getString(node->getNodeValue());
	if (name == nodeName && node->getNodeType() ==  xercesc_3_0::DOMNode::ELEMENT_NODE)
		return node;


	xercesc_3_0::DOMNode* child = NULL;
	const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
	for (unsigned int c = 0; c < list->getLength(); c++)
	{
		child = getNode(nodeName, list->item(c));
		if (child)
			break;
	}
	return child;
}

void ParserOpenCOLLADA::parseLibraryVisualScenes(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order)
{
	const xercesc_3_0::DOMNodeList* list1 = node->getChildNodes();
	for (unsigned int c = 0; c < list1->getLength(); c++)
	{
		xercesc_3_0::DOMNode* node1 = list1->item(c);
		std::string nodeName = getString(node1->getNodeName());
		if (nodeName == "visual_scene")
			parseJoints(node1, skeleton, motion, scale, order, NULL);
	}
}

void ParserOpenCOLLADA::parseJoints(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, SkJoint* parent)
{
	const xercesc_3_0::DOMNodeList* list = node->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		xercesc_3_0::DOMNode* childNode = list->item(i);
		std::string nodeName = getString(childNode->getNodeName());
		if (nodeName == "node")
		{
			xercesc_3_0::DOMNamedNodeMap* childAttr = childNode->getAttributes();
			xercesc_3_0::DOMNode* nameNode = childAttr->getNamedItem(XMLString::transcode("name"));
			std::string nameAttr = "";
			if (nameNode)
				nameAttr = getString(nameNode->getNodeValue());
			xercesc_3_0::DOMNode* typeNode = childAttr->getNamedItem(XMLString::transcode("type"));
			std::string typeAttr = "";
			if (typeNode)
				typeAttr = getString(typeNode->getNodeValue());
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
				skeleton.channels().add(joint->name(), SkChannel::Quat);
				joint->quat()->activate();
				float rotx = 0.0f;
				float roty = 0.0f;
				float rotz = 0.0f;
				float jorientx = 0.0f;
				float jorienty = 0.0f;
				float jorientz = 0.0f;
				SrVec offset;

				std::vector<std::string> orderVec;

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
						offset.x = (float)atof(tokenize(offsetString).c_str()) * scale;
						offset.y = (float)atof(tokenize(offsetString).c_str()) * scale;
						offset.z = (float)atof(tokenize(offsetString).c_str()) * scale;
					}
					if (infoNodeName == "rotate")
					{
						xercesc_3_0::DOMNamedNodeMap* rotateAttr = infoNode->getAttributes();
						xercesc_3_0::DOMNode* sidNode = rotateAttr->getNamedItem(XMLString::transcode("sid"));
						std::string sidAttr = getString(sidNode->getNodeValue());

						if (sidAttr.substr(0, 11) == "jointOrient")
						{
							std::string jointOrientationString = getString(infoNode->getTextContent());
							float finalValue;
							for (int tokenizeC = 0; tokenizeC < 4; tokenizeC++)
								finalValue = (float)atof(tokenize(jointOrientationString).c_str());
							if (sidAttr == "jointOrientX") jorientx = finalValue;
							if (sidAttr == "jointOrientY") jorienty = finalValue;
							if (sidAttr == "jointOrientZ") jorientz = finalValue;
							if (orderVec.size() != 3)
								orderVec.push_back(sidAttr.substr(11, 1));
						}
						if (sidAttr.substr(0, 6) == "rotate")
						{
							std::string rotationString = getString(infoNode->getTextContent());
							float finalValue;
							for (int tokenizeC = 0; tokenizeC < 4; tokenizeC++)
								finalValue = (float)atof(tokenize(rotationString).c_str());
							if (sidAttr == "rotateX") rotx = finalValue;
							if (sidAttr == "rotateY") roty = finalValue;
							if (sidAttr == "rotateZ") rotz = finalValue;
							if (orderVec.size() != 3)
								orderVec.push_back(sidAttr.substr(6, 1));
						}
					}
				}
				order = getRotationOrder(orderVec);
				if (order == -1)
					LOG("ParserOpenCOLLADA::parseJoints ERR: rotation info not correct in the file");

				SrMat rotMat;
				rotx *= float(M_PI) / 180.0f;
				roty *= float(M_PI) / 180.0f;
				rotz *= float(M_PI) / 180.0f;
				sr_euler_mat(order, rotMat, rotx, roty, rotz);
				SrMat jorientMat;
				jorientx *= float(M_PI) / 180.0f;
				jorienty *= float(M_PI) / 180.0f;
				jorientz *= float(M_PI) / 180.0f;
				sr_euler_mat(order, jorientMat, jorientx, jorienty, jorientz);
				joint->offset(offset);
				SrMat finalRotMat = rotMat;
				SrQuat quat = SrQuat(rotMat);
				SkJointQuat* jointQuat = joint->quat();
				jointQuat->prerot(quat);
				SrQuat jorientQ = SrQuat(jorientMat);
				jointQuat->orientation(jorientQ);
				parseJoints(list->item(i), skeleton, motion, scale, order, joint);
			}
			else
				parseJoints(list->item(i), skeleton, motion, scale, order, parent);
		}
	}
}

void ParserOpenCOLLADA::parseLibraryAnimations(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order)
{
	SkChannelArray& skChannels = skeleton.channels();
	motion.init(skChannels);
	SkChannelArray& motionChannels = motion.channels();
	bool initFrames = true;

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
					size_t pos = sourceIdAttr.find_last_of("-");
					std::string op = sourceIdAttr.substr(pos + 1);
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
						
							if (initFrames && op == "input")
							{
								for (int frameCt = 0; frameCt < counter; frameCt++)
								{
									motion.insert_frame(frameCt, (float)atof(tokenize(arrayString).c_str()));
									for (int postureCt = 0; postureCt < motion.posture_size(); postureCt++)
										motion.posture(frameCt)[postureCt] = 0.0f;
								}
								initFrames = false;
							}

							if (op == "output")
							{
								int channelId = getMotionChannelId(motionChannels, sourceIdAttr);
								if (channelId >= 0)
								{
									int stride = counter / motion.frames();
									for (int frameCt = 0; frameCt < motion.frames(); frameCt++)
										for (int strideCt = 0; strideCt < stride; strideCt++)
										{
											float v = (float)atof(tokenize(arrayString).c_str());
											motion.posture(frameCt)[channelId + strideCt] = v * scale;
										}
								}
							}
						}
					}
					
				}
			}
		}
	}
	// now transfer the motion euler data to quaternion data
	std::vector<int> quatIndices;
	for (int i = 0; i < motionChannels.size(); i++)
	{
		SkChannel& chan = motionChannels[i];
		if (chan.type == SkChannel::Quat)
		{
			int id = motionChannels.float_position(i);
			quatIndices.push_back(id);
		}
	}
	for (int frameCt = 0; frameCt < motion.frames(); frameCt++)
		for (size_t i = 0; i < quatIndices.size(); i++)
		{
			int quatId = quatIndices[i];
			float rotx = motion.posture(frameCt)[quatId + 0] / scale;
			float roty = motion.posture(frameCt)[quatId + 1] / scale;
			float rotz = motion.posture(frameCt)[quatId + 2] / scale;
			rotx *= float(M_PI) / 180.0f;
			roty *= float(M_PI) / 180.0f;
			rotz *= float(M_PI) / 180.0f;
			SrMat mat;
			sr_euler_mat(order, mat, rotx, roty, rotz);
			SrQuat quat = SrQuat(mat);
			motion.posture(frameCt)[quatId + 0] = quat.w;
			motion.posture(frameCt)[quatId + 1] = quat.x;
			motion.posture(frameCt)[quatId + 2] = quat.y;
			motion.posture(frameCt)[quatId + 3] = quat.z;
		}

	double duration = double(motion.duration());
	motion.synch_points.set_time(0.0, duration / 3.0, duration / 2.0, duration * 2.0/3.0, duration);
	motion.compress();
}

void ParserOpenCOLLADA::animationPostProcess(SkSkeleton& skeleton, SkMotion& motion)
{
	SkChannelArray& motionChannels = motion.channels();
	int numChannel = motionChannels.size(); 
	for (int i = 0; i < motion.frames(); i++)
	{
		int chanIndex = 0;
		for (int j = 0; j < numChannel; j++)
		{
			SkChannel& chan = motionChannels[j];
			std::string chanName = motionChannels.name(j);
			SkChannel::Type chanType = chan.type;
			SkJoint* joint = skeleton.search_joint(chanName.c_str());
			
			if (chanType == SkChannel::XPos)
			{
				if (joint)
					motion.posture(i)[chanIndex] -= joint->offset().x;
				chanIndex++;
			}
			if (chanType == SkChannel::YPos)
			{
				if (joint)
					motion.posture(i)[chanIndex] -= joint->offset().y;
				chanIndex++;
			}
			if (chanType == SkChannel::ZPos)
			{
				if (joint)
					motion.posture(i)[chanIndex] -= joint->offset().z;
				chanIndex++;
			}
			if (chanType == SkChannel::Quat)
			{
				SrQuat globalQuat = SrQuat(motion.posture(i)[chanIndex], motion.posture(i)[chanIndex + 1], motion.posture(i)[chanIndex + 2], motion.posture(i)[chanIndex + 3]);
				SrQuat preQuat = joint->quat()->prerot();
				SrQuat localQuat = preQuat.inverse() * globalQuat;
				motion.posture(i)[chanIndex] = localQuat.w;
				motion.posture(i)[chanIndex + 1] = localQuat.x;
				motion.posture(i)[chanIndex + 2] = localQuat.y;
				motion.posture(i)[chanIndex + 3] = localQuat.z;
				chanIndex += 4;
			}

		}
	}
}

int ParserOpenCOLLADA::getMotionChannelId(SkChannelArray& mChannels, std::string sourceName)
{
	int id = -1;
	int dataId = -1;
	SkJointName jName = SkJointName(tokenize(sourceName, ".").c_str());
	SkChannel::Type chanType;
	
	if (sourceName.find("translate") != std::string::npos)
	{
		chanType = SkChannel::XPos;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
	}
	else if (sourceName.find("translateX") != std::string::npos)
	{
		chanType = SkChannel::XPos;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
	}
	else if (sourceName.find("translateY") != std::string::npos)
	{
		chanType = SkChannel::YPos;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
	}
	else if (sourceName.find("translateZ") != std::string::npos)
	{
		chanType = SkChannel::ZPos;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
	}
	else if (sourceName.find("rotateX") != std::string::npos)
	{
		chanType = SkChannel::Quat;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
	}
	else if (sourceName.find("rotateY") != std::string::npos)
	{
		chanType = SkChannel::Quat;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
		if (id >= 0)
			dataId += 1;
	}
	else if (sourceName.find("rotateZ") != std::string::npos)
	{
		chanType = SkChannel::Quat;
		id = mChannels.search(jName, chanType);
		dataId = mChannels.float_position(id);
		if (id >= 0)
			dataId += 2;
	}
	return dataId;
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

int ParserOpenCOLLADA::getRotationOrder(std::vector<std::string> orderVec)
{
	if (orderVec.size() == 3)
	{
		if (orderVec[0] == "X" && orderVec[1] == "Y" && orderVec[2] == "Z")
			return 321;
		if (orderVec[0] == "X" && orderVec[1] == "Z" && orderVec[2] == "Y")
			return 231;
		if (orderVec[0] == "Y" && orderVec[1] == "X" && orderVec[2] == "Z")
			return 213;
		if (orderVec[0] == "Y" && orderVec[1] == "Z" && orderVec[2] == "X")
			return 132;
		if (orderVec[0] == "Z" && orderVec[1] == "X" && orderVec[2] == "Y")
			return 213;
		if (orderVec[0] == "Z" && orderVec[1] == "Y" && orderVec[2] == "X")
			return 123;
	}
	return -1;
} 