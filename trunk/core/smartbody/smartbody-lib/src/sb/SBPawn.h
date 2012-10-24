#ifndef _SBPAWN_H_
#define _SBPAWN_H_

#include <sbm/sbm_pawn.hpp>

namespace SmartBody {

class SBSkeleton;

class SBPawn : public SbmPawn
{
	public:
		SBPawn();
		SBPawn( const char* name );
		~SBPawn();

		virtual void addMesh(std::string mesh);
		
		virtual SBSkeleton* getSkeleton();
		virtual void setSkeleton(SBSkeleton* skel);

		SrVec getPosition();
		SrQuat getOrientation();
		void setPosition(SrVec pos);
		void setPositionSmooth(SrVec pos, float smoothTime);
		void setOrientation(SrQuat quat);
		void setHPR(SrVec hpr);
		void setHPRSmooth(SrVec hpr, float smoothTime);
		SrVec getHPR();

		virtual void afterUpdate(double time);
		virtual void notify(SBSubject* subject);

	private:
		SmartBody::DoubleAttribute* _posX;
		SmartBody::DoubleAttribute* _posY;
		SmartBody::DoubleAttribute* _posZ;
		SmartBody::DoubleAttribute* _rotX;
		SmartBody::DoubleAttribute* _rotY;
		SmartBody::DoubleAttribute* _rotZ;
		SrVec initialHPR, initialPos;
		SrVec targetHPR, targetPos;
		float hprStartTime, posStartTime;
		float hprEndTime, posEndTime;		
		bool  smoothTargetHPR, smoothTargetPos;
};

};

#endif