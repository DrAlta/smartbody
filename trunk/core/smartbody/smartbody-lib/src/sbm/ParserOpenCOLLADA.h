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
#include "sk/sk_skeleton.h"
#include "sk/sk_motion.h"
#include "mcontrol_util.h"
#include "gwiz_math.h"

class ParserOpenCOLLADA
{
	public:
		static bool parse(SkSkeleton& skeleton, SkMotion& motion, std::string fileName, float scale);
		static xercesc_3_0::DOMNode* getNode(std::string nodeName, xercesc_3_0::DOMNode* node);
		static void parseLibraryVisualScenes(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order);
		static void parseJoints(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order, SkJoint* parent = NULL);
		static void parseLibraryAnimations(xercesc_3_0::DOMNode* node, SkSkeleton& skeleton, SkMotion& motion, float scale, int& order);
		static void animationPostProcess(SkSkeleton& skeleton, SkMotion& motion);

	private:
		static int getMotionChannelId(SkChannelArray& channels, std::string sourceName);
		static std::string getString(const XMLCh* s);
		static std::string tokenize(std::string& str,const std::string& delimiters = " ", int mode = 1);
		static int getRotationOrder(std::vector<std::string> orderVec);
};

#endif