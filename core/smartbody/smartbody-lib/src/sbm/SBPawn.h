#ifndef _SBPAWN_H_
#define _SBPAWN_H_

#include <sbm/sbm_pawn.hpp>
#include <sbm/SBSkeleton.h>

namespace SmartBody {

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
		void setOrientation(SrQuat quat);
		void setHPR(SrVec hpr);
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
};

};

#endif