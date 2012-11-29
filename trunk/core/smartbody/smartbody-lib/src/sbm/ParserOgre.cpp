/*
 *  ParserOgre.cpp - part of Motion Engine and SmartBody-lib
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
 */

#include "ParserOgre.h"
#include "sr/sr_euler.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>    
#include <algorithm>
#include <map>
#include <cctype>
#include <string>
#include <sbm/BMLDefs.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif

#if !defined (__ANDROID__) && !defined(SBM_IPHONE)
#include <sbm/GPU/SbmTexture.h>
#endif


bool ParserOgre::parse(SkSkeleton& skeleton, std::vector<SkMotion*>& motions, std::string pathName, float scale, bool doParseSkeleton, bool doParseMotion)
{
	try 
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) 
	{
		std::string message = "";
		xml_utils::xml_translate(&message, toCatch.getMessage());
		LOG("Error during ParserOgre initialization! :\n %s\n", message.c_str());
		return false;
	}

	XercesDOMParser* parser = new XercesDOMParser();
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true);    // optional

	ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
	parser->setErrorHandler(errHandler);

	bool parseOk = false;

	try 
	{
		std::string filebasename = boost::filesystem::basename(pathName);
		std::string fileextension = boost::filesystem::extension(pathName);
		std::stringstream strstr;
		if (fileextension.size() > 0 && fileextension[0] == '.')
			strstr << filebasename << fileextension;
		else
			strstr << filebasename << "." << fileextension;
		skeleton.name(strstr.str().c_str());
		parser->parse(pathName.c_str());
		DOMDocument* doc = parser->getDocument();

		if (doParseSkeleton)
		{
			DOMNode* skeletonNode = getNode("skeleton", doc);
			if (!skeletonNode)
			{
				// is this a COLLADA file? 
				DOMNode* colladaNode = getNode("library_visual_scenes", doc);
				if (colladaNode)
				{
					LOG("File is a COLLADA file, not an Ogre file. Please use a .dae extension.");
					return false;
				}
				LOG("<skeleton> was not found in file %s.", pathName.c_str());
				return false;
			}
			parseOk = parseSkeleton(skeletonNode, skeleton, pathName, scale);
		}

			
		if (doParseMotion)
		{
			DOMNode* animations = getNode("animations", doc);
			if (!animations)
			{
				// is this a COLLADA file? 
				DOMNode* colladaNode = getNode("library_animations", doc);
				if (colladaNode)
				{
					LOG("File is a COLLADA file, not an Ogre file. Please use a .dae extension.");
					return false;
				}
				LOG("<animations> was not found in file %s. No motions will be loaded.", pathName.c_str());
				return false;
			}
			SkMotion* motion = new SkMotion(); 
			motions.push_back(motion);
			parseOk =  parseMotion(animations, motions, motion, pathName, scale);
		}

		return parseOk;

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
		LOG("Unexpected Exception in ParserOgreSkeleton::parse()");
		return false;
	}

	delete parser;
	delete errHandler;
	return true;
}

DOMNode* ParserOgre::getNode(const std::string& nodeName, DOMNode* node)
{
	int type = node->getNodeType();
	std::string name;
	xml_utils::xml_translate(&name, node->getNodeName());
	std::string value;
	xml_utils::xml_translate(&value, node->getNodeValue());
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

bool ParserOgre::parseSkeleton(DOMNode* skeletonNode, SkSkeleton& skeleton, std::string pathName, float scale)
{
	// get the bone hierarchy
	DOMNode* hierarchy = getNode("bonehierarchy", skeletonNode);
	if (!hierarchy)
	{
		LOG("<hierarchy> was not found in file %s.", pathName.c_str());
		return false;
	}

	std::map<std::string, std::string> parentHierarchy;

	DOMNode* child = NULL;
	const DOMNodeList* list = hierarchy->getChildNodes();
	for (unsigned int i = 0; i < list->getLength(); i++)
	{
		child = list->item(i);
		DOMNamedNodeMap* childAttr = child->getAttributes();
		if (childAttr)
		{
			DOMNode* boneNode = childAttr->getNamedItem(L"bone");
			std::string boneAttr = "";
			if (boneNode)
				xml_utils::xml_translate(&boneAttr, boneNode->getNodeValue());
			DOMNode* parentNode = childAttr->getNamedItem(L"parent");
			std::string parentAttr = "";
			if (parentNode)
				xml_utils::xml_translate(&parentAttr, parentNode->getNodeValue());
			if (boneNode && parentNode)
				parentHierarchy.insert(std::pair<std::string, std::string>(boneAttr, parentAttr));
		}
			
	}

	// get the bones
	DOMNode* bones = getNode("bones", skeletonNode);
	if (!hierarchy)
	{
		LOG("<bones> was not found in file %s.", pathName.c_str());
		return false;
	}
	const DOMNodeList* boneList = bones->getChildNodes();
	for (unsigned int i = 0; i < boneList->getLength(); i++)
	{
		child = boneList->item(i);
		DOMNamedNodeMap* childAttr = child->getAttributes();
		if (childAttr)
		{
			DOMNode* idNode = childAttr->getNamedItem(L"id");
			std::string idAttr = "";
			if (idNode)
				xml_utils::xml_translate(&idAttr, idNode->getNodeValue());
			DOMNode* nameNode = childAttr->getNamedItem(L"name");
			std::string nameAttr = "";
			if (nameNode)
				xml_utils::xml_translate(&nameAttr, nameNode->getNodeValue());

			if (idNode && nameNode)
			{
				int parentId = -1;
				std::map<std::string, std::string>::iterator iter = parentHierarchy.find(nameAttr);
				if (iter != parentHierarchy.end())
				{
					const std::string& parent = (*iter).second;
					SkJoint* parentJoint = skeleton.linear_search_joint(parent.c_str());
					if (!parentJoint)
					{
						LOG("Parent joint %s to joint %s was not found in file %s.", parent.c_str(), nameAttr.c_str(), pathName.c_str());
					}
					else
					{
						parentId = parentJoint->index();
					}
				}

				if (skeleton.joints().size() == 0)
					parentId = -1;
				SkJoint* joint = skeleton.add_joint(SkJoint::TypeQuat, parentId);
				joint->quat()->activate();
				joint->name(nameAttr);
				joint->extName(nameAttr);
				joint->extID(idAttr);
				joint->extSID(idAttr);

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

				const DOMNodeList* transformList = child->getChildNodes();
				for (unsigned int j = 0; j < transformList->getLength(); j++)
				{
					DOMNode* transformNode = transformList->item(j);
					std::string transformNodeName;
					xml_utils::xml_translate(&transformNodeName, transformNode->getNodeName());
					if (transformNodeName == "position")
					{
						DOMNamedNodeMap* positionAttr = transformNode->getAttributes();
						if (positionAttr)
						{
							DOMNode* xNode = positionAttr->getNamedItem(L"x");
							std::string xAttr = "";
							if (xNode)
								xml_utils::xml_translate(&xAttr, xNode->getNodeValue());
							offset.x = (float) atof(xAttr.c_str());
							DOMNode* yNode = positionAttr->getNamedItem(L"y");
							std::string yAttr = "";
							if (yNode)
								xml_utils::xml_translate(&yAttr, yNode->getNodeValue());
							offset.y = (float) atof(yAttr.c_str());
							DOMNode* zNode = positionAttr->getNamedItem(L"z");
							std::string zAttr = "";
							if (zNode)
								xml_utils::xml_translate(&zAttr, zNode->getNodeValue());
							offset.z = (float) atof(zAttr.c_str());
							offset *= scale;
							joint->offset(offset);
						}
					}
					else if (transformNodeName == "rotation")
					{
						float angle;
						DOMNamedNodeMap* rotationAttr = transformNode->getAttributes();
						if (rotationAttr)
						{					
							DOMNode* angleNode = rotationAttr->getNamedItem(L"angle");
							std::string angleAttr = "";
							if (angleNode)
								xml_utils::xml_translate(&angleAttr, angleNode->getNodeValue());
							angle = (float) atof(angleAttr.c_str());
						}
						const DOMNodeList* angleChildList = transformNode->getChildNodes();
						for (unsigned int k = 0; k < angleChildList->getLength(); k++)
						{
							DOMNode* angleChildNode = angleChildList->item(k);
							std::string angleChildNodeName;
							xml_utils::xml_translate(&angleChildNodeName, angleChildNode->getNodeName());
							if (angleChildNodeName == "axis")
							{
								DOMNamedNodeMap* axisAttr = angleChildNode->getAttributes();
								if (axisAttr)
								{
									SrVec axis;
									DOMNode* xNode = axisAttr->getNamedItem(L"x");
									std::string xAttr = "";
									if (xNode)
										xml_utils::xml_translate(&xAttr, xNode->getNodeValue());
									axis.x = (float) atof(xAttr.c_str());
									DOMNode* yNode = axisAttr->getNamedItem(L"y");
									std::string yAttr = "";
									if (yNode)
										xml_utils::xml_translate(&yAttr, yNode->getNodeValue());
									axis.y = (float) atof(yAttr.c_str());
									DOMNode* zNode = axisAttr->getNamedItem(L"z");
									std::string zAttr = "";
									if (zNode)
										xml_utils::xml_translate(&zAttr, zNode->getNodeValue());
									axis.z = (float) atof(zAttr.c_str());
									SrQuat orientation(axis, angle);	
									SkJointQuat* jointQuat = joint->quat();
									jointQuat->prerot(orientation);
								}
							}
						}
					}
				}
			}	
		}
	}

	skeleton.updateGlobalMatricesZero();

	return true;
}


bool ParserOgre::parseMotion(DOMNode* animationsNode, std::vector<SkMotion*>& motions, SkMotion* motion,std::string pathName, float scale)
{
	// many animations might be present. for now, only parse one of them
	DOMNode* animationNode = NULL;
	const DOMNodeList* animationList = animationsNode->getChildNodes();
	bool hasAnimation = false;
	for (unsigned int i = 0; i < animationList->getLength(); i++)
	{
		animationNode = animationList->item(i);
		std::string animationName;
		xml_utils::xml_translate(&animationName, animationNode->getNodeName());
		if (animationName == "animation")
		{
			if (hasAnimation)
			{
				SkMotion* motion2 = new SkMotion();
				motions.push_back(motion2);
				motion = motion2;
			}
			hasAnimation = true;

			// perform animation parsing in two passes. Ogre motion data does not require
			// a fixed number of frames for each channel of data, so this data must be
			// converted first.
			// first pass = establish all keytimes
			// second pass = interpolate and fill in data

			int numKeys = 0;
			std::set<float> times;
			const DOMNodeList* firstPassAnimationChildren = animationNode->getChildNodes();
			for (unsigned int a = 0; a < firstPassAnimationChildren->getLength(); a++)
			{
				DOMNode* firstPassTracksNode = firstPassAnimationChildren->item(a);
				std::string firstPassTracksNodeStr;
				xml_utils::xml_translate(&firstPassTracksNodeStr, firstPassTracksNode->getNodeName());
				if (firstPassTracksNodeStr == "tracks")
				{
					const DOMNodeList* firstPassTracksNodeChildren = firstPassTracksNode->getChildNodes();
					for (unsigned int b = 0; b < firstPassTracksNodeChildren->getLength(); b++)
					{
						DOMNode* firstPassTrackNode = firstPassTracksNodeChildren->item(b);
						std::string firstPassTrackNodeStr;
						xml_utils::xml_translate(&firstPassTrackNodeStr, firstPassTrackNode->getNodeName());
						if (firstPassTrackNodeStr == "track")
						{
							const DOMNodeList* firstPassTrackNodeChildren = firstPassTrackNode->getChildNodes();
							for (unsigned int c = 0; c < firstPassTrackNodeChildren->getLength(); c++)
							{
								DOMNode* firstPassKeyframesNode = firstPassTrackNodeChildren->item(c);
								std::string firstPassKeyframesNodeStr;
								xml_utils::xml_translate(&firstPassKeyframesNodeStr, firstPassKeyframesNode->getNodeName());
								if (firstPassKeyframesNodeStr == "keyframes")
								{
									const DOMNodeList* firstPassKeyframesChildren = firstPassKeyframesNode->getChildNodes();
									for (unsigned int d = 0; d < firstPassKeyframesChildren->getLength(); d++)
									{
										DOMNode* firstPassKeyframeNode = firstPassKeyframesChildren->item(d);
										std::string firstPassKeyframeNodeStr;
										xml_utils::xml_translate(&firstPassKeyframeNodeStr, firstPassKeyframeNode->getNodeName());
										if (firstPassKeyframeNodeStr == "keyframe")
										{
											numKeys++;
											DOMNamedNodeMap* keyframeAttributeList = firstPassKeyframeNode->getAttributes();
											if (keyframeAttributeList)
											{
												DOMNode* timeNode = keyframeAttributeList->getNamedItem(L"time");
												std::string timeStr;
												xml_utils::xml_translate(&timeStr, timeNode->getNodeValue());
												float time = (float) atof(timeStr.c_str());
												times.insert(time);
											}
										}
									}
								}
							}
						}
					}
				}
			}
			std::vector<float> allTimes;
			for (std::set<float>::iterator iter = times.begin(); iter != times.end(); iter++)
				allTimes.push_back(*iter);
			
			std::sort(allTimes.begin(), allTimes.end());

			int numKeyFrames = allTimes.size();
			std::vector<float*> allData;

			int counter = 0;
			std::map<float, int> timeMap;
			for (std::vector<float>::iterator iter = allTimes.begin();
				 iter != allTimes.end();
				 iter++)
			{
				timeMap.insert(std::pair<float, int>((*iter), counter));
				counter++;
			}

			SkChannelArray motionChannels;

			// second pass, parsing the data
			DOMNamedNodeMap* animationAttributeList = animationNode->getAttributes();
			if (animationAttributeList)
			{
				DOMNode* nameNode = animationAttributeList->getNamedItem(L"name");
				std::string animationName;
				xml_utils::xml_translate(&animationName, nameNode->getNodeValue());
				motion->setName(animationName);
				DOMNode* lengthNode = animationAttributeList->getNamedItem(L"length");
				std::string lengthStr;
				xml_utils::xml_translate(&lengthStr, lengthNode->getNodeValue());
				float length = (float) atof(lengthStr.c_str());

				const DOMNodeList* tracksList = animationNode->getChildNodes();
				for (unsigned int j = 0; j < tracksList->getLength(); j++)
				{
					DOMNode* tracksNode = tracksList->item(j);
					std::string tracksStr;
					xml_utils::xml_translate(&tracksStr, tracksNode->getNodeName());
					if (tracksStr == "tracks")
					{
						const DOMNodeList* tracksList = tracksNode->getChildNodes();
						for (unsigned int k = 0; k < tracksList->getLength(); k++)
						{
							DOMNode* trackNode = tracksList->item(k);
							std::string trackStr;
							xml_utils::xml_translate(&trackStr, trackNode->getNodeName());
							if (trackStr == "track")
							{
								std::vector<float> vecTime(numKeyFrames, 0);
								std::vector<float> vecTranslateX(numKeyFrames, 0);
								std::vector<float> vecTranslateY(numKeyFrames, 0);
								std::vector<float> vecTranslateZ(numKeyFrames, 0);
								std::vector<SrVec> vecAxis(numKeyFrames);
								SrVec vec(0, 1, 0);
								for (size_t v = 0; v < (size_t) numKeyFrames; v++)
									vecAxis[v] = vec;
								std::vector<float> vecAngle(numKeyFrames, 0);
								int firstKey = -1;
								int lastKey = -1;

								std::vector<bool> vecKey(numKeyFrames, false);

								std::string trackBoneStr;
								DOMNamedNodeMap* trackAttributeList = trackNode->getAttributes();
								if (trackAttributeList)
								{
									DOMNode* trackBoneNode = trackAttributeList->getNamedItem(L"bone");
									xml_utils::xml_translate(&trackBoneStr, trackBoneNode->getNodeValue());
									if (trackBoneStr == "")
										continue;
									// .............................
									const DOMNodeList* keyframesList = trackNode->getChildNodes();
									for (unsigned int l = 0; l < keyframesList->getLength(); l++)
									{
										DOMNode* keyframesNode = keyframesList->item(l);
										std::string keyframesStr;
										xml_utils::xml_translate(&keyframesStr, keyframesNode->getNodeName());
										if (keyframesStr == "keyframes")
										{										
											const DOMNodeList* keyframeList = keyframesNode->getChildNodes();
											for (unsigned int m = 0; m < keyframeList->getLength(); m++)
											{
												DOMNode* keyframeNode = keyframeList->item(m);
												std::string keyframeStr;
												xml_utils::xml_translate(&keyframeStr, keyframeNode->getNodeName());
												if (keyframeStr == "keyframe")
												{
													DOMNamedNodeMap* keyframeAttributeList = keyframeNode->getAttributes();
													if (keyframeAttributeList)
													{
														DOMNode* timeNode = keyframeAttributeList->getNamedItem(L"time");
														std::string timeStr;
														xml_utils::xml_translate(&timeStr, timeNode->getNodeValue());
														float time = (float) atof(timeStr.c_str());
														// determine the index of this key
														std::map<float, int>::iterator keyIndexIter = timeMap.find(time);
														int keyIndex = (*keyIndexIter).second;
														
														vecKey[keyIndex] = true;
														if (firstKey == -1)
															firstKey = keyIndex;
														lastKey = keyIndex;

														vecTime.push_back(time); 

														SrVec translation;
														SrVec axis;
														float angle = 0.f;
														const DOMNodeList* transformList = keyframeNode->getChildNodes();
														for (unsigned int n = 0; n < transformList->getLength(); n++)
														{
															DOMNode* transformNode = transformList->item(n);
															std::string transformNodeName;
															xml_utils::xml_translate(&transformNodeName, transformNode->getNodeName());
															if (transformNodeName == "translate")
															{
																DOMNamedNodeMap* positionAttr = transformNode->getAttributes();
																if (positionAttr)
																{
																	DOMNode* xNode = positionAttr->getNamedItem(L"x");
																	std::string xAttr = "";
																	if (xNode)
																		xml_utils::xml_translate(&xAttr, xNode->getNodeValue());
																	translation.x = (float) atof(xAttr.c_str());
																	DOMNode* yNode = positionAttr->getNamedItem(L"y");
																	std::string yAttr = "";
																	if (yNode)
																		xml_utils::xml_translate(&yAttr, yNode->getNodeValue());
																	translation.y = (float) atof(yAttr.c_str());
																	DOMNode* zNode = positionAttr->getNamedItem(L"z");
																	std::string zAttr = "";
																	if (zNode)
																		xml_utils::xml_translate(&zAttr, zNode->getNodeValue());
																	translation.z = (float) atof(zAttr.c_str());

																	vecTranslateX[keyIndex] = translation.x;
																	vecTranslateY[keyIndex] = translation.y;
																	vecTranslateZ[keyIndex] = translation.z;
																}
															}
															else if (transformNodeName == "rotate")
															{
																float angle;
																DOMNamedNodeMap* rotationAttr = transformNode->getAttributes();
																if (rotationAttr)
																{			
																	DOMNode* angleNode = rotationAttr->getNamedItem(L"angle");
																	std::string angleAttr = "";
																	if (angleNode)
																		xml_utils::xml_translate(&angleAttr, angleNode->getNodeValue());
																	angle = (float) atof(angleAttr.c_str());
																}
																
																SrVec axis;
																const DOMNodeList* angleChildList = transformNode->getChildNodes();
																for (unsigned int o = 0; o < angleChildList->getLength(); o++)
																{
																	DOMNode* angleChildNode = angleChildList->item(o);
																	std::string angleChildNodeName;
																	xml_utils::xml_translate(&angleChildNodeName, angleChildNode->getNodeName());
																	if (angleChildNodeName == "axis")
																	{
																		DOMNamedNodeMap* axisAttr = angleChildNode->getAttributes();
																		if (axisAttr)
																		{																			
																			DOMNode* xNode = axisAttr->getNamedItem(L"x");
																			std::string xAttr = "";
																			if (xNode)
																				xml_utils::xml_translate(&xAttr, xNode->getNodeValue());
																			axis.x = (float) atof(xAttr.c_str());
																			DOMNode* yNode = axisAttr->getNamedItem(L"y");
																			std::string yAttr = "";
																			if (yNode)
																				xml_utils::xml_translate(&yAttr, yNode->getNodeValue());
																			axis.y = (float) atof(yAttr.c_str());
																			DOMNode* zNode = axisAttr->getNamedItem(L"z");
																			std::string zAttr = "";
																			if (zNode)
																				xml_utils::xml_translate(&zAttr, zNode->getNodeValue());
																			axis.z = (float) atof(zAttr.c_str());																						
																		}
																	}
																}
																vecAxis[keyIndex] = axis;
																vecAngle[keyIndex] = angle;
															}
														}
													}
												}
											}
										}
									}
								}
								if (trackBoneStr == "")
									continue;

								// interpolate any missing data
								if (numKeys > 0)
								{
									int leftKey = -1;
									int rightKey = -1;
									int lastLeftKey = -1;
									for (size_t b = 0; b < vecKey.size(); b++)
									{
										if (vecKey[b] == false)
										{
											float curTime = allTimes[b];
											// get the first right key
											for (size_t c = b + 1; c < (size_t) vecKey.size(); c++)
											{
												if (vecKey[c] == true)
												{
													rightKey = c;
													break;
												}
											}
											if (rightKey == -1)
											{
												// no right key
												// replicate all data from the first left key
												for (size_t f = b; f < vecKey.size(); f++)
												{
													vecTranslateX[f] = vecTranslateX[leftKey];
													vecTranslateY[f] = vecTranslateY[leftKey];
													vecTranslateZ[f] = vecTranslateZ[leftKey];
													vecAxis[f] = vecAxis[leftKey];
													vecAngle[f] = vecAngle[leftKey];
												}
												break;
											}
											// get the left key
											if (leftKey == -1)
											{
												// no left key, use right key value
												for (size_t f = b - 1; f >= 0; f++)
												{
													vecTranslateX[f] = vecTranslateX[rightKey];
													vecTranslateY[f] = vecTranslateY[rightKey];
													vecTranslateZ[f] = vecTranslateZ[rightKey];
													vecAxis[f] = vecAxis[rightKey];
													vecAngle[f] = vecAngle[rightKey];
												}
												break;
											}
											// interpolate between left and right keys
											float leftTime = allTimes[leftKey];
											float rightTime = allTimes[rightKey];
											float denomTime = rightTime - leftTime;
											float interp = 1.0f;
											if (fabs(denomTime) < .001)
											{
												interp = 1.0f;
											}
											else
											{
												interp = (curTime - leftTime) / (rightTime - leftTime);
											}

											vecTranslateX[b] = (1.0f - interp) * vecTranslateX[leftKey] + interp * vecTranslateX[rightKey];
											vecTranslateY[b] = (1.0f - interp) * vecTranslateY[leftKey] + interp * vecTranslateY[rightKey];
											vecTranslateZ[b] = (1.0f - interp) * vecTranslateZ[leftKey] + interp * vecTranslateZ[rightKey];;

											SrQuat leftQuat(vecAxis[leftKey], vecAngle[leftKey]);
											SrQuat rightQuat(vecAxis[rightKey], vecAngle[rightKey]);
											SrQuat interpQuat = slerp(leftQuat, rightQuat, interp);
											vecAxis[b] = interpQuat.axis();
											vecAngle[b] = interpQuat.angle();											
										}
										else
										{
											leftKey = b;
										}
									}
								}
								
								// finished with the track
								// add a frame for that channel							
								motionChannels.add(trackBoneStr, SkChannel::XPos);
								motionChannels.add(trackBoneStr, SkChannel::YPos);
								motionChannels.add(trackBoneStr, SkChannel::ZPos);
								motionChannels.add(trackBoneStr, SkChannel::Quat);

								float* data = NULL;
								size_t xsize = vecTranslateX.size();
								data = new float[xsize];
								for (size_t t = 0; t < xsize; t++)
									data[t] = vecTranslateX[t] * scale;
								allData.push_back(data);

								size_t ysize = vecTranslateY.size();
								data = new float[ysize];
								for (size_t  t = 0; t < ysize; t++)
									data[t] = vecTranslateY[t] * scale;
								allData.push_back(data);

								size_t zsize = vecTranslateZ.size();
								data = new float[zsize];
								for (size_t t = 0; t < zsize; t++)
									data[t] = vecTranslateZ[t] * scale;
								allData.push_back(data);

								size_t anglesize = vecAngle.size();
								size_t axisSize = vecAxis.size();
								data = new float[4 * anglesize];
								for (size_t t = 0; t < anglesize; t++)
								{
									SrQuat quat(vecAxis[t], vecAngle[t]);
									data[t * 4] = quat.w;
									data[t * 4 + 1] = quat.x;
									data[t * 4 + 2] = quat.y;
									data[t * 4 + 3] = quat.z;
								}
								allData.push_back(data);
							}
						}
					}
				}

				motion->init(motionChannels);

				int counter = 0;
				for (std::vector<float>::iterator iter = allTimes.begin();
					 iter != allTimes.end();
					 iter++)
				{
					// set the key times
					motion->insert_frame(counter, (*iter));
					counter++;
				}

				// add all the data to the motion
				for (size_t f = 0; f < (size_t) numKeyFrames; f++)
				{
					float* posture = motion->posture(f);	
					int psize = motion->posture_size();
					int counter = 0;
					for (size_t x = 0; x < allData.size(); x++)
					{
						int stage = x % 4;
						if (stage < 3)
						{
							posture[counter] = allData[x][f];
							counter++;
						}
						else
						{
							posture[counter] = allData[x][f * 4];
							posture[counter + 1] = allData[x][f  * 4 + 1];
							posture[counter + 2] = allData[x][f  * 4 + 2];
							posture[counter + 3] = allData[x][f  * 4 + 3];
							counter += 4;
						}
					}
				}
			}
			else
			{
				LOG("Animation in file %s has no name or length, will not be parsed.", pathName.c_str());
				return false;
			}
		}
	}
	return true;
}
