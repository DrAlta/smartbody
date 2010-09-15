#ifndef _HEIGHTFIELD_
#define _HEIGHTFIELD_

#include <windows.h>

#define TERRAIN_SCALE	2000.0f
#define ELEVATION_SCALE 200.0f

class Heightfield
{
	public:
		Heightfield();
		~Heightfield();
	
		void load( char* filename );
		void set_scale( float x, float y, float z );
		void set_origin( float x, float y, float z );
		void set_auto_origin( void );
		void load_normals( void );
		
		void render( void ); 
		float get_elevation( float x, float z, float *normal_p = NULL );

	private:
		unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER* bitmapInfoHeader);
		void initializeTerrain(unsigned char* terrain);
		void calc_normal( float *N_out, float *A, float *B, float *C, float *S );
		void normalize_arr3( float V[ 3 ] );
		
		float get_raw_elevation( int i, int j );

		BITMAPINFOHEADER* _header;
		unsigned char* _imageData;

		int image_width;
		int image_height;
		int num_color_comps;

		int 	mesh_resx;
		int 	mesh_resz;
		float	mesh_scale[ 3 ];
		float	mesh_origin[ 3 ];

		float *vertex_arr;
		float *normal_arr;
		unsigned char *color_arr;
};
#endif
