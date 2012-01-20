
#include "vhcl.h"

#include "SbmDebuggerClient.h"

#include <string>
#include <vector>

#include <stdio.h>

#include "vhmsg-tt.h"

#include "SbmDebuggerCommon.h"


using std::string;
using std::vector;


SbmDebuggerClient::SbmDebuggerClient()
{
}

SbmDebuggerClient::~SbmDebuggerClient()
{
}

void SbmDebuggerClient::QuerySbmProcessIds()
{
   m_processIdList.clear();
   vhmsg::ttu_notify1("sbmdebugger queryids");
}

void SbmDebuggerClient::Connect(const string & id)
{
   m_sbmId = id;
   m_connectResult = false;
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s connect", m_sbmId.c_str()).c_str());
}

void SbmDebuggerClient::Disconnect()
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s disconnect", m_sbmId.c_str()).c_str());
   m_sbmId = "";
   m_connectResult = false;
}

void SbmDebuggerClient::Init()
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s send_init", m_sbmId.c_str()).c_str());
}

void SbmDebuggerClient::Update()
{
}

void SbmDebuggerClient::StartUpdates(double updateFrequencyS)
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s start_update %f", m_sbmId.c_str(), updateFrequencyS).c_str());
}

void SbmDebuggerClient::EndUpdates()
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s end_update", m_sbmId.c_str()).c_str());
}

void SbmDebuggerClient::ProcessVHMsgs(const char * op, const char * args)
{
   string message = string(op) + " " + string(args);
   vector<string> split;
   vhcl::Tokenize(message, split, " \t\n");

   if (split.size() > 0)
   {
      if (split[0] == "sbmdebugger")
      {
         if (split.size() > 1)
         {
            if (split.size() > 2)
            {
               if (split[2] == "id")
               {
                  m_processIdList.push_back(split[1]);
               }
            }

            if (split[1] == m_sbmId)
            {
               if (split.size() > 2)
               {
                  if (split[2] == "connect_success")
                  {
                     m_connectResult = true;
                  }
                  else if (split[2] == "init")
                  {
                     // handle init
                     Character character;

                     if (split.size() > 4)
                     {
                        string character_string = split[3];
                        string name = split[4];

                        character.m_name = name;
                     }

                     if (split.size() > 6)
                     {
                        string bones_string = split[5];
                        int numBones = vhcl::ToInt(split[6]);
                     }

                     Joint * parent = NULL;
                     for (size_t i = 7; i < split.size(); i++)
                     {
                        if (split[i] == "{")
                        {
                           string name = split[i + 1];
                           string pos  = split[i + 2];
                           double posX = vhcl::ToDouble(split[i + 3]);
                           double posY = vhcl::ToDouble(split[i + 4]);
                           double posZ = vhcl::ToDouble(split[i + 5]);
                           string prerot  = split[i + 6];
                           double rotX = vhcl::ToDouble(split[i + 7]);
                           double rotY = vhcl::ToDouble(split[i + 8]);
                           double rotZ = vhcl::ToDouble(split[i + 9]);
                           double rotW = vhcl::ToDouble(split[i + 10]);
                           i += 10;

                           Joint * j = new Joint();
                           j->m_name = name;
                           j->posOrig.x = posX;
                           j->posOrig.y = posY;
                           j->posOrig.z = posZ;
                           j->rotOrig.x = rotX;
                           j->rotOrig.y = rotY;
                           j->rotOrig.z = rotZ;
                           j->rotOrig.w = rotW;
                           j->m_parent = parent;

                           if (parent == NULL)
                           {
                              character.m_joints.push_back(j);
                              parent = *(character.m_joints.end() - 1);
                           }
                           else
                           {
                              parent->m_joints.push_back(j);
                              parent = *(parent->m_joints.end() - 1);
                           }
                        }
                        else if (split[i] == "}")
                        {
                           if (parent != NULL)
                           {
                              parent = parent->m_parent;
                           }
                        }
                     }


                     m_scene.m_characters.push_back(character);


                     //printf("Init Received\n");
                     //printf(message.c_str());
                     //printf("\n");
                  }
                  else if (split[2] == "update")
                  {
                     // handle updates

                     if (split.size() > 3)
                     {
                        if (split[3] == "character")
                        {
                           Character * character = NULL;

                           if (split.size() > 4)
                           {
                              string character_string = split[3];
                              string name = split[4];

                              for (size_t i = 0; i < m_scene.m_characters.size(); i++)
                              {
                                 Character * c = &m_scene.m_characters[i];
                                 if (c->m_name == name)
                                 {
                                    character = c;
                                 }
                              }
                           }

                           if (character)
                           {
                              if (split.size() > 5)
                              {
                                 /*if (split[5] == "position")
                                 {
                                    if (split.size() > 12)
                                    {
                                       double posX = vhcl::ToDouble(split[6]);
                                       double posY = vhcl::ToDouble(split[7]);
                                       double posZ = vhcl::ToDouble(split[8]);
                                       double rotX = vhcl::ToDouble(split[9]);
                                       double rotY = vhcl::ToDouble(split[10]);
                                       double rotZ = vhcl::ToDouble(split[11]);
                                       double rotW = vhcl::ToDouble(split[12]);

                                       character->pos.x = posX;
                                       character->pos.y = posY;
                                       character->pos.z = posZ;
                                       character->rot.x = rotX;
                                       character->rot.y = rotY;
                                       character->rot.z = rotZ;
                                       character->rot.w = rotW;
                                    }
                                 }
                                 else*/ if (split[5] == "bones")
                                 {
                                    if (split.size() > 6)
                                    {
                                       int numBones = vhcl::ToInt(split[6]);
                                    }

                                    for (size_t i = 7; i < split.size(); i++)
                                    {
                                       string name = split[i];
                                       string pos  = split[i + 1];
                                       double posX = vhcl::ToDouble(split[i + 2]);
                                       double posY = vhcl::ToDouble(split[i + 3]);
                                       double posZ = vhcl::ToDouble(split[i + 4]);
                                       string rot  = split[i + 5];
                                       double rotX = vhcl::ToDouble(split[i + 6]);
                                       double rotY = vhcl::ToDouble(split[i + 7]);
                                       double rotZ = vhcl::ToDouble(split[i + 8]);
                                       double rotW = vhcl::ToDouble(split[i + 9]);
                                       i += 9;

                                       Joint * joint = character->FindJoint(name);
                                       if (joint)
                                       {
                                          joint->pos.x = posX;
                                          joint->pos.y = posY;
                                          joint->pos.z = posZ;
                                          joint->rot.x = rotX;
                                          joint->rot.y = rotY;
                                          joint->rot.z = rotZ;
                                          joint->rot.w = rotW;
                                       }
                                    }
                                 }
                              }
                           }
                        }
                        else if (split[3] == "camera")
                        {
                           int i = 4;
                           string pos  = split[i];
                           double posX = vhcl::ToDouble(split[i + 1]);
                           double posY = vhcl::ToDouble(split[i + 2]);
                           double posZ = vhcl::ToDouble(split[i + 3]);
                           string rot  = split[i + 4];
                           double rotX = vhcl::ToDouble(split[i + 5]);
                           double rotY = vhcl::ToDouble(split[i + 6]);
                           double rotZ = vhcl::ToDouble(split[i + 7]);
                           double rotW = vhcl::ToDouble(split[i + 8]);
                           string persp  = split[i + 9];
                           double fovY   = vhcl::ToDouble(split[i + 10]);
                           double aspect = vhcl::ToDouble(split[i + 11]);
                           double zNear  = vhcl::ToDouble(split[i + 12]);
                           double zFar   = vhcl::ToDouble(split[i + 13]);

                           m_scene.m_camera.pos.x = posX;
                           m_scene.m_camera.pos.y = posY;
                           m_scene.m_camera.pos.z = posZ;
                           m_scene.m_camera.rot.x = rotX;
                           m_scene.m_camera.rot.y = rotY;
                           m_scene.m_camera.rot.z = rotZ;
                           m_scene.m_camera.rot.w = rotW;
                           m_scene.m_camera.fovY   = fovY;
                           m_scene.m_camera.aspect = aspect;
                           m_scene.m_camera.zNear  = zNear;
                           m_scene.m_camera.zFar   = zFar;
                        }
                     }


                     //printf("Update Received\n");
                     //printf(message.c_str());
                     //printf("\n");
                  }
               }
            }
         }
      }
   }
}
