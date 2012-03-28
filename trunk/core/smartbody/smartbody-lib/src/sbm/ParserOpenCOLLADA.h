/*
 *  ParserOpenCOLLADA.h - part of Motion Engine and SmartBody-lib
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

#ifndef _PARSER_OPENCOLLADA_H_
#define _PARSER_OPENCOLLADA_H_

#include <fstream>
#include "xercesc_utils.hpp"
#include "sk/sk_skeleton.h"
#include "sk/sk_motion.h"
#include "mcontrol_util.h"
#include "gwiz_math.h"

typedef std::map<std::string, std::vector<SrVec>> VecListMap;

class ParserOpenCOLLADA
{
	public:
		static DOMNode* getNode(std::string nodeName, DOMNode* node);
		static DOMNode* getNode(std::string nodeName, std::string fileName);
		static bool parse(SkSkeleton& skeleton, SkMotion& motion, std::string fileName, float scale);
		static void parseLibraryVisualScenes(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, std::map<std::string, std::string>& materialId2Name);
		static void parseJoints(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, std::map<std::string, std::string>& materialId2Name, SkJoint* parent = NULL);
		static void parseLibraryAnimations(DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order);
		static void animationPostProcess(SkSkeleton& skeleton, SkMotion& motion);
		static void animationPostProcessByChannels(SkSkeleton& skeleton, SkMotion& motion, SkChannelArray& channels);
		static void parseLibraryGeometries(DOMNode* node, const char* file, SrArray<SrMaterial>& M, SrStringArray& mnames, std::map<std::string,std::string>& mtlTexMap, std::map<std::string,std::string>& mtlTexBumpMap, std::vector<SrModel*>& meshModelVec, float scale);
		static void load_texture(int type, const char* file, const SrStringArray& paths);
		static void parseLibraryMaterials(DOMNode* node, std::map<std::string, std::string>& effectId2MaterialId);
		static void parseLibraryImages(DOMNode* node, std::map<std::string, std::string>& pictureId2File);
		static void parseLibraryEffects(DOMNode* node, std::map<std::string, std::string>&effectId2MaterialId, std::map<std::string, std::string>& materialId2Name, std::map<std::string, std::string>& pictureId2File, SrArray<SrMaterial>& M, SrStringArray& mnames, std::map<std::string,std::string>& mtlTexMap, std::map<std::string,std::string>& mtlTexBumpMap);

	private:
		static int getMotionChannelId(SkChannelArray& channels, std::string sourceName);
		static std::string tokenize(std::string& str,const std::string& delimiters = " ", int mode = 1);
		static int getRotationOrder(std::vector<std::string> orderVec);
		static std::string getGeometryType(std::string s);
		static void setModelVertexSource(std::string& sourceName, std::string& semanticName, SrModel* model, VecListMap& vecMap);
};

#endif
