#ifndef _VOXELIZERWINDOW_
#define _VOXELIZERWINDOW_

#include <FL/Fl_Gl_Window.H>
#include <sr/sr_model.h>
#include <string>
#include "PolyVoxCore/SimpleVolume.h"
#include "PolyVoxCore/CubicSurfaceExtractorWithNormals.h"
#include "PolyVoxCore/CubicSurfaceExtractor.h"
#include "PolyVoxCore/MarchingCubesSurfaceExtractor.h"
#include "PolyVoxCore/SurfaceMesh.h"
#include "PolyVoxCore/MeshDecimator.h"

class VoxelizerWindow : public Fl_Gl_Window
{
public:
	VoxelizerWindow(int x, int y, int w, int h, char* name);
	~VoxelizerWindow();

	void initVoxelizer(SrModel* inMesh, int voxelRes);
	bool isFinishBuildVoxels();
	// just for testing....should be removed later
	void saveVoxels(std::string outFilename);

	PolyVox::SimpleVolume<uint8_t>* getVoxels();	
	PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>* getNormalizedVoxelMesh();
	virtual void draw();	
protected:
	bool finishBuildVoxels;
	bool startBuildVoxels;
	SrModel* mesh;
	PolyVox::SimpleVolume<uint8_t>* voxels; // use uint8 to tell between inside, outside, and shell
	PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>* voxelMesh;
	SrVec voxelCenter;
	float voxelScale;

	// generate a slice of volume from render buffer	
	void generateVoxelsParityVote();
	void generateVoxelsZBufferCarving();
	void generateSlice(int dir, int depth, void* sliceA, void* sliceB, bool readZBuffer = false);
	void drawModel();
	// using parity voting to determine the solid voxels
	void voxelVoting(int dir, int depth, unsigned char* sliceA, unsigned char* sliceB); 
	void voxelCarving(int dir, float* sliceA, float* sliceB); 
	void clearVoxels(PolyVox::SimpleVolume<uint8_t>* vol, uint8_t clearValue);

	void voxelFindConnectedComponents(PolyVox::SimpleVolume<uint8_t>* vol);

};

#endif
