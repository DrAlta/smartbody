#ifndef _BONEMAP_H_
#define _BONEMAP_H_

#include <map>
#include "sk/sk_motion.h"
#include "sk/sk_skeleton.h"

class BoneMap
{
	public:
		BoneMap();
		~BoneMap();

		void apply(SkMotion* motion);
		void apply(SkSkeleton* skeleton);

		std::map<std::string, std::string> map;
};

#endif