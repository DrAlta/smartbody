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

#ifndef _PARSER_OGRRESKELETON_H_
#define _PARSER_OGRRESKELETON_H_

#include <fstream>
#include "sbm/xercesc_utils.hpp"
#include "sk/sk_skeleton.h"
#include "sk/sk_motion.h"
#include "sbm/mcontrol_util.h"
#include "sbm/gwiz_math.h"

class ParserOgre
{
	public:
		static bool parse(SkSkeleton& skeleton, std::vector<SkMotion*>& motions, std::string fileName, float scale, bool doParseSkeleton, bool doParseMotion);
		static DOMNode* getNode(const std::string& nodeName, DOMNode* node);

		static bool parseSkeleton(DOMNode* skeletonNode, SkSkeleton& skeleton, std::string fileName, float scale);
		static bool parseMotion(DOMNode* motionNode, std::vector<SkMotion*>& motions, SkMotion* motion,std::string fileName, float scale);

};

#endif
