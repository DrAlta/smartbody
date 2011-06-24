/*
 *  ParserFBX.cpp - part of Motion Engine and SmartBody-lib
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
 *      Adam Reilly, USC
 */

#include "vhcl.h"

#include "ParserFBX.h"

#if ENABLE_FBX_PARSER   // defined in the project settings

#include <iostream>

#include "boost/filesystem/operations.hpp"
// starts up the fbx sdk
#include "boost/filesystem/convenience.hpp"

#include "sr/sr_euler.h"


bool ParserFBX::Init(KFbxSdkManager** pSdkManager, KFbxImporter** pImporter, KFbxScene** pScene, const std::string& fileName)
{
   // Initialize the sdk manager. This object handles all our memory management.
   *pSdkManager = KFbxSdkManager::Create();

   /*
   To import the contents of an FBX file, a KFbxIOSettings object and a KFbxImporter object must be created.
   A KFbxImporter object is initialized by providing the filename of the file we want to import along with a
   KFbxIOSettings object which has been appropriately configured to suit our importing needs (see I/O Settings).
   */

   // Create the io settings object.
   KFbxIOSettings *ios = KFbxIOSettings::Create(*pSdkManager, IOSROOT);

   (*pSdkManager)->SetIOSettings(ios);

   // Create an importer using our sdk manager.
   *pImporter = KFbxImporter::Create(*pSdkManager,"");
    
   // Use the first argument as the filename for the importer.
   if(!(*pImporter)->Initialize(fileName.c_str(), -1, (*pSdkManager)->GetIOSettings())) 
   {
       LOG("Call to KFbxImporter::Initialize() failed. Error returned: %s\n\n", (*pImporter)->GetLastErrorString());
       return false;
   }

   /*
      The KFbxImporter object populates a provided KFbxScene object with the elements contained in the FBX file.
      Observe that the KFbxScene::Create() function is passed an empty string as its second parameter. Objects
      created in the FBX SDK can be given arbitrary, non-unique names, which allow the user or other programs to
      identify the object after it has been exported. After the KFbxScene is populated, the KFbxImporter can be
      safely destroyed.
   */
   // Create a new scene so it can be populated by the imported file.
   *pScene = KFbxScene::Create(*pSdkManager, "");

   // Import the contents of the file into the scene.
   (*pImporter)->Import(*pScene);

   return true;
}

// shuts down the fbx sdk
void ParserFBX::Shutdown(KFbxSdkManager* pSdkManager, KFbxImporter* pImporter)
{
   // The file has been imported; we can get rid of the importer.
   pImporter->Destroy();

   // Shutdown sdk manager
   pSdkManager->Destroy();
}

bool ParserFBX::parse(SkSkeleton& skeleton, SkMotion& motion, const std::string& fileName, float scale)
{
   KFbxSdkManager* pSdkManager;
   KFbxImporter* pImporter;
   KFbxScene* pScene;
   if (!Init(&pSdkManager, &pImporter, &pScene, fileName))
   {
      return false;
   }

   // save the name of the skeleton/anim
   std::string filebasename = boost::filesystem::basename(fileName);
   motion.name(filebasename.c_str());
   skeleton.name(filebasename.c_str());
   int order = -1;

   KFbxNode* pRootNode = pScene->GetRootNode();

   if(pRootNode) 
   {
       FBXMetaData metaData;
       for (int i = 0; i < pRootNode->GetChildCount(); i++)
       {
          parseJoints(pRootNode->GetChild(i), skeleton, motion, scale, order, metaData, NULL);
          //parseSkinRecursive(pRootNode->GetChild(i), "", 1.0f, std::string(""), NULL, NULL);
       }

       // go through all the animation data and add it
       parseAnimation(pScene, skeleton, motion, scale, order, metaData);
   }
   else
   {
      LOG("Failed to load fbx: %s\n", filebasename.c_str());
      return false;
   }
          
   Shutdown(pSdkManager, pImporter);
   return true;
}

bool ParserFBX::parseSkin(const std::string& fileName, const char* char_name, float scaleFactor, std::string& jointPrefix, mcuCBHandle* mcu_p)
{
   KFbxSdkManager* pSdkManager;
   KFbxImporter* pImporter;
   KFbxScene* pScene;
   if (!Init(&pSdkManager, &pImporter, &pScene, fileName))
   {
      return false;
   }

   KFbxNode* pRootNode = pScene->GetRootNode();
   if(pRootNode) 
   {
       SbmCharacter* char_p = mcu_p->character_map.lookup( char_name );
       std::vector<SrModel*> meshModelVec;
       //FBXSkinData fbxSkinData;
       for (int i = 0; i < pRootNode->GetChildCount(); i++)
       {
          parseSkinRecursive(pRootNode->GetChild(i), char_name, scaleFactor, jointPrefix, mcu_p, char_p, meshModelVec);
       }
         
      // cache the joint names for each skin weight
      for (size_t x = 0; x < char_p->dMesh_p->skinWeights.size(); x++)
      {
         SkinWeight* skinWeight = char_p->dMesh_p->skinWeights[x];
         for (size_t j = 0; j < skinWeight->infJointName.size(); j++)
         {
            std::string& jointName = skinWeight->infJointName[j];
            SkJoint* curJoint = char_p->skeleton_p->search_joint(jointName.c_str());
            skinWeight->infJoint.push_back(curJoint); // NOTE: If joints are added/removed during runtime, this list will contain stale data
         }
      }

      for (unsigned int i = 0; i < meshModelVec.size(); i++)
      {
         for (int j = 0; j < meshModelVec[i]->V.size(); j++)
         {
            meshModelVec[i]->V[j] *= scaleFactor;
         }
         SrSnModel* srSnModelDynamic = new SrSnModel();
         SrSnModel* srSnModelStatic = new SrSnModel();
         srSnModelDynamic->shape(*meshModelVec[i]);
         srSnModelStatic->shape(*meshModelVec[i]);
         srSnModelDynamic->changed(true);
         srSnModelDynamic->visible(false);
         srSnModelStatic->shape().name = meshModelVec[i]->name;
         srSnModelDynamic->shape().name = meshModelVec[i]->name;
         char_p->dMesh_p->dMeshDynamic_p.push_back(srSnModelDynamic);
         char_p->dMesh_p->dMeshStatic_p.push_back(srSnModelStatic);
         mcu_p->root_group_p->add(srSnModelDynamic);	
      }
   }
   else
   {
      LOG("No root node found in %s", fileName.c_str());
   }

   Shutdown(pSdkManager, pImporter);
   return true;
}

void ParserFBX::parseSkinRecursive(KFbxNode* pNode, const char* char_name, float scaleFactor,
                                   std::string& jointPrefix, mcuCBHandle* mcu_p, SbmCharacter* char_p,
                                   std::vector<SrModel*>& meshModelVec)
{
   if (pNode->GetNodeAttribute()->GetAttributeType() == KFbxNodeAttribute::eMESH)
   {
      KFbxMesh* pMesh = (KFbxMesh*)pNode->GetNodeAttribute();
      KFbxSkin* pSkin = (KFbxSkin*)pMesh->GetDeformer(0, KFbxDeformer::eSKIN);

      const char* pMeshName = pMesh->GetName();
      KFbxXMatrix transformMatrix;
      SrPnt pnt;   
      int accumulatedAffectedVertices = 0;

      SkinWeight* skinWeight = new SkinWeight();
      skinWeight->sourceMesh = pMeshName;

      // map key = vertex index
      // map value = vector of pairs
      //    -pair first = index into the jointNameIndex array in SkinWeight
      //    -pair second = index into the weightIndex array in SkinWeight
      std::map<int, std::vector<std::pair<int, int>>> vertexToIndexMap;

      int weightIndexer = 0;
      int ClusterCount = pSkin->GetClusterCount();
      for (int j = 0; j < ClusterCount; ++j)
      {
         KFbxCluster* pCluster = pSkin->GetCluster(j);
         KFbxNode* pBone = pCluster->GetLink();

         // get the transform
         //pCluster->GetTransformMatrix(transformMatrix);
         pCluster->GetTransformLinkMatrix(transformMatrix);

         // convert to smartbody format
         double* matBuffer = new double[16];
         for (int i = 0; i < 4; ++i)
         {
            KFbxVector4 row = transformMatrix.GetRow(i);
            matBuffer[i * 4] = row.GetAt(0);
            matBuffer[i * 4 + 1] = row.GetAt(1);
            matBuffer[i * 4 + 2] = row.GetAt(2);
            matBuffer[i * 4 + 3] = row.GetAt(3);
         }

         matBuffer[12] *= -1;
         matBuffer[13] *= -1;
         matBuffer[14] *= -1;

         skinWeight->bindShapeMat.set(matBuffer);
         SrMat sbmMatrix(matBuffer);
         skinWeight->bindPoseMat.push_back(sbmMatrix);

         const char* pBoneName = pBone->GetName();
         skinWeight->infJointName.push_back(pBoneName);

         int AffectedVertexCount = pCluster->GetControlPointIndicesCount(); 
         for (int k = 0; k < AffectedVertexCount; ++k, ++weightIndexer) 
         {         
            int AffectedVertexIndex = pCluster->GetControlPointIndices()[k];

            // map the vertex index to the weight and the joint that influences it
            vertexToIndexMap[AffectedVertexIndex].push_back(std::pair<int, int>(j, accumulatedAffectedVertices + k));

            float Weight = (float)pCluster->GetControlPointWeights()[k]; 
            skinWeight->bindWeight.push_back(Weight);
         }

         // update the offset 
         accumulatedAffectedVertices += AffectedVertexCount;
      } 

      // convert all the stored index data from vertexToIndexMap to the SkinWeight class
      SrModel* objModel = new SrModel();
      std::map<int, std::vector<std::pair<int, int>>>::iterator it;
      for (it = vertexToIndexMap.begin(); it != vertexToIndexMap.end(); it++)
      {
         // use the map key (index into the vertex buffer) to find the vertex position
         KFbxVector4 vertex = pMesh->GetControlPointAt(it->first);
         ConvertKFbxVector4ToSrPnt(vertex, pnt);
         objModel->V.push(pnt);
         
         skinWeight->numInfJoints.push_back(it->second.size());
         for (unsigned int k = 0; k < it->second.size(); k++)
         {
            skinWeight->jointNameIndex.push_back(it->second[k].first);
            skinWeight->weightIndex.push_back(it->second[k].second);
         }
      }

      // build the face list
      parseGeometry(pMesh, objModel, meshModelVec);

      if (char_p)
      {
         char_p->dMesh_p->skinWeights.push_back(skinWeight);
      }
   }

   for (int i = 0; i < pNode->GetChildCount(); i++)
   {
        // iterate on the children of this node
        parseSkinRecursive(pNode->GetChild(i), char_name, scaleFactor, jointPrefix, mcu_p, char_p, meshModelVec);
   }
}

void ParserFBX::parseGeometry(KFbxMesh* pMesh, SrModel* objModel, std::vector<SrModel*>& meshModelVec)
{
   // set up the drawing data that will be used in the viewer
   //SrModel* objModel = new SrModel();
   objModel->name = pMesh->GetName();
   SrModel::Face face;
   SrPnt pnt; SrPnt2 pnt2;
   KFbxVector4 normal;
   KFbxVector2 uv;

   // for each polygon
   int nPolygonCount = pMesh->GetPolygonCount();
   for (int i = 0; i < nPolygonCount; ++i)
   {
      int nPolygonSize = pMesh->GetPolygonSize(i);
      std::vector<int> savedVertexIndices;

      KStringList uvSetNames;
      pMesh->GetUVSetNames(uvSetNames);

      for (int j = 0; j < nPolygonSize; j++)
      {
         // get the vertex index
         int nControlPointIndex = pMesh->GetPolygonVertex(i, j);

         // get the normal of this vertex
         pMesh->GetPolygonVertexNormal(i, nControlPointIndex, normal);
         ConvertKFbxVector4ToSrPnt(normal, pnt);
         objModel->N.push(pnt);

         // get the uv of this vertex
         //int nUVIndex = pMesh->GetTextureUVIndex(i, j);
         //pMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv);
         //ConvertKFbxVector2ToSrPnt2(uv, pnt2);
         //objModel->T.push(pnt2);

         savedVertexIndices.push_back(nControlPointIndex);   
      }

      // create the faces based off the vertices
      if (savedVertexIndices.size() == 3 || savedVertexIndices.size() > 4)
      {
         face.set(savedVertexIndices[0], savedVertexIndices[1], savedVertexIndices[2]);
         objModel->F.push(face);
      }
      else if (savedVertexIndices.size() == 4)
      {
         // make 2 triangles from the 4 verts
         face.set(savedVertexIndices[0], savedVertexIndices[1], savedVertexIndices[2]);
         objModel->F.push(face);
         face.set(savedVertexIndices[0], savedVertexIndices[2], savedVertexIndices[3]);
         objModel->F.push(face);
      }  
   }

   objModel->smooth();
   meshModelVec.push_back(objModel);
}

void ParserFBX::parseJoints(KFbxNode* pNode, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, FBXMetaData& metaData, SkJoint* parent)
{
   SkJoint* joint = parent;
   if (pNode->GetNodeAttribute()->GetAttributeType() == KFbxNodeAttribute::eSKELETON)
   {
      // we only create joints for nodes flagged as having the eSKELETON attribute
      joint = createJoint(pNode, skeleton, motion, scale, order, parent);
   }

   // if it has this meta data property, then it has all the others,
   // so parse them out save them
   KFbxProperty prop = pNode->FindProperty("rdyTime");
   if (prop.IsValid())
   {
      parseMetaData(pNode, metaData);
   }

   for (int i = 0; i < pNode->GetChildCount(); i++)
   {
        // iterate on the children of this node
        parseJoints(pNode->GetChild(i), skeleton, motion, scale, order, metaData, joint);
   }
}

SkJoint* ParserFBX::createJoint(KFbxNode* pNode, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, SkJoint* parent)
{
   int index = -1;
   if (parent != NULL)	
   {
      index = parent->index();
   }

   // get joint name
   std::string nameAttr = pNode->GetName();
   SkJoint* joint = skeleton.add_joint(SkJoint::TypeQuat, index);
   joint->quat()->activate();
   joint->name(SkJointName(nameAttr.c_str()));

   // setup skeleton channels	
   bool bHasChannelProperty = false;
   if (HasSmartbodyChannel(pNode, "SbodyPosX", bHasChannelProperty) && bHasChannelProperty)
   {
      skeleton.channels().add(joint->name(), SkChannel::XPos);
      joint->pos()->limits(SkVecLimits::X, false);
   }

   if (HasSmartbodyChannel(pNode, "SbodyPosY", bHasChannelProperty) && bHasChannelProperty)
   {
      skeleton.channels().add(joint->name(), SkChannel::YPos);
      joint->pos()->limits(SkVecLimits::Y, false);
   }

   if (HasSmartbodyChannel(pNode, "SbodyPosZ", bHasChannelProperty) && bHasChannelProperty)
   {
      skeleton.channels().add(joint->name(), SkChannel::ZPos);
      joint->pos()->limits(SkVecLimits::Z, false);
   }

   if (HasSmartbodyChannel(pNode, "SbodyQuat", bHasChannelProperty) && bHasChannelProperty)
   {
      skeleton.channels().add(joint->name(), SkChannel::Quat);
      joint->quat()->activate();
   }

   SrVec offset, rot, jointRot;

   if (parent == NULL)
      skeleton.root(joint);

   order = 123;//getRotationOrder(orderVec);
   if (order == -1)
      LOG("ParserFBX::parseJoints ERR: rotation info not correct in the file");

   // get local position
   fbxDouble3 translation = pNode->LclTranslation.Get();
   ConvertfbxDouble3ToSrVec(translation, offset);
   offset *= scale;
   joint->offset(offset);

   // get local rotation
   fbxDouble3 rotation = pNode->LclRotation.Get();
   ConvertfbxDouble3ToSrVec(rotation, rot);
   rot *= float(M_PI) / 180.0f;

   // get prerotation
   fbxDouble3 jointRotation = pNode->PreRotation.Get();
   ConvertfbxDouble3ToSrVec(jointRotation, jointRot);
   jointRot *= float(M_PI) / 180.0f;

   // convert from euler angles to mat
   SrMat rotMat, jorientMat;
   sr_euler_mat(order, rotMat, rot.x, rot.y, rot.z);
   sr_euler_mat(order, jorientMat, jointRot.x, jointRot.y, jointRot.z);
   //SrMat finalRotMat = rotMat;

   // convert from mat to quat
   SrQuat quat = SrQuat(rotMat);
   SrQuat jorientQ = SrQuat(jorientMat);
   SkJointQuat* jointQuat = joint->quat();
   jointQuat->prerot(quat);
   jointQuat->orientation(jorientQ);

   return joint;
}

void ParserFBX::parseMetaData(KFbxNode* pNode, FBXMetaData& out_metaData)
{
   //KFbxNode* smartbodyNode = pRootNode->FindChild("AnimDef_ChrBillFord_IdleSeated01");
   KFbxProperty prop;

   prop = pNode->FindProperty("rdyTime");
   if (prop.IsValid())
   {
      prop.Get(&out_metaData.readyTime, eDOUBLE1);
   }

   prop = pNode->FindProperty("sStart");
   if (prop.IsValid())
   {
      prop.Get(&out_metaData.strokeStart, eDOUBLE1);
   }

   prop = pNode->FindProperty("seTime");
   if (prop.IsValid())
   {
      prop.Get(&out_metaData.emphasisTime, eDOUBLE1);
   }

   prop = pNode->FindProperty("sEnd");
   if (prop.IsValid())
   {
      prop.Get(&out_metaData.strokeTime, eDOUBLE1);
   }

   prop = pNode->FindProperty("rlxTime");
   if (prop.IsValid())
   {
      prop.Get(&out_metaData.relaxTime, eDOUBLE1);
   }

   const double oneOverThirty = 1.0 / 30.0;
   out_metaData -= 1.0f; // subtract 1 frame
   out_metaData *= oneOverThirty;
}

void ParserFBX::parseAnimation(KFbxScene* pScene, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, const FBXMetaData& metaData)
{
   KFbxNode* rootNode = pScene->GetRootNode();

   // need this to store the anim data then give it to the motion struct
   std::vector<FBXAnimData*> fbxAnimData;
   for (int i = 0; i < pScene->GetSrcObjectCount(FBX_TYPE(KFbxAnimStack)); i++)
   {
      // stacks hold animation layers, so go through the stacks first
      KFbxAnimStack* pAnimStack = KFbxCast<KFbxAnimStack>(pScene->GetSrcObject(FBX_TYPE(KFbxAnimStack), i));
      pScene->GetEvaluator()->SetContext(pAnimStack);

      KTimeSpan timeSpan = pAnimStack->GetLocalTimeSpan();

      parseAnimationRecursive(pAnimStack, rootNode, skeleton, motion, scale, order, fbxAnimData);
      ConvertfbxAnimToSBM(fbxAnimData, skeleton, motion, scale, order, metaData);

      // start fresh for new anim
      for (unsigned int i = 0; i < fbxAnimData.size(); i++)
      {
         delete fbxAnimData[i];
      }
      fbxAnimData.clear();
   }
} 

void ParserFBX::parseAnimationRecursive(KFbxAnimStack* pAnimStack, KFbxNode* pNode, SkSkeleton& skeleton, SkMotion& motion,
                                      float scale, int& order, std::vector<FBXAnimData*>& fbxAnimData)
{
   int nbAnimLayers = pAnimStack->GetMemberCount(FBX_TYPE(KFbxAnimLayer));

   std::string takeName = pAnimStack->GetName();  
   for (int layerNo = 0; layerNo < nbAnimLayers; layerNo++)
   {
      // layers hold animcurves so go through each layers set of curves
      KFbxAnimLayer* pAnimLayer = pAnimStack->GetMember(FBX_TYPE(KFbxAnimLayer), layerNo);

      parseAnimationRecursive(pAnimLayer, pNode, takeName, skeleton, motion, scale, order, fbxAnimData);
   }
}

void ParserFBX::parseKeyData(const KFbxAnimCurve* pCurve, const SkChannel::Type type, const std::string& jointName,
                             SkMotion& motion, std::vector<FBXAnimData*>& fbxAnimData)
{
   if (!pCurve)
   {
      return;
   }

   // create a new FBXAnimData and store it
   FBXAnimData* pNewAnimData = new FBXAnimData();
   pNewAnimData->channelName = jointName;
   pNewAnimData->channelType = type;
  
   int numKeys = pCurve->KeyGetCount();
   for (int i = 0; i < numKeys; i++)
   {
      KFbxAnimCurveKey key = pCurve->KeyGet(i);
      
      // the time at which the data takes places
      KTime time = key.GetTime();
      const kLongLong t = time.Get();

      // the key channel transformation data
      float value = key.GetValue();
      pNewAnimData->keyFrameData[t] = value;
      pNewAnimData->keyFrameDataFrame[i] = value;
   }

   fbxAnimData.push_back(pNewAnimData);
}

void ParserFBX::parseAnimationRecursive(KFbxAnimLayer* pAnimLayer, KFbxNode* pNode, std::string &takeName, SkSkeleton& skeleton,
                                      SkMotion& motion, float scale, int& order, std::vector<FBXAnimData*>& fbxAnimData)
{
   // get the joint name
   std::string jointName = pNode->LclTranslation.GetParent().GetName();
   KFbxAnimCurve *pCurve = NULL;
   
   bool bHasChannelProperty = false;

   if (HasSmartbodyChannel(pNode, "SbodyPosX", bHasChannelProperty) && bHasChannelProperty)
   {
      // x pos data
      pCurve = pNode->LclTranslation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_T_X);
      parseKeyData(pCurve, SkChannel::XPos, jointName, motion, fbxAnimData);
   }

   if (HasSmartbodyChannel(pNode, "SbodyPosY", bHasChannelProperty) && bHasChannelProperty)
   {
       // y pos data
      pCurve = pNode->LclTranslation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_T_Y);
      parseKeyData(pCurve, SkChannel::YPos, jointName, motion, fbxAnimData);
   }

   if (HasSmartbodyChannel(pNode, "SbodyPosZ", bHasChannelProperty) && bHasChannelProperty)
   {
       // z pos data
      pCurve = pNode->LclTranslation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_T_Z);
      parseKeyData(pCurve, SkChannel::ZPos, jointName, motion, fbxAnimData);
   }

   if (HasSmartbodyChannel(pNode, "SbodyQuat", bHasChannelProperty) && bHasChannelProperty)
   {
        // x rot data
      pCurve = pNode->LclRotation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_R_X);
      parseKeyData(pCurve, SkChannel::XRot, jointName, motion, fbxAnimData);

      // y rot data
      pCurve = pNode->LclRotation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_R_Y);
      parseKeyData(pCurve, SkChannel::YRot, jointName, motion, fbxAnimData);

      // z rot data
      pCurve = pNode->LclRotation.GetCurve<KFbxAnimCurve>(pAnimLayer, KFCURVENODE_R_Z);
      parseKeyData(pCurve, SkChannel::ZRot, jointName, motion, fbxAnimData);
   }
    
   int childCount = pNode->GetChildCount();
   for (int i = 0; i < childCount; i++)
   {
      parseAnimationRecursive(pAnimLayer, pNode->GetChild(i), takeName, skeleton, motion, scale, order, fbxAnimData);
   }
}

void ParserFBX::ConvertfbxDouble3ToSrVec(const fbxDouble3& fbx, SrVec& sbm)
{
   sbm.x = (float)fbx.mData[0];
   sbm.y = (float)fbx.mData[1];
   sbm.z = (float)fbx.mData[2];
}

void ParserFBX::ConvertKFbxVector4ToSrPnt(const KFbxVector4& fbx, SrPnt& sbm)
{
   sbm.x = (float)fbx.GetAt(0);
   sbm.y = (float)fbx.GetAt(1);
   sbm.z = (float)fbx.GetAt(2);
}

void ParserFBX::ConvertKFbxVector2ToSrPnt2(const KFbxVector2& fbx, SrPnt2& sbm)
{
   sbm.x = (float)fbx.GetAt(0);
   sbm.y = (float)fbx.GetAt(1);
}

void ParserFBX::ConvertfbxAnimToSBM(const std::vector<FBXAnimData*>& fbxAnimData, SkSkeleton& skeleton,
                                    SkMotion& motion, float scale, int& order, const FBXMetaData& metaData)
{
   if (fbxAnimData.empty())
   {
      return;
   }

   const int stride = 6;
   const float DEG_TO_RAD = float(M_PI) / 180.0f;
   motion.init(skeleton.channels());

   // create the frames and the times assoicated with them
   int numKeys = fbxAnimData[0]->keyFrameData.size();
   std::map<kLongLong, float>::iterator it;
   int f = 0;
   for (it = fbxAnimData[0]->keyFrameData.begin(); it != fbxAnimData[0]->keyFrameData.end(); f++, it++)
   {
      // convert the way fbx stores the time into the way sbm does (in seconds)
      motion.insert_frame(f, (float)(it->first) / 46186158000.0f);

      for (int j = 0; j < motion.posture_size(); j++)
      {
         motion.posture(f)[j] = 0.0f;
      }
   }

   SrMat mat; SrQuat quat;
   /*for (int i = 0; i < numKeys; i++)
   {
      for (unsigned int j = 0; j < fbxAnimData.size(); j+= stride)
      {
         std::string jointName = fbxAnimData[j]->channelName;

         // find the skeleton's joint index based off the name of the joint and its channel type
         int jointIndex = motion.channels().search(SkJointName(fbxAnimData[j]->channelName.c_str()), SkChannel::XPos);
         int floatIndex = -1;
         if (jointIndex >= 0)
         {
            floatIndex = motion.channels().float_position(jointIndex);
         }
            
         if (floatIndex < 0)
         {
            // either the joint doesn't exist or the channel's buffer doesn't exist
            continue;
         }

         // i need the joint so that I can subtract the starting offset from the anim data
         SkJoint* joint = skeleton.search_joint(fbxAnimData[j]->channelName.c_str());

         // set x,y,z pos channels
         motion.posture(i)[floatIndex] = (fbxAnimData[j]->keyFrameDataFrame[i] - joint->offset().x) / scale; //x pos
         motion.posture(i)[floatIndex+1] = (fbxAnimData[j+1]->keyFrameDataFrame[i] - joint->offset().y) / scale; //y pos
         motion.posture(i)[floatIndex+2] = (fbxAnimData[j+2]->keyFrameDataFrame[i] - joint->offset().z) / scale; //z pos

         // convert euler angles to mat
         sr_euler_mat(order, mat, fbxAnimData[j+3]->keyFrameDataFrame[i] * DEG_TO_RAD,
            fbxAnimData[j+4]->keyFrameDataFrame[i] * DEG_TO_RAD, fbxAnimData[j+5]->keyFrameDataFrame[i] * DEG_TO_RAD);
         
         // convert the mat to a quat
         quat = SrQuat(mat);

         // set the rotation
         motion.posture(i)[floatIndex+3] = quat.w;
         motion.posture(i)[floatIndex+4] = quat.x;
         motion.posture(i)[floatIndex+5] = quat.y;
         motion.posture(i)[floatIndex+6] = quat.z;
      }
   }*/

   for (int i = 0; i < numKeys; i++)
   {
      int offset = 1;
      for (unsigned int j = 0; j < fbxAnimData.size(); j+= offset)
      {
         std::string jointName = fbxAnimData[j]->channelName;

         // find the skeleton's joint index based off the name of the joint and its channel type
         int jointIndex = motion.channels().search(SkJointName(fbxAnimData[j]->channelName.c_str()),
            fbxAnimData[j]->channelType == SkChannel::XRot ? SkChannel::Quat : fbxAnimData[j]->channelType);
         int floatIndex = -1;
         if (jointIndex >= 0)
         {
            floatIndex = motion.channels().float_position(jointIndex);
         }
            
         if (floatIndex < 0)
         {
            // either the joint doesn't exist or the channel's buffer doesn't exist
            offset = 1;
            continue;
         }

         // i need the joint so that I can subtract the starting offset from the anim data
         SkJoint* joint = skeleton.search_joint(fbxAnimData[j]->channelName.c_str());

         if (fbxAnimData[j]->channelType == SkChannel::XRot)
         {
            // rotational data 
            // convert euler angles to mat
            sr_euler_mat(order, mat, fbxAnimData[j]->keyFrameDataFrame[i] * DEG_TO_RAD,
               fbxAnimData[j+1]->keyFrameDataFrame[i] * DEG_TO_RAD, fbxAnimData[j+2]->keyFrameDataFrame[i] * DEG_TO_RAD);
            
            // convert the mat to a quat
            quat = SrQuat(mat);

            // set the rotation
            motion.posture(i)[floatIndex] = quat.w;
            motion.posture(i)[floatIndex+1] = quat.x;
            motion.posture(i)[floatIndex+2] = quat.y;
            motion.posture(i)[floatIndex+3] = quat.z;
         }
         else
         {
            // translational data, x, y, or z
            motion.posture(i)[floatIndex] = (fbxAnimData[j]->keyFrameDataFrame[i] - joint->offset()[SkChannel::ZPos - fbxAnimData[j]->channelType]) / scale;
         }

         // rots are stored as 3 seperate entries in fbxAnimData, x, y, z euler angles.  You need all 3 of them to 
         // make the sbm quat, so if this was rotational data, move to the next joint, otherwise, for translational data,
         // we move by 1 channel.
         offset = fbxAnimData[j]->channelType == SkChannel::XRot ? 3 : 1; 
      }
   }

   // set the meta data
   motion.synch_points.set_time(metaData.readyTime, metaData.strokeStart,
      metaData.emphasisTime, metaData.strokeTime, metaData.relaxTime);
   motion.compress();
}

bool ParserFBX::HasSmartbodyChannel(KFbxNode* pNode, const char* pChannelName, bool& out_ChannelValue)
{
   KFbxProperty prop = pNode->FindProperty(pChannelName);
   if (prop.IsValid())
   {
      prop.Get(&out_ChannelValue, eBOOL1);
      return true;
   }

   return false;
}

#else

int g_parser_fbx_cpp____unused = 0;  // to prevent warning

#endif  // ENABLE_FBX_PARSER
