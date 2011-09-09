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
#include <algorithm>
#include <cctype>
#include <string>
#include <sbm/BMLDefs.h>

bool ParserOpenCOLLADA::parse(SkSkeleton& skeleton, SkMotion& motion, std::string pathName, float scale)
{
	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.getMessage());
		std::cout << "Error during initialization! :\n" << message << "\n";
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
		DOMDocument* doc = parser->getDocument();
		DOMNode* skNode = getNode("library_visual_scenes", doc);
		if (!skNode)
		{
			LOG("ParserOpenCOLLADA::parse ERR: no skeleton info contained in this file");
			return false;
		}
		parseLibraryVisualScenes(skNode, skeleton, motion, scale, order);
		DOMNode* skmNode = getNode("library_animations", doc);
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
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.getMessage());
		std::cout << "Exception message is: \n" << message << "\n";
		return false;
	}
	catch (const DOMException& toCatch) {
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.msg);
		std::cout << "Exception message is: \n" << message << "\n";
		return false;
	}
	catch (...) {
		LOG("Unexpected Exception in ParseOpenCollada::parse()");
		return false;
	}

	delete parser;
	delete errHandler;
	return true;
}

DOMNode* ParserOpenCOLLADA::getNode(std::string nodeName, DOMNode* node)
{
	int type = node->getNodeType();
	std::string name = getString(node->getNodeName());
	std::string value = getString(node->getNodeValue());
	if (name == nodeName && node->getNodeType() ==  DOMNode::ELEMENT_NODE)
		return node;

	DOMNode* child = NULL;
	const DOMNodeList* list = node->getChildNodes();
	for (unsigned int c = 0; c < list->getLength(); c++)
	{
		child = getNode(nodeName, list->item(c));
		if (child)
			break;
	}
	return child;
}

DOMNode* ParserOpenCOLLADA::getNode(std::string nodeName, std::string fileName)
{
	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.getMessage());
		std::cout << "Error during initialization! :\n" << message << "\n";
		return NULL;
	}

	XercesDOMParser* parser = new XercesDOMParser();
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true);    // optional

	ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
	parser->setErrorHandler(errHandler);

	try 
	{
		std::string filebasename = boost::filesystem::basename(fileName);
		parser->parse(fileName.c_str());
		DOMDocument* doc = parser->getDocument();
		return getNode(nodeName, doc);
	}
	catch (const XMLException& toCatch) 
	{
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.getMessage());
		LOG("Exception message is: %s", message.c_str());
		return NULL;
	}
	catch (const DOMException& toCatch) {
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.msg);
		LOG("Exception message is: %s", message.c_str());
		return NULL;
	}
		catch (...) {
		LOG("Unexpected Exception in ParserOpenCOLLADA::getNode()");
		return NULL;
	}

	delete parser;
	delete errHandler;
	return NULL;
}

void ParserOpenCOLLADA::parseLibraryVisualScenes(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order)
{
	const DOMNodeList* list1 = node->getChildNodes();
	for (unsigned int c = 0; c < list1->getLength(); c++)
	{
		DOMNode* node1 = list1->item(c);
		std::string nodeName = getString(node1->getNodeName());
		if (nodeName == "visual_scene")
			parseJoints(node1, skeleton, motion, scale, order, NULL);
	}
}

void ParserOpenCOLLADA::parseJoints(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, SkJoint* parent)
{
	const DOMNodeList* list = node->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		DOMNode* childNode = list->item(i);
		std::string nodeName = getString(childNode->getNodeName());
		if (nodeName == "node")
		{
			DOMNamedNodeMap* childAttr = childNode->getAttributes();

			DOMNode* nameNode = childAttr->getNamedItem(BML::BMLDefs::ATTR_NAME);
			std::string nameAttr = "";
			if (nameNode)
				nameAttr = getString(nameNode->getNodeValue());
			DOMNode* typeNode = childAttr->getNamedItem(BML::BMLDefs::ATTR_TYPE);
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
				joint->name(nameAttr);

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

				const DOMNodeList* infoList = childNode->getChildNodes();
				for (unsigned int j = 0; j < infoList->getLength(); j++)
				{
					DOMNode* infoNode = infoList->item(j);
					std::string infoNodeName = getString(infoNode->getNodeName());
					if (infoNodeName == "translate")
					{
						std::string offsetString = getString(infoNode->getTextContent());
						std::vector<std::string> tokens;
						vhcl::Tokenize(offsetString, tokens, " ");
						offset.x = (float)atof(tokens[0].c_str()) * scale;
						offset.y = (float)atof(tokens[1].c_str()) * scale;
						offset.z = (float)atof(tokens[2].c_str()) * scale;
					}
					if (infoNodeName == "rotate")
					{
						DOMNamedNodeMap* rotateAttr = infoNode->getAttributes();
						
						DOMNode* sidNode = rotateAttr->getNamedItem(BML::BMLDefs::ATTR_SID);
						std::string sidAttr = getString(sidNode->getNodeValue());

						if (sidAttr.substr(0, 11) == "jointOrient")
						{
							std::string jointOrientationString = getString(infoNode->getTextContent());
							std::vector<std::string> tokens;
							vhcl::Tokenize(jointOrientationString, tokens, " ");
							float finalValue;
							for (int tokenizeC = 0; tokenizeC < 4; tokenizeC++)
								finalValue = (float)atof(tokens[tokenizeC].c_str());
							if (sidAttr == "jointOrientX") jorientx = finalValue;
							if (sidAttr == "jointOrientY") jorienty = finalValue;
							if (sidAttr == "jointOrientZ") jorientz = finalValue;
							if (orderVec.size() != 3)
								orderVec.push_back(sidAttr.substr(11, 1));
						}
						if (sidAttr.substr(0, 6) == "rotate")
						{
							std::string rotationString = getString(infoNode->getTextContent());
							std::vector<std::string> tokens;
							vhcl::Tokenize(rotationString, tokens, " ");
							float finalValue;
							for (int tokenizeC = 0; tokenizeC < 4; tokenizeC++)
								finalValue = (float)atof(tokens[tokenizeC].c_str());
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

void ParserOpenCOLLADA::parseLibraryAnimations(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order)
{
	SkChannelArray& skChannels = skeleton.channels();
	motion.init(skChannels);
	SkChannelArray& motionChannels = motion.channels();
	SkChannelArray channelsForAdjusting;

	const DOMNodeList* list = node->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		DOMNode* node1 = list->item(i);
		std::string node1Name = getString(node1->getNodeName());
		if (node1Name == "animation")
		{
			DOMNamedNodeMap* animationAttr = node1->getAttributes();
			DOMNode* idNode = animationAttr->getNamedItem(BML::BMLDefs::ATTR_ID);
			std::string idAttr = getString(idNode->getNodeValue()); // these three variables have no use
			std::string jointName = tokenize(idAttr, ".-");	
			std::string channelType = tokenize(idAttr, "_");
			int numTimeInput = -1;
			if (channelType == "rotateX" || channelType == "rotateY" || channelType == "rotateZ")
			{
				if (channelsForAdjusting.search(jointName.c_str(), SkChannel::Quat) < 0)
					channelsForAdjusting.add(jointName.c_str(), SkChannel::Quat);
			}
			if (channelType == "translate")
			{
				channelsForAdjusting.add(jointName.c_str(), SkChannel::XPos);
				channelsForAdjusting.add(jointName.c_str(), SkChannel::YPos);
				channelsForAdjusting.add(jointName.c_str(), SkChannel::ZPos);
			}
			if (channelType == "translateX")
				channelsForAdjusting.add(jointName.c_str(), SkChannel::XPos);
			if (channelType == "translateY")
				channelsForAdjusting.add(jointName.c_str(), SkChannel::YPos);
			if (channelType == "translateZ")
				channelsForAdjusting.add(jointName.c_str(), SkChannel::ZPos);
			
			const DOMNodeList* list1 = node1->getChildNodes();
			for (unsigned int j = 0; j < list1->getLength(); j++)
			{
				DOMNode* node2 = list1->item(j);
				std::string node2Name = getString(node2->getNodeName());
				if (node2Name == "source")
				{
					DOMNamedNodeMap* sourceAttr = node2->getAttributes();
					DOMNode* sourceIdNode = sourceAttr->getNamedItem(BML::BMLDefs::ATTR_ID);
					std::string sourceIdAttr = getString(sourceIdNode->getNodeValue());
					size_t pos = sourceIdAttr.find_last_of("-");
					std::string op = sourceIdAttr.substr(pos + 1);
					const DOMNodeList* list2 = node2->getChildNodes();
					for (unsigned int k = 0; k < list2->getLength(); k++)
					{
						DOMNode* node3 = list2->item(k);
						std::string node3Name = getString(node3->getNodeName());
						if (node3Name == "float_array")
						{
							DOMNamedNodeMap* arrayAttr = node3->getAttributes();
							DOMNode* arrayCountNode = arrayAttr->getNamedItem(BML::BMLDefs::ATTR_COUNT);
							int counter = atoi(getString(arrayCountNode->getNodeValue()).c_str());
							std::string arrayString = getString(node3->getTextContent());
							std::vector<std::string> tokens;
							vhcl::Tokenize(arrayString, tokens, " ");
						
							if (op == "input")
							{
								numTimeInput = counter;
								if (motion.frames() == 0)
								{
									for (int frameCt = 0; frameCt < counter; frameCt++)
									{
										motion.insert_frame(frameCt, (float)atof(tokens[frameCt].c_str()));
										for (int postureCt = 0; postureCt < motion.posture_size(); postureCt++)
											motion.posture(frameCt)[postureCt] = 0.0f;
									}
								}
/*								if (motion.frames() < counter)
								{
									for (int frameCt = 0; frameCt < counter; frameCt++)
									{
										float k = (float)atof(tokenize(arrayString).c_str());
										float keyTime = motion.keytime(frameCt);
										if (keyTime > k)
										{
											bool flag = motion.insert_frame(frameCt, k);
											for (int postureCt = 0; postureCt < motion.posture_size(); postureCt++)
												motion.posture(frameCt)[postureCt] = 0.0f;
										}
									}
								}
*/
							}

							if (op == "output")
							{
								int channelId = getMotionChannelId(motionChannels, sourceIdAttr);
								if (channelId >= 0)
								{
									int stride = counter / numTimeInput;
									for (int frameCt = 0; frameCt < motion.frames(); frameCt++)
										for (int strideCt = 0; strideCt < stride; strideCt++)
										{
											float v = (float)atof(tokens[frameCt].c_str());
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
	motion.synch_points.set_time(0.0, duration / 3.0, duration / 2.0, duration / 2.0, duration / 2.0, duration * 2.0/3.0, duration);
	motion.compress();
	// now there's adjust for the channels by default
	animationPostProcessByChannels(skeleton, motion, channelsForAdjusting);
}

void ParserOpenCOLLADA::animationPostProcess(SkSkeleton& skeleton, SkMotion& motion)
{
	ParserOpenCOLLADA::animationPostProcessByChannels(skeleton, motion, motion.channels());
}

void ParserOpenCOLLADA::animationPostProcessByChannels(SkSkeleton& skeleton, SkMotion& motion, SkChannelArray& motionChannels)
{
	int numChannel = motionChannels.size(); 
	for (int i = 0; i < motion.frames(); i++)
	{
		for (int j = 0; j < numChannel; j++)
		{
			SkChannel& chan = motionChannels[j];
			std::string chanName = motionChannels.name(j);
			SkChannel::Type chanType = chan.type;
			SkJoint* joint = skeleton.search_joint(chanName.c_str());
			if (!joint)
				continue;

			int id = motion.channels().search(chanName.c_str(), chanType);
			int dataId = motion.channels().float_position(id);
			if (dataId < 0)
				continue;

			if (chanType == SkChannel::XPos)
			{
				float v = motion.posture(i)[dataId];
				motion.posture(i)[dataId] = v - joint->offset().x;
			}
			if (chanType == SkChannel::YPos)
			{
				float v = motion.posture(i)[dataId];
				motion.posture(i)[dataId] = v - joint->offset().y;
			}
			if (chanType == SkChannel::ZPos)
			{
				float v = motion.posture(i)[dataId];
				motion.posture(i)[dataId] = v - joint->offset().z;
			}
			if (chanType == SkChannel::Quat)
			{
				SrQuat globalQuat = SrQuat(motion.posture(i)[dataId], motion.posture(i)[dataId + 1], motion.posture(i)[dataId + 2], motion.posture(i)[dataId + 3]);
				SrQuat preQuat = joint->quat()->prerot();
				SrQuat localQuat = preQuat.inverse() * globalQuat;
				motion.posture(i)[dataId] = localQuat.w;
				motion.posture(i)[dataId + 1] = localQuat.x;
				motion.posture(i)[dataId + 2] = localQuat.y;
				motion.posture(i)[dataId + 3] = localQuat.z;
			}

		}
	}
}

int ParserOpenCOLLADA::getMotionChannelId(SkChannelArray& mChannels, std::string sourceName)
{
	int id = -1;
	int dataId = -1;
	std::string jName = tokenize(sourceName, ".-");
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
	std::string temp;
	xml_utils::xml_translate(&temp, s);
	return temp;
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

std::string ParserOpenCOLLADA::getGeometryType(std::string idString)
{
	size_t found = idString.find("position");
	if (found != string::npos)
		return "positions";
	found = idString.find("normal");
	if (found != string::npos)
		return "normals";
	found = idString.find("uv");
	if (found != string::npos)
		return "texcoords";
	found = idString.find("map");
	if (found != string::npos)
		return "texcoords";

	LOG("ParserOpenCOLLADA::getGeometryType ERR!");	
	return "";
}

void ParserOpenCOLLADA::parseLibraryGeometries(DOMNode* node, std::vector<SrModel*>& meshModelVec, float scale)
{
	const DOMNodeList* list = node->getChildNodes();
	for (unsigned int c = 0; c < list->getLength(); c++)
	{
		DOMNode* node = list->item(c);
		std::string nodeName = getString(node->getNodeName());
		if (nodeName == "geometry")
		{
			SrModel* newModel = new SrModel();
			DOMNamedNodeMap* nodeAttr = node->getAttributes();
			DOMNode* nameNode = nodeAttr->getNamedItem(BML::BMLDefs::ATTR_NAME);
			std::string nameAttr = "";
			if (nameNode)
				nameAttr = getString(nameNode->getNodeValue());
			newModel->name = SrString(nameAttr.c_str());
			DOMNode* meshNode = ParserOpenCOLLADA::getNode("mesh", node);
			if (!meshNode)	continue;
			for (unsigned int c1 = 0; c1 < meshNode->getChildNodes()->getLength(); c1++)
			{
				DOMNode* node1 = meshNode->getChildNodes()->item(c1);
				std::string nodeName1 = getString(node1->getNodeName());
				if (nodeName1 == "source")
				{
					DOMNamedNodeMap* sourceAttr = node1->getAttributes();
					DOMNode* idNode = sourceAttr->getNamedItem(BML::BMLDefs::ATTR_ID);
					std::string idString = getString(idNode->getNodeValue());
					size_t pos = idString.find_last_of("-");
					std::string tempString = idString.substr(pos + 1);
					idString = tempString;
					std::transform(idString.begin(), idString.end(), idString.begin(), ::tolower);
					std::string idType = getGeometryType(idString);
					// below is a faster way to parse all the data, have potential bug
					DOMNode* floatNode = ParserOpenCOLLADA::getNode("float_array", node1);
					DOMNode* countNode = floatNode->getAttributes()->getNamedItem(BML::BMLDefs::ATTR_COUNT);
					int count = atoi(getString(countNode->getNodeValue()).c_str());
					if (idType == "positions")
						count /= 3;
					if (idType == "normals")
						count /= 3;
					if (idType == "texcoords")
						count /= 2;
					std::string floatString = getString(floatNode->getTextContent());
					std::vector<std::string> tokens;
					vhcl::Tokenize(floatString, tokens, " ");
					int index = 0;
					for (int i = 0; i < count; i++)
					{
						if (idType == "positions")
						{
							newModel->V.push();
							newModel->V.top().x = (float)atof(tokens[index++].c_str());
							newModel->V.top().y = (float)atof(tokens[index++].c_str());
							newModel->V.top().z = (float)atof(tokens[index++].c_str());
						}
						if (idType == "normals")
						{
							newModel->N.push();
							newModel->N.top().x = (float)atof(tokens[index++].c_str());
							newModel->N.top().y = (float)atof(tokens[index++].c_str());
							newModel->N.top().z = (float)atof(tokens[index++].c_str());
						}
						if (idType == "texcoords")
						{
							newModel->T.push();
							newModel->T.top().x = (float)atof(tokens[index++].c_str());
							newModel->T.top().y = (float)atof(tokens[index++].c_str());
						}
					}
				}
				if (nodeName1 == "triangles" || nodeName1 == "polylist")
				{
					DOMNamedNodeMap* nodeAttr1 = node1->getAttributes();
					DOMNode* countNode = nodeAttr1->getNamedItem(BML::BMLDefs::ATTR_COUNT);
					int count = atoi(getString(countNode->getNodeValue()).c_str());
					std::vector<std::string> inputs;
					int numInput = 0;
					std::vector<int> vcountList;
					for (unsigned int c2 = 0; c2 < node1->getChildNodes()->getLength(); c2++)
					{
						DOMNode* inputNode = node1->getChildNodes()->item(c2);
						if (XMLString::compareString(inputNode->getNodeName(), BML::BMLDefs::ATTR_INPUT) == 0)
						{
							DOMNamedNodeMap* inputNodeAttr = inputNode->getAttributes();
							DOMNode* semanticNode = inputNodeAttr->getNamedItem(BML::BMLDefs::ATTR_SEMANTIC);
							inputs.push_back(getString(semanticNode->getNodeValue()));
							numInput++;
						}
						if (XMLString::compareString(inputNode->getNodeName(), BML::BMLDefs::ATTR_VCOUNT) == 0)
						{
							std::string vcountString = getString(inputNode->getTextContent());
							std::vector<std::string> tokens;
							vhcl::Tokenize(vcountString, tokens, " ");
							for (int i = 0; i < count; i++)
								vcountList.push_back(atoi(tokens[i].c_str()));
						}
					}
					if (vcountList.size() == 0)
					{
						for (int i = 0; i < count; i++)
							vcountList.push_back(3);
					}
					DOMNode* pNode = ParserOpenCOLLADA::getNode("p", node1);
					std::string pString = getString(pNode->getTextContent());
					// below code has potential bug because input order maybe different
					std::vector<std::string> tokens;
					vhcl::Tokenize(pString, tokens, " ");
					int index = 0;
					for (int i = 0; i < count; i++)
					{
						if (numInput == 1)
						{
							if (inputs[0] == "VERTEX")
							{
								if (vcountList[i] == 3)
								{
									newModel->F.push();
									newModel->F.top().a = atoi(tokens[index++].c_str());
									newModel->F.top().b = atoi(tokens[index++].c_str());
									newModel->F.top().c = atoi(tokens[index++].c_str());
								}
								if (vcountList[i] == 4)
								{
									newModel->F.push();
									int a = atoi(tokens[index++].c_str());
									int b = atoi(tokens[index++].c_str());
									int c = atoi(tokens[index++].c_str());
									int d = atoi(tokens[index++].c_str());
									newModel->F.top().a = a;
									newModel->F.top().b = b;
									newModel->F.top().c = c;
									newModel->F.push();
									newModel->F.top().a = a;
									newModel->F.top().b = c;
									newModel->F.top().c = d;
								}
							}
							else
								LOG("ParserOpenCOLLADA::parseLibraryGeometries ERR!");
						}
						if (numInput == 2)
						{
							if (inputs[0] == "VERTEX" && inputs[1] == "TEXCOORD")
							{
								if (vcountList[i] == 3)
								{
									newModel->F.push();
									newModel->Ft.push();
									newModel->F.top().a = atoi(tokens[index++].c_str());
									newModel->Ft.top().a = atoi(tokens[index++].c_str());
									newModel->F.top().b = atoi(tokens[index++].c_str());
									newModel->Ft.top().b = atoi(tokens[index++].c_str());
									newModel->F.top().c = atoi(tokens[index++].c_str());
									newModel->Ft.top().c = atoi(tokens[index++].c_str());	
								}
								if (vcountList[i] == 4)
								{
									int a = atoi(tokens[index++].c_str());
									int a1 = atoi(tokens[index++].c_str());
									int b = atoi(tokens[index++].c_str());
									int b1 = atoi(tokens[index++].c_str()); 
									int c = atoi(tokens[index++].c_str());
									int c1 = atoi(tokens[index++].c_str());
									int d = atoi(tokens[index++].c_str());
									int d1 = atoi(tokens[index++].c_str()); 
									newModel->F.push();
									newModel->Ft.push();
									newModel->F.top().a = a;
									newModel->Ft.top().a = a1;
									newModel->F.top().b = b;
									newModel->Ft.top().b = b1;
									newModel->F.top().c = c;
									newModel->Ft.top().c = c1;	

									newModel->F.push();
									newModel->Ft.push();
									newModel->F.top().a = a;
									newModel->Ft.top().a = a1;
									newModel->F.top().b = c;
									newModel->Ft.top().b = c1;
									newModel->F.top().c = d;
									newModel->Ft.top().c = d1;
								}
							}
							else if (inputs[0] == "VERTEX" && inputs[1] == "NORMAL")
							{
								if (vcountList[i] == 3)
								{
									newModel->F.push();
									newModel->Fn.push();
									newModel->F.top().a = atoi(tokens[index++].c_str());
									newModel->Fn.top().a = atoi(tokens[index++].c_str());
									newModel->F.top().b = atoi(tokens[index++].c_str());
									newModel->Fn.top().b = atoi(tokens[index++].c_str());
									newModel->F.top().c = atoi(tokens[index++].c_str());
									newModel->Fn.top().c = atoi(tokens[index++].c_str());
								}
								if (vcountList[i] == 4)
								{
									int a = atoi(tokens[index++].c_str());
									int a1 = atoi(tokens[index++].c_str());
									int b = atoi(tokens[index++].c_str());
									int b1 = atoi(tokens[index++].c_str()); 
									int c = atoi(tokens[index++].c_str());
									int c1 = atoi(tokens[index++].c_str());
									int d = atoi(tokens[index++].c_str());
									int d1 = atoi(tokens[index++].c_str()); 
									newModel->F.push();
									newModel->Fn.push();
									newModel->F.top().a = a;
									newModel->Fn.top().a = a1;
									newModel->F.top().b = b;
									newModel->Fn.top().b = b1;
									newModel->F.top().c = c;
									newModel->Fn.top().c = c1;

									newModel->F.push();
									newModel->Fn.push();
									newModel->F.top().a = a;
									newModel->Fn.top().a = a1;
									newModel->F.top().b = c;
									newModel->Fn.top().b = c1;
									newModel->F.top().c = d;
									newModel->Fn.top().c = d1;
								}
							}
							else
								LOG("ParserOpenCOLLADA::parseLibraryGeometries ERR!");
						} 
						if (numInput == 3)
						{
							if (inputs[0] == "VERTEX" && inputs[1] == "NORMAL" && inputs[2] == "TEXCOORD")
							{
								if (vcountList[i] == 3)
								{
									newModel->F.push();
									newModel->Fn.push();
									newModel->Ft.push();
									newModel->F.top().a = atoi(tokens[index++].c_str());
									newModel->Fn.top().a = atoi(tokens[index++].c_str());
									newModel->Ft.top().a = atoi(tokens[index++].c_str());
									newModel->F.top().b = atoi(tokens[index++].c_str());
									newModel->Fn.top().b = atoi(tokens[index++].c_str());
									newModel->Ft.top().b = atoi(tokens[index++].c_str());
									newModel->F.top().c = atoi(tokens[index++].c_str());
									newModel->Fn.top().c = atoi(tokens[index++].c_str());
									newModel->Ft.top().c = atoi(tokens[index++].c_str());	
								}
								if (vcountList[i] == 4)
								{
									int a = atoi(tokens[index++].c_str());
									int a1 = atoi(tokens[index++].c_str());
									int a2 = atoi(tokens[index++].c_str());
									int b = atoi(tokens[index++].c_str());
									int b1 = atoi(tokens[index++].c_str()); 
									int b2 = atoi(tokens[index++].c_str()); 
									int c = atoi(tokens[index++].c_str());
									int c1 = atoi(tokens[index++].c_str());
									int c2 = atoi(tokens[index++].c_str());
									int d = atoi(tokens[index++].c_str());
									int d1 = atoi(tokens[index++].c_str()); 
									int d2 = atoi(tokens[index++].c_str()); 

									newModel->F.push();
									newModel->Fn.push();
									newModel->Ft.push();
									newModel->F.top().a = a;
									newModel->Fn.top().a = a1;
									newModel->Ft.top().a = a2;
									newModel->F.top().b = b;
									newModel->Fn.top().b = b1;
									newModel->Ft.top().b = b2;
									newModel->F.top().c = c;
									newModel->Fn.top().c = c1;
									newModel->Ft.top().c = c2;	

									newModel->F.push();
									newModel->Fn.push();
									newModel->Ft.push();
									newModel->F.top().a = a;
									newModel->Fn.top().a = a1;
									newModel->Ft.top().a = a2;
									newModel->F.top().b = c;
									newModel->Fn.top().b = c1;
									newModel->Ft.top().b = c2;
									newModel->F.top().c = d;
									newModel->Fn.top().c = d1;
									newModel->Ft.top().c = d2;	
								}
							}
							else
								LOG("ParserOpenCOLLADA::parseLibraryGeometries ERR!");
						}
					}				
				}
			}
			newModel->validate();
			newModel->remove_redundant_materials();
//			newModel->remove_redundant_normals();
			newModel->compress();
			meshModelVec.push_back(newModel);
		}
	}	
}
