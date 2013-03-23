#ifndef _SBNAVIGATIONMESH_H_
#define _SBNAVIGATIONMESH_H_

#include <vhcl.h>
#include <sb/SBTypes.h>
#include <sr/sr_model.h>
#include <string>
#include <vector>
#include <map>

class dtNavMesh;
class dtNavMeshQuery;

namespace SmartBody {

class SBNavigationMesh
{
	public:
		SBAPI SBNavigationMesh();		
		SBAPI ~SBNavigationMesh();		
		SBAPI bool buildNavigationMesh(SrModel& inMesh);	
		SBAPI SrModel* getRawMesh();
		SBAPI SrModel* getNavigationMesh();
		SBAPI float queryFloorHeight(SrVec pos, SrVec searchSize);
		SBAPI SrVec queryMeshPoint(SrVec& p1, SrVec& p2);
		SBAPI void findPath(SrVec& spos, SrVec& epos, std::vector<SrVec>& pathList);
	protected:
		float m_cellSize;
		float m_cellHeight;
		float m_agentHeight;
		float m_agentRadius;
		float m_agentMaxClimb;
		float m_agentMaxSlope;
		float m_regionMinSize;
		float m_regionMergeSize;
		bool m_monotonePartitioning;
		float m_edgeMaxLen;
		float m_edgeMaxError;
		float m_vertsPerPoly;
		float m_detailSampleDist;
		float m_detailSampleMaxError;
		SrModel* rawMesh; // the orignal model
		SrModel* naviMesh; // the navigation mesh built from original model

		class dtNavMesh* m_navMesh;
		class dtNavMeshQuery* m_navQuery;

};

}


#endif