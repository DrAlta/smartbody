#ifndef _BITMAPUTILITY_
#define _BITMAPUTILITY_

#include <windows.h>

#define TERRAIN_SIZE_X 512
#define TERRAIN_SIZE_Z 512
#define TERRAIN_SCALE	1000.0f
#define ELEVATION_SCALE 100.0f

class Heightfield
{
	public:
		Heightfield();
		~Heightfield();
	
		void load(char* filename);
		void render(); 
		float getHeight(float x, float y);
		float getHeight( int x, int z );

	private:
		unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER* bitmapInfoHeader);
		void initializeTerrain(unsigned char* terrain);

		BITMAPINFOHEADER* _header;
		unsigned char* _imageData;
		int image_width;
		int image_height;
		int num_color_comps;
		float _terrain[TERRAIN_SIZE_X][TERRAIN_SIZE_Z][3];

};
#endif
