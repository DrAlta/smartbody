#include "Heightfield.h"

#include <windows.h> // standard Windows app include
#include <gl/gl.h> // standard OpenGL include
#include <gl/glu.h> // OpenGL utilties
//#include <gl/glaux.h> // OpenGL auxiliary functions
#include <cstdio>
#include <cstdlib>
#include <math.h>

#define BITMAP_ID 0x4D42
//#define MAP_SCALE 20.0f
//#define PI 3.14159

Heightfield::Heightfield()
{
	_header = NULL;
	_imageData = NULL;
	image_width = 0;
	image_height = 0;
	num_color_comps = 0;
}

Heightfield::~Heightfield()
{
	delete _header;
	if (_imageData)
		delete [] _imageData;
}

void Heightfield::load(char* filename)
{
	if (_header)
		delete _header;
	_header = new BITMAPINFOHEADER();

	_imageData = LoadBitmapFile(filename, _header);
	initializeTerrain(_imageData);
}

void Heightfield::render()
{
	if (true)
	{
		//we are going to loop through all of our terrain's data points,
		//but we only want to draw one triangle strip for each set along the x axis.
		for (int z = 0; z < TERRAIN_SIZE_Z-1; z++ )
		{
			glBegin(GL_TRIANGLE_STRIP);
			for (int x = 0; x < TERRAIN_SIZE_X-1; x++ )
			{
				//for each vertex, we calculate the grayscale shade color,
				//we set the texture coordinate, and we draw the vertex.

				/*
				the vertices are drawn in this order:

				0 --->1
					/
				   /
				  /
				2 --->3
					   */

				//draw vertex 0
				glColor3f(_terrain[x][z][1]/255.0f, _terrain[x][z][1]/255.0f, _terrain[x][z][1]/255.0f);
				glVertex3f(_terrain[x][z][0], _terrain[x][z][1], _terrain[x][z][2]);

				//draw vertex 1
				glColor3f(_terrain[x+1][z][1]/255.0f, _terrain[x+1][z][1]/255.0f, _terrain[x+1][z][1]/255.0f);
				glVertex3f(_terrain[x+1][z][0], _terrain[x+1][z][1], _terrain[x+1][z][2]);

				//draw vertex 2
				glColor3f(_terrain[x][z+1][1]/255.0f, _terrain[x][z+1][1]/255.0f, _terrain[x][z+1][1]/255.0f);
				glVertex3f(_terrain[x][z+1][0], _terrain[x][z+1][1], _terrain[x][z+1][2]);

				//draw vertex 3
				glColor3f(_terrain[x+1][z+1][1]/255.0f, _terrain[x+1][z+1][1]/255.0f, _terrain[x+1][z+1][1]/255.0f);
				glVertex3f(_terrain[x+1][z+1][0], _terrain[x+1][z+1][1], _terrain[x+1][z+1][2]);
			}
			glEnd();
		}
	}
}

void Heightfield::initializeTerrain(unsigned char* terrain)
{

	int hist[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int n = image_width * image_height * num_color_comps;
	for( int i=0; i< n; i++ )	{
		hist[ (int)( terrain[ i ] ) / 32 ]++;
	}
	printf( "HIST: %d %d %d %d %d %d %d %d\n", hist[0], hist[1], hist[2], hist[3], hist[4], hist[5], hist[6], hist[7] );

	// loop through all of the heightfield points, calculating the coordinates for each point
	for (int z = 0; z < TERRAIN_SIZE_Z; z++)
	{
		for (int x = 0; x < TERRAIN_SIZE_X; x++)
		{
			unsigned char celev = _imageData[ ( z * image_width + x ) * num_color_comps ];
			_terrain[x][z][1] = ELEVATION_SCALE * (float)celev / 255.0f;
			_terrain[x][z][0] = float(x)*TERRAIN_SCALE / ( TERRAIN_SIZE_X - 1 );
			_terrain[x][z][2] = -float(z)*TERRAIN_SCALE / ( TERRAIN_SIZE_Z - 1 );
		}
	}
}


unsigned char* Heightfield::LoadBitmapFile(char *filename, BITMAPINFOHEADER * bitmapInfoHeader)
{
	FILE *filePtr; //the file pointer
	BITMAPFILEHEADER bitmapFileHeader; //bitmap file header
	unsigned char *bitmapImage; //bitmap image data
	int imageIdx = 0; //image index counter
	unsigned char tempRGB; //swap variable

	printf( "HF:BM: open '%s'\n", filename );

	//open filename in "read binary" mode
	filePtr = fopen(filename, "rb");
	if( filePtr == NULL)
	return NULL;

	//read the bitmap file header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	printf( "HF:BM: FileHeader:\n" );
	printf( "HF:BM: bfType %d == %d\n", bitmapFileHeader.bfType, BITMAP_ID );
	printf( "HF:BM: bfSize %d\n", bitmapFileHeader.bfSize );

	//verify that this is a bitmap by checking for the universal bitmap id
	if( bitmapFileHeader.bfType != BITMAP_ID )
	{
		fclose(filePtr);
		return NULL;
	}

	//read the bitmap information header
	fread(bitmapInfoHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	printf( "HF:BM: InfoHeader:\n" );
	printf( "HF:BM: biSize %d\n", bitmapInfoHeader->biSize );
	printf( "HF:BM: biWidth %d\n", bitmapInfoHeader->biWidth );
	printf( "HF:BM: biHeight %d\n", bitmapInfoHeader->biHeight );
	printf( "HF:BM: biPlanes %d\n", bitmapInfoHeader->biPlanes );
	printf( "HF:BM: biBitCount %d\n", bitmapInfoHeader->biBitCount );
	printf( "HF:BM: biCompression %d\n", bitmapInfoHeader->biCompression );
	printf( "HF:BM: biSizeImage %d\n", bitmapInfoHeader->biSizeImage );
	printf( "HF:BM: biClrUsed %d\n", bitmapInfoHeader->biClrUsed );
	printf( "HF:BM: biClrImportant %d\n", bitmapInfoHeader->biClrImportant );

	image_width = bitmapInfoHeader->biWidth;
	image_height = bitmapInfoHeader->biHeight;
	long image_area = image_width * image_height;
	num_color_comps = 0;
	for( int i=1; num_color_comps == 0; i++ )	{
		if( ( image_area * ( i + 1 ) ) > bitmapFileHeader.bfSize )	{
			num_color_comps = i;
		}
	}
	printf( "HF:BM: color comps %d\n", num_color_comps );
	long image_bytes = image_area * num_color_comps;

	//move file pointer to beginning of bitmap data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	//allocate enough memory for the bitmap image data
//	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);
	bitmapImage = (unsigned char*)malloc( image_bytes );

	//verify memory allocation
	if(!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	//read in the bitmap image data
//	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	size_t bytes_read = fread( bitmapImage, 1, image_bytes, filePtr );
	printf( "HF:BM: bytes read %d\n", bytes_read );

	//make sure bitmap image data was read
//	if (bitmapImage == NULL)
//	{
//		fclose(filePtr);
//		return NULL;
//	}

#if 0
	//swap the R and B values to get RGB since the bitmap color format is in BGR
	for( imageIdx = 0; imageIdx < image_bytes; imageIdx += 3 )
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
#endif

	//close the file and return the bitmap image data
	fclose(filePtr);

	return bitmapImage;
}

float Heightfield::getHeight( int x, int z )
{
    float h=0;
  
    if ( x < 0 ) x = 0;
    if ( z < 0 ) z = 0;
    if ( x > TERRAIN_SIZE_X - 1 ) x = TERRAIN_SIZE_X - 1;
    if ( z > TERRAIN_SIZE_Z - 1 ) z = TERRAIN_SIZE_Z - 1;

    
    h = _terrain[x][z][1];

    return h;
}

float Heightfield::getHeight( float x, float z )
{
	float dnX = floorf( x * 1.0f / TERRAIN_SIZE_X);
	float dnZ = floorf( z * 1.0f / TERRAIN_SIZE_Z );

    float dx = ( x - ( dnX * TERRAIN_SIZE_X ) ) * 1.0f / TERRAIN_SIZE_X;
    float dz = ( z - ( dnZ * TERRAIN_SIZE_Z ) ) * 1.0f / TERRAIN_SIZE_Z;

    int nX = int( dnX );
    int nZ = int( dnZ );

    float y, y0;

    if ( dx + dz <= float( 1.0 ) ) // Use <= comparison to prefer simpler branch
    {
        y0 = getHeight( nX, nZ );

        y = y0 + ( getHeight( nX + 1, nZ ) - y0 ) * dx
            + ( getHeight( nX, nZ + 1 ) - y0 ) * dz;
    }
    else
    {
        y0 = getHeight( nX + 1, nZ + 1 );

        y = y0	+ ( getHeight( nX + 1, nZ ) - y0 ) * ( float(1.0) - dz ) +
            ( getHeight( nX, nZ + 1 ) - y0 ) * ( float(1.0) - dx );
    }

    return y;
}






