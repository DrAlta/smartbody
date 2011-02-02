/**************************************************
Copyright 2005 by Ari Shapiro and Petros Faloutsos

DANCE
Dynamic ANimation and Control Environment

 ***************************************************************
 ******General License Agreement and Lack of Warranty ***********
 ****************************************************************

This software is distributed for noncommercial use in the hope that it will 
be useful but WITHOUT ANY WARRANTY. The author(s) do not accept responsibility
to anyone for the consequences	of using it or for whether it serves any 
particular purpose or works at all. No warranty is made about the software 
or its performance. Commercial use is prohibited. 

Any plugin code written for DANCE belongs to the developer of that plugin,
who is free to license that code in any manner desired.

Content and code development by third parties (such as FLTK, Python, 
ImageMagick, ODE) may be governed by different licenses.
You may modify and distribute this software as long as you give credit 
to the original authors by including the following text in every file 
that is distributed: */

/*********************************************************
	Copyright 2005 by Ari Shapiro and Petros Faloutsos

	DANCE
	Dynamic ANimation and Control Environment
	-----------------------------------------
	AUTHOR:
		Ari Shapiro (ashapiro@cs.ucla.edu)
	ORIGINAL AUTHORS: 
		Victor Ng (victorng@dgp.toronto.edu)
		Petros Faloutsos (pfal@cs.ucla.edu)
	CONTRIBUTORS:
		Yong Cao (abingcao@cs.ucla.edu)
		Paco Abad (fjabad@dsic.upv.es)
**********************************************************/

#include "vhcl_log.h"
#include "ParserBVH.h"
#include <iostream>
#include <stack>
#include <sstream>
#include <cstdlib>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

enum {XPOSITION, YPOSITION, ZPOSITION, XROTATION, YROTATION, ZROTATION};

void ParserBVH::parse(SkSkeleton& skeleton, SkMotion& motion, std::string name, std::ifstream &file, int N1, int N2)
{
	// check to make sure we have properly opened the file
	if (!file.good())
	{
		LOG("Could not open file\n");
		return;
	}
	char line[8192];
	skeleton.name(name.c_str());
	int state = 0;
	char* str = NULL;
	stack<SkJoint*> stack;
	SkJoint* cur = NULL;
	int numFrames = 0;
	int curFrame = -1;
	double frameTime = 0;
	int jointCounter = 0;
	int foundRoot = 0; // 0 = root not found, 1 = root found, 2 = next joint found

	SkChannelArray& skChannels = skeleton.channels(); 

	while(!file.eof() && file.good())
	{
		file.getline(line, 8192, '\n');
                // remove any trailing \r
                if (line[strlen(line) - 1] == '\r')
                        line[strlen(line) - 1] = '\0';
		if (strlen(line) == 0) // ignore blank lines
			continue;
		switch (state)
		{
			case 0:	// looking for 'HIERARCHY'
				str = strtok(line, " \t");	
				if (strncmp(str, "HIERARCHY", strlen("HIERARCHY")) == 0)
					state = 1;
				else
				{
					LOG("HIERARCHY not found...\n");
					file.close();
					return;
				}
				break;
			case 1:	// looking for 'ROOT'
				str = strtok(line, " \t");	
				if (str != NULL && strncmp(str, "ROOT", strlen("ROOT")) == 0)
				{
					str = strtok(NULL, " \t");
					if (str != NULL)
					{
						std::string trimmedname = str;
						boost::trim(trimmedname);
						std::stringstream strstr;
						strstr << trimmedname;
						SkJoint* root = skeleton.add_joint(SkJoint::TypeQuat);	
						root->quat()->activate();
						jointCounter++;
						root->name(SkJointName(strstr.str().c_str()));
						skeleton.make_active_channels();
						cur = root;
						state = 2;
					}
					else
					{
						LOG("ROOT name not found...\n");
						file.close();
						return;
					}
				}
				else
				{
					LOG("ROOT not found...\n");
					file.close();
					return;
				}
				break;
			case 2: // looking for '{'
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "{", 1) == 0)
				{
					stack.push(cur);
					state = 3;
				}
				else
				{
					LOG("{ not found...\n");
					file.close();
					return;
				}
				break;
			case 3: // looking for 'OFFSET'
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "OFFSET", strlen("OFFSET")) == 0)
				{
					if (foundRoot == 0)
						foundRoot = 1;
					else if (foundRoot == 1)
						foundRoot = 2;
					double x = 0; double y = 0; double z = 0;
					str = strtok(NULL, " \t");
					x = atof(str);
					str = strtok(NULL, " \t");
					y = atof(str);
					str = strtok(NULL, " \t");
					z = atof(str);
					cur->offset(SrVec(float(x), float(y), float(z)));
					//cout << "Found offset of " << x << " " << y << " " << z << " " << endl;
					state = 4;
				}
				else
				{
					LOG("OFFSET not found...\n");
					file.close();
					return;
				}
				break;
			case 4: // looking for 'CHANNELS'
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "CHANNELS", strlen("CHANNELS")) == 0)
				{
					str = strtok(NULL, " \t");
					int numChannels = atoi(str);

					// make sure that only the root has > 3 channels
					if (numChannels > 3 && foundRoot == 2)
					{
						//LOG("Too many channels (%d) found for non-root node at %s, reducing to %d...", numChannels, cur->getName(), numChannels - 3);
						//cur->setIgnoreChannels(true);
					}
					int channels[6] = {0};
					//LOG("Found %d channels...\n", numChannels);
					bool hasRotation = false;
					for (int c = 0; c < numChannels; c++)
					{
						str = strtok(NULL, " \t");
						if (strncmp(str, "Xrotation", strlen("Xrotation")) == 0)
						{
							channels[c] = XROTATION;
						}
						else if (strncmp(str, "Yrotation", strlen("Yrotation")) == 0)
						{
							channels[c] = YROTATION;							
						}
						else if (strncmp(str, "Zrotation", strlen("Zrotation")) == 0)
						{
							channels[c] = ZROTATION;							
						}
						else if (strncmp(str, "Xposition", strlen("Xposition")) == 0)
						{
							channels[c] = XPOSITION;		
							skChannels.add(cur, SkChannel::XPos);
						}
						else if (strncmp(str, "Yposition", strlen("Yposition")) == 0)
						{
							channels[c] = YPOSITION;		
							skChannels.add(cur, SkChannel::YPos);
						}
						else if (strncmp(str, "Zposition", strlen("Zposition")) == 0)
						{
							channels[c] = ZPOSITION;	
							skChannels.add(cur, SkChannel::ZPos);
						}
						else
						{
							LOG("Unknown channel: %s...\n", str);;
							file.close();
						}
						if (!hasRotation)
						{
							hasRotation = true;
							skChannels.add(cur, SkChannel::Quat);
						}
					}

					//if (cur->isIgnoreChannels())
					//{
					//	for (int i = 0; i < 3; i++)
					//		channels[i] = channels[i + 3];
					//	numChannels -= 3;
					//}

					// convert the BVH channels to SmartBody channels
					// TODO: Need to preserve channel ordering so that data can be parsed properly
					//cur->setChannels(numChannels, channels);
					state = 5;
				}
				else
				{
					LOG("CHANNELS not found...\n");;
					file.close();
					return;
				}
				break;
			case 5: // looking for 'JOINT' or 'End Site' or '}' or 'MOTION'
				str = strtok(line, " \t");
				if (strncmp(str, "JOINT", strlen("JOINT")) == 0)
				{
					str = strtok(NULL, "");
					if (str != NULL)
					{
						std::string trimmedname = str;
						boost::trim(trimmedname);
						SkJoint* parent = stack.top();
						SkJoint* joint = skeleton.add_joint(SkJoint::TypeQuat, parent->index());
						joint->quat()->activate();
						jointCounter++;
						joint->name(SkJointName(trimmedname.c_str()));
						skeleton.make_active_channels();
						cur = joint;
						//cout << "Found joint " << str << endl;
						state = 2;
					}
					else
					{
						LOG("ROOT name not found...\n");;
						file.close();
						return;
					}
				}
				else if (strncmp(str, "End", strlen("End")) == 0)
				{
					str = strtok(NULL, " \t");
					if (strncmp(str, "Site", strlen("Site")) == 0)
					{
						state = 6;
					}
					else
					{
						LOG("End site not found...\n");;
						file.close();
						return;
					}
				}
				else if (strncmp(str, "}", 1) == 0)
				{
					str = strtok(line, " \t");
					if (str != NULL && strncmp(str, "}", 1) == 0)
					{
						stack.pop();
						state = 5;
					}
					else
					{
						LOG("} not found...\n");;
						file.close();
						return;
					}
				}
				else if (strncmp(str, "MOTION", strlen("MOTION")) == 0)
				{
					state = 9;
				}
				else
				{
					LOG("JOINT or End Site not found...\n");;
					file.close();
					return;
				}
				break;
			case 6: // looking for 'OFFSET' within end effector
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "{", 1) == 0)
				{
					state = 7;
				}
				else
				{
					LOG("{ not found for end effector...\n");;
					cerr << "{ not found for end effector..." << endl;
					file.close();
					return;
				}
				break;
			case 7:
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "OFFSET", strlen("OFFSET")) == 0)
				{
					double x = 0; double y = 0; double z = 0;
					str = strtok(NULL, " \t");
					x = atof(str);
					str = strtok(NULL, " \t");
					y = atof(str);
					str = strtok(NULL, " \t");
					z = atof(str);
					cur->endEffectorOffset(SrVec(float(x), float(y), float(z)));
					//cur->setEndEffector(true);
					//LOG("Found end effector at %s", cur->getName());
					//cout << "Found end effector offset of " << x << " " << y << " " << z << " " << endl;
					state = 8;
				}
				else
				{
					LOG("End effector OFFSET not found...\n");;
					file.close();
					return;
				}
				break;
			case 8: // looking for '}' to finish the  end effector
				str = strtok(line, " \t");
				if (str != NULL && strncmp(str, "}", 1) == 0)
				{
					state = 5;
				}
				else
				{
					LOG("} not found for end effector...\n");;
					file.close();
					return;
				}
				break;
			case 9: // found 'MOTION', looking for 'Frames'
				str = strtok(line, ":");
				if (str != NULL && strncmp(str, "Frames", strlen("Frames")) == 0)
				{
					str = strtok(NULL, " \t");
					numFrames = atoi(str);
					//LOG("Found %d frames of animation...\n", numFrames);
					state = 10;
				}
				else
				{
					LOG("Frames: not found...\n");;
					file.close();
					return;
				}
				break;
			case 10: // found 'Frames', looking for 'Frame time:'
				str = strtok(line, ":");
				if (str != NULL && strncmp(str, "Frame Time", strlen("Frame Time")) == 0)
				{
					str = strtok(NULL, " \t");
					frameTime = atof(str);
					//LOG("Frame time is %f...\n", frameTime);
					//curFrame = 0;
					state = 11;
				}
				else
				{
					LOG("Frame Time: not found...\n");;
					file.close();
					return;
				}
				break;
			case 11: // parsing 
				// TODO:
				state = 50;
				break;
				curFrame++;
				if( (curFrame <= N2) && (curFrame >= N1))
				{
					if (curFrame < numFrames)
					{
						int index = 0;
						str = strtok(line, " \t");
						SkJoint* oldJoint = NULL;
						double frames[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
						// clean up any line feeds or carriage returns
						while (str != NULL && str[0] != 13)
						{
							double val = atof(str);
							int channelNum;

							SkJoint* j = NULL;//skeleton.getSkJoint(index, channelNum);
							if (j != oldJoint)
							{
								// add the values to the old joint
								if (oldJoint != NULL)
								{
									if (0)//oldJoint->isIgnoreChannels())
									{
										// shift the frame values down by three
										for (int i = 0; i < 3; i++)
										{
											frames[i] = frames[i + 3];
										}
										index -= 3;
									}
									// TODO: Add data to SkMotion
									//oldJoint->addFrame(frames);
								}

								for (int x = 0; x < 6; x++)
									frames[x] = 0.0;
								oldJoint = j;
							}

							frames[channelNum] = val;
							if (j != NULL)
							{
								// TODO: set the frame timing
								//j->setFrameTime(frameTime);
							}
							else
							{
								LOG("No joint found for index %d and channel #%d on frame %d...\n", index, channelNum, curFrame);
							}
							index++;
						
							str = strtok(NULL, " \t");
						}

						// flush any values to the old joint
						if (oldJoint != NULL)
						{
							// TODO: set the frame timing
							//oldJoint->addFrame(frames);
						}
						state = 11;
					}
					else
					{
						state = 12;
					}
				}

				break;
			case 12: 
				state = 50;
				break;
			
			case 50:
				//LOG("Finished parsing motion with %d frames...", numFrames);
				file.close();
				//skeleton.recalculateJointList();
				//skeleton.calculateMatrices(0);	
				skChannels.compress();
				return;
			default:
				LOG("State %d not expected.");
				file.close();
				return;
		}
	}
	return;
}
