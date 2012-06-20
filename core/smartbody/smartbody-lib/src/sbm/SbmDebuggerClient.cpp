
#include "vhcl.h"
#include "vhcl_socket.h"

#include "SbmDebuggerClient.h"
#include "SbmDebuggerUtility.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "vhmsg-tt.h"

#include <sb/SBScene.h>

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


void * m_sockTCP_client = NULL;


void SbmDebuggerClient::Connect(const string & id)
{
   m_sbmId = id;
   m_connectResult = false;
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s connect", m_sbmId.c_str()).c_str());

   // id == host:port:friendlyName
   vector<string> idSplit;
   vhcl::Tokenize(id, idSplit, ":");
   const char * server = idSplit[0].c_str();
   int port = vhcl::ToInt(idSplit[1]);


   bool ret = vhcl::SocketStartup();
   if (!ret)
   {
      printf("SocketStartup() failed\n");
   }


   m_sockTCP_client = vhcl::SocketOpenTcp();
   if (m_sockTCP_client == NULL)
   {
      printf( "SocketOpenTcp() failed\n" );
      vhcl::SocketShutdown();
      return;
   }

   ret = vhcl::SocketConnect(m_sockTCP_client, server, port);
   if (!ret)
   {
      printf( "SocketConnect() failed\n" );
      vhcl::SocketClose(m_sockTCP_client);
      vhcl::SocketShutdown();
      return;
   }
}

void SbmDebuggerClient::Disconnect()
{
   if ( m_sockTCP_client )
   {
      vhcl::SocketClose(m_sockTCP_client);
      m_sockTCP_client = NULL;
   }

   vhcl::SocketShutdown();


   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s disconnect", m_sbmId.c_str()).c_str());
   m_processIdList.clear();
   m_sbmId = "";
   m_connectResult = false;
}

void SbmDebuggerClient::Init()
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s send_init", m_sbmId.c_str()).c_str());
}






string m_tcpData;
int m_tcpDataCount = 0;

void SbmDebuggerClient::Update()
{
   {
      void * s = m_sockTCP_client;

      int tcpDataPending;
      tcpDataPending = vhcl::SocketIsDataPending(s);

      while ( tcpDataPending )
      {
         tcpDataPending = 0;

         // TODO - preallocate this
         char str[ 1000 ] = { 0 };

         int bytesReceived = vhcl::SocketReceive(s, str, sizeof( str ) - 1);
         if ( bytesReceived > 0 )
         {
            m_tcpData.append(str);
            m_tcpDataCount += m_tcpData.length();

            //printf("TCP %d\n", m_tcpDataCount);

            tcpDataPending = vhcl::SocketIsDataPending(s);
         }

         // sanity check for spamming too much data
         if (m_tcpData.length() > 500000)
         {
            //static int i = 0;
            //printf("TCP Overflow %d - %d\n", i++);
            tcpDataPending = 0;
         }
      }


      // if m_tcpData contains a ;, then we have a full command, extract it and process it

      size_t marker = m_tcpData.find_first_of(';');
      while (marker != string::npos)
      {
         string command = m_tcpData.substr(0, marker);
         m_tcpData.erase(0, marker + 1);


         vector< string > msgTokens;
         vhcl::Tokenize( command, msgTokens, "|" );

         vector<string> split;
         vhcl::Tokenize(command, split, " \t\n");

         if ( msgTokens.size() > 0 )
         {
            if ( msgTokens[ 0 ] == "CreateActor" )
            {
               if ( msgTokens.size() > 4 )
               {
                  string charIdStr = msgTokens[ 1 ];
                  int charId       = atoi( charIdStr.c_str() );
                  string uClass    = msgTokens[ 2 ];
                  string name      = msgTokens[ 3 ];
                  int skeletonType = atoi( msgTokens[ 4 ].c_str() );

                  //if ( m_onCreateCharacterFunc )
                  {
                     //m_onCreateCharacterFunc( charId, uClass, name, skeletonType, m_onClientConnectUserData );
                  }
               }
            }
            else if (split[2] == "update")
            {
               // handle updates
               if (split.size() > 3)
               {
                  if (split[3] == "character")
                  {
                     if (split.size() > 4)
                     {
                        string character_string = split[3];
                        string name = split[4];
                     }

                    if (split[5] == "bones")
                       {
                          if (split.size() > 6)
                          {
                             int numBones = vhcl::ToInt(split[6]);
                          }

                          for (size_t i = 7; i < split.size(); i++)
                          {
                             string name = split[i];
                             string pos  = split[i + 1];
							 float posX = vhcl::ToFloat(split[i + 2]);
                             float posY = vhcl::ToFloat(split[i + 3]);
                             float posZ = vhcl::ToFloat(split[i + 4]);
                             string rot  = split[i + 5];
                             float rotX = vhcl::ToFloat(split[i + 6]);
                             float rotY = vhcl::ToFloat(split[i + 7]);
                             float rotZ = vhcl::ToFloat(split[i + 8]);
                             float rotW = vhcl::ToFloat(split[i + 9]);
                             i += 9;

							SmartBody::SBScene::getScene()->getDebuggerUtility()->updateCharacter(split[4], name, posX, posY, posZ, rotX, rotY, rotZ, rotW);
						  }
                       }
                     }
                  else if (split[3] == "pawn")
                  {
					// sbmdebugger <sbmid> update pawn <name> pos <x y z> rot <x y z w> geom <s> size <s> 

						float posX = vhcl::ToFloat(split[6]);
						float posY = vhcl::ToFloat(split[7]);
						float posZ = vhcl::ToFloat(split[8]);
						float rotX = vhcl::ToFloat(split[10]);
						float rotY = vhcl::ToFloat(split[11]);
						float rotZ = vhcl::ToFloat(split[12]);
						float rotW = vhcl::ToFloat(split[13]);
						SmartBody::SBScene::getScene()->getDebuggerUtility()->updatePawn(split[4], posX, posY, posZ, rotX, rotY, rotZ, rotW);                
                  }
                  else if (split[3] == "camera")
                  {
                     int i = 4;
                     string pos  = split[i];
                     float posX = vhcl::ToFloat(split[i + 1]);
                     float posY = vhcl::ToFloat(split[i + 2]);
                     float posZ = vhcl::ToFloat(split[i + 3]);
                     string rot  = split[i + 4];
                     float rotX = vhcl::ToFloat(split[i + 5]);
                     float rotY = vhcl::ToFloat(split[i + 6]);
                     float rotZ = vhcl::ToFloat(split[i + 7]);
                     float rotW = vhcl::ToFloat(split[i + 8]);
                     string persp  = split[i + 9];
                     float fovY   = vhcl::ToFloat(split[i + 10]);
                     float aspect = vhcl::ToFloat(split[i + 11]);
                     float zNear  = vhcl::ToFloat(split[i + 12]);
                     float zFar   = vhcl::ToFloat(split[i + 13]);
					 string center  = split[i + 14];
					 float centerX = vhcl::ToFloat(split[i + 15]);
					 float centerY = vhcl::ToFloat(split[i + 16]);
					 float centerZ = vhcl::ToFloat(split[i + 17]);
					 SmartBody::SBScene::getScene()->getDebuggerUtility()->updateCamera(posX, posY, posZ, centerX, centerY, centerZ, fovY, aspect, zNear, zFar);
                  }
               }


               //printf("Update Received\n");
               //printf(message.c_str());
               //printf("\n");
            }

         }

         marker = m_tcpData.find_first_of(';');
      }
	}
}

void SbmDebuggerClient::StartUpdates(double updateFrequencyS)
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s start_update %f", m_sbmId.c_str(), updateFrequencyS).c_str());
}

void SbmDebuggerClient::EndUpdates()
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s end_update", m_sbmId.c_str()).c_str());
}

void SbmDebuggerClient::SendSBMCommand(int requestId, const std::string & command)
{
   // send a void command, not expecting a return
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s %d request void \"ret = %s\"", m_sbmId.c_str(), 42, command.c_str()).c_str());

   m_netRequestManager.CreateNetRequest(requestId, NULL, NULL);
}

void SbmDebuggerClient::SendSBMCommand(int requestId, const std::string & returnValue, const std::string & functionNameandParams,
                                       NetRequest::RequestCallback cb,  void* callbackOwner)
{
   vhmsg::ttu_notify1(vhcl::Format("sbmdebugger %s %d request %s \"ret = %s\"", m_sbmId.c_str(),
      requestId, returnValue.c_str(), functionNameandParams.c_str()).c_str());

   m_netRequestManager.CreateNetRequest(requestId, cb, callbackOwner);
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
               if (split.size() >= 4 && split[3] == "response")
               {
                  NetRequest::RequestId rid = (NetRequest::RequestId)atoi(split[2].c_str());
                  split.erase(split.begin(), split.begin() + 4); // erase first 4 elements
                  m_netRequestManager.ProcessRequest(rid, split);
               }
               else if (split.size() >= 5 && split[3] == "response-fail")
               {
                  NetRequest::RequestId rid = (NetRequest::RequestId)atoi(split[2].c_str());
                  m_netRequestManager.RemoveRequest(rid);
                  printf("Error: response-fail on on net request %d. Reason: %s", (int)rid, split[4].c_str());
               }

               if (split.size() > 2)
               {
                  if (split[2] == "connect_success")
                  {
                     m_connectResult = true;					
					 SmartBody::SBScene::getScene()->getDebuggerUtility()->initScene();
					 SmartBody::SBScene::getScene()->getDebuggerUtility()->queryResources();
                  }
                  else if (split[2] == "init")
                  {
                     if (split.size() > 3)
                     {
                        if (split[3] == "character-skeleton")
						{
							string name = split[4];
							string skeletonName = split[5];
							SmartBody::SBScene::getScene()->getDebuggerUtility()->initCharacter(name, skeletonName);
						}
						else if (split[3] == "character-face_definition")
						{
							string name = split[4];
							string faceDefName = split[5];
							SmartBody::SBScene::getScene()->getDebuggerUtility()->initCharacterFaceDefinition(name, faceDefName, message);

						}
						else if (split[3] == "skeleton")
						{
							std::string skFileName = split[4];
							std::string command = "# " + message;
							SmartBody::SBScene::getScene()->getDebuggerUtility()->initSkeleton(skFileName, command);
						}
                        else if (split[3] == "pawn")
                        {
							SmartBody::SBScene::getScene()->getDebuggerUtility()->initPawn(split[4]);
                        }
						else if (split[3] == "face_definition")
						{
							std::string command = "# " + message;
							SmartBody::SBScene::getScene()->getDebuggerUtility()->runPythonCommand(command);						
						}
						else if (split[3] == "blend")
						{
							std::string command = "# " + message;
							SmartBody::SBScene::getScene()->getDebuggerUtility()->runPythonCommand(command);						
						}
						else if (split[3] == "transition")
						{
							std::string command = "# " + message;
							SmartBody::SBScene::getScene()->getDebuggerUtility()->runPythonCommand(command);						
						}
                     }


                     //printf("Init Received\n");
                     //printf(message.c_str());
                     //printf("\n");
                  }
               }
            }
         }
      }
      else if (split[0] == "vrAllCall")
      {
         vhmsg::ttu_notify1("vrComponent sbmdebugger");
      }
      else if (split[0] == "vrKillComponent")
      {
         if (split.size() > 1 && split[1] == "sbmdebugger")
         {
            
         }
      }
      else if (split[0] == "vrKillAll")
      {
         
      }
   }
}
