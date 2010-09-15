#include "Heightfield.h"

#include <windows.h> // standard Windows app include
#include <gl/gl.h> // standard OpenGL include
#include <gl/glu.h> // OpenGL utilties
//#include <gl/glaux.h> // OpenGL auxiliary functions
#include <cstdio>
#include <cstdlib>
#include <math.h>

#define BITMAP_ID 0x4D42

Heightfield::Heightfield( void )	{

	_header = NULL;
	_imageData = NULL;
	
	image_width = 0;
	image_height = 0;
	num_color_comps = 0;
	
	mesh_resx = 0;
	mesh_resz = 0;

	mesh_scale[ 0 ] = 1.0f;
	mesh_scale[ 1 ] = 1.0f;
	mesh_scale[ 2 ] = 1.0f;

	mesh_origin[ 0 ] = 0.0f;
	mesh_origin[ 1 ] = 0.0f;
	mesh_origin[ 2 ] = 0.0f;
	
	vertex_arr = NULL;
	normal_arr = NULL;
	color_arr = NULL;
}

Heightfield::~Heightfield()
{
	delete _header;
	if (_imageData)
		delete [] _imageData;
		
	if( vertex_arr )	{
		delete [] vertex_arr;
	}
	if( normal_arr )	{
		delete [] normal_arr;
	}
	if( color_arr )	{
		delete [] color_arr;
	}
}

void Heightfield::load(char* filename)
{
	if (_header)
		delete _header;
	_header = new BITMAPINFOHEADER();

	_imageData = LoadBitmapFile(filename, _header);
	initializeTerrain(_imageData);
}

void Heightfield::render( void )	{

	if( vertex_arr )	{
		glPushMatrix();
		glTranslatef( mesh_origin[ 0 ], mesh_origin[ 1 ], mesh_origin[ 2 ] );
		glScalef( mesh_scale[ 0 ], mesh_scale[ 1 ], mesh_scale[ 2 ] );

		for( int j = 0; j < mesh_resz - 1; j++ )	{

			glBegin( GL_TRIANGLE_STRIP );
			for( int i = 0; i < mesh_resx - 1; i++ )	{

				int index = ( j * mesh_resx + i ) * 3;
				glColor3ubv( color_arr + index );
				glVertex3fv( vertex_arr + index );

				index = ( ( j + 1 ) * mesh_resx + i ) * 3;
				glColor3ubv( color_arr + index );
				glVertex3fv( vertex_arr + index );

				index = ( j * mesh_resx + i + 1 ) * 3;
				glColor3ubv( color_arr + index );
				glVertex3fv( vertex_arr + index );

				index = ( ( j + 1 ) * mesh_resx + i + 1 ) * 3;
				glColor3ubv( color_arr + index );
				glVertex3fv( vertex_arr + index );
			}
			glEnd();
		}
		glPopMatrix();
	}
}

void Heightfield::initializeTerrain(unsigned char* terrain)
{

#if 0
	int hist[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int n = image_width * image_height * num_color_comps;
	for( int i=0; i< n; i++ )	{
		hist[ (int)( terrain[ i ] ) / 32 ]++;
	}
	printf( "HIST: %d %d %d %d %d %d %d %d\n", hist[0], hist[1], hist[2], hist[3], hist[4], hist[5], hist[6], hist[7] );
#endif

	vertex_arr = new float[ image_width * image_height * 3 ];
	normal_arr = new float[ image_width * image_height * 3 ];
	color_arr = new unsigned char[ image_width * image_height * 3 ];

	for( int j = 0; j < image_height; j++ )	{
		for( int i = 0; i < image_width; i++ )	{
			
			int img_index = ( j * image_width + i ) * num_color_comps;
			int arr_index = ( j * image_width + i ) * 3;
			
			vertex_arr[ arr_index + 0 ] = (float) i / (float)( image_width - 1 );
			vertex_arr[ arr_index + 1 ] = (float) _imageData[ img_index ] / 255.0f;
			vertex_arr[ arr_index + 2 ] = (float) j / (float)( image_height - 1 );
			
			color_arr[ arr_index + 0 ] = _imageData[ img_index ];
			color_arr[ arr_index + 1 ] = _imageData[ img_index ];
			color_arr[ arr_index + 2 ] = _imageData[ img_index ];
		}
	}

	mesh_resx = image_width;
	mesh_resz = image_height;

#if 0
	mesh_scale[ 0 ] = TERRAIN_SCALE;
	mesh_scale[ 1 ] = ELEVATION_SCALE;
	mesh_scale[ 2 ] = TERRAIN_SCALE;

	mesh_origin[ 0 ] = -mesh_scale[ 0 ] * 0.5f;
	mesh_origin[ 1 ] = 0.0f;
	mesh_origin[ 2 ] = -mesh_scale[ 2 ] * 0.5f;

	float center_elev = getHeight( 0.0f, 0.0f );
	mesh_origin[ 1 ] = -center_elev;
#endif

#if 0
	for( int i = 0; i <= 10; i++ )	{
//		float px = 0.0;
//		float px = 1.0;
		float px = (float)i/10.0;
//		float px = 1.0f - (float)i/10.0f;
//		float pz = 0.0;
//		float pz = 1.0;
		float pz = (float)i/10.0;
//		float pz = 1.0f - (float)i/10.0f;
		float y = getHeight( px, pz );
		printf( "%f %f : %f\n", px, pz, y );
	}
#endif
}

void Heightfield::set_scale( float x, float y, float z )	{

	mesh_scale[ 0 ] = x;
	mesh_scale[ 1 ] = y;
	mesh_scale[ 2 ] = z;
}

void Heightfield::set_origin( float x, float y, float z )	{

	mesh_origin[ 0 ] = x;
	mesh_origin[ 1 ] = y;
	mesh_origin[ 2 ] = z;
}

void Heightfield::set_auto_origin( void )	{

	mesh_origin[ 0 ] = -mesh_scale[ 0 ] * 0.5f;
	mesh_origin[ 1 ] = 0.0f;
	mesh_origin[ 2 ] = -mesh_scale[ 2 ] * 0.5f;

	float center_elev = getHeight( 0.0f, 0.0f );
	mesh_origin[ 1 ] = -center_elev;
}

unsigned char* Heightfield::LoadBitmapFile(char *filename, BITMAPINFOHEADER * bitmapInfoHeader)
{
	FILE *filePtr; //the file pointer
	BITMAPFILEHEADER bitmapFileHeader; //bitmap file header
	unsigned char *bitmapImage; //bitmap image data

#if 0
	bitmapImage = (unsigned char*)malloc( 4 );
	image_width = 2;
	image_height = 2;
	num_color_comps = 1;
	bitmapImage[ 0 ] = 0;
	bitmapImage[ 1 ] = 63;
	bitmapImage[ 2 ] = 0;
	bitmapImage[ 3 ] = 255;
	return bitmapImage;
#endif

#if 0
	bitmapImage = (unsigned char*)malloc( 9 );
	image_width = 3;
	image_height = 3;
	num_color_comps = 1;
	bitmapImage[ 0 ] = 0;
	bitmapImage[ 1 ] = 0;
	bitmapImage[ 2 ] = 0;
	bitmapImage[ 3 ] = 0;
	bitmapImage[ 4 ] = 255;
	bitmapImage[ 5 ] = 0;
	bitmapImage[ 6 ] = 0;
	bitmapImage[ 7 ] = 0;
	bitmapImage[ 8 ] = 0;
	return bitmapImage;
#endif

	printf( "Heightfield::LoadBitmapFile: open '%s'\n", filename );

	//open filename in "read binary" mode
	filePtr = fopen( filename, "rb" );
	if( filePtr == NULL)	{
		printf( "Heightfield::LoadBitmapFile ERR: fopen '%s' FAILED\n", filename );
		return NULL;
	}
	
	//read the bitmap file header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

#if 0
	printf( "HF:BM: FileHeader:\n" );
	printf( "HF:BM: bfType %d == %d\n", bitmapFileHeader.bfType, BITMAP_ID );
	printf( "HF:BM: bfSize %d\n", bitmapFileHeader.bfSize );
#endif

	//verify that this is a bitmap by checking for the universal bitmap id
	if( bitmapFileHeader.bfType != BITMAP_ID )
	{
		fclose(filePtr);
		return NULL;
	}

	//read the bitmap information header
	fread(bitmapInfoHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

#if 0
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
#endif

	image_width = bitmapInfoHeader->biWidth;
	image_height = bitmapInfoHeader->biHeight;
	long image_area = image_width * image_height;
	num_color_comps = 0;
	for( int i=1; num_color_comps == 0; i++ )	{
		if( ( image_area * ( i + 1 ) ) > (int)( bitmapFileHeader.bfSize ) )	{
			num_color_comps = i;
		}
	}
	printf( " image: width %d height %d comps %d\n", image_width, image_height, num_color_comps );
	long image_bytes = image_area * num_color_comps;

	//move file pointer to beginning of bitmap data
	fseek( filePtr, bitmapFileHeader.bfOffBits, SEEK_SET );

	//allocate enough memory for the bitmap image data
	bitmapImage = (unsigned char*)malloc( image_bytes );

	//verify memory allocation
	if( bitmapImage == NULL )	{
		printf( "Heightfield::LoadBitmapFile ERR: malloc %d bytes FAILED\n", image_bytes );
		fclose(filePtr);
		return NULL;
	}

	//read in the bitmap image data
	size_t bytes_read = fread( bitmapImage, 1, image_bytes, filePtr );

	fclose(filePtr);

	//make sure bitmap image data was read
	if( (int)( bytes_read ) < image_bytes )	{
		printf( "Heightfield::LoadBitmapFile ERR: file '%s': read %d of %d bytes\n", bytes_read, image_bytes );
		return NULL;
	}

#if 0
	unsigned char tempRGB; //swap variable
	//swap the R and B values to get RGB since the bitmap color format is in BGR
	for( int imageIdx = 0; imageIdx < image_bytes; imageIdx += 3 )
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
#endif

	return bitmapImage;
}

float Heightfield::getNHeight( int i, int j )	{

	if( i < 0 ) return( 0.0f );
	if( j < 0 ) return( 0.0f );
	if( i >= mesh_resx ) return( 0.0f );
	if( j >= mesh_resz ) return( 0.0f );
	return( vertex_arr[ ( j * image_width + i ) * 3 + 1 ] );
}

float Heightfield::getHeight( float px, float pz )	{

//	float dnX = floorf( x / mesh_resx );
//	float dnZ = floorf( z / mesh_resz );

//    float dx = ( x - ( dnX * mesh_resx ) ) / mesh_resx;
//    float dz = ( z - ( dnZ * mesh_resz ) ) / mesh_resz;

	float nx = ( px - mesh_origin[ 0 ] ) / mesh_scale[ 0 ];
	float nz = ( pz - mesh_origin[ 2 ] ) / mesh_scale[ 2 ];
	
	float ix = nx * ( mesh_resx - 1 );
	float iz = nz * ( mesh_resz - 1 );
	
	int i = (int) ix;
	int j = (int) iz;
	
	float dx = ix - (float)i;
	float dz = iz - (float)j;
	
    float y, y0;

	if( ( dx + dz ) < 1.0f )	{

		y0 = getNHeight( i, j );
		y = y0 
			+ ( getNHeight( i + 1, j ) - y0 ) * dx
			+ ( getNHeight( i, j + 1 ) - y0 ) * dz;
	}
	else	{
	
		y0 = getNHeight( i + 1, j + 1 );
		y = y0 
			+ ( getNHeight( i + 1, j ) - y0 ) * ( 1.0f - dz )
			+ ( getNHeight( i, j + 1 ) - y0 ) * ( 1.0f - dx );
	}
	
#if 0
	float x = ( px - mesh_origin[ 0 ] ) / mesh_scale[ 0 ];
	float z = ( pz - mesh_origin[ 2 ] ) / mesh_scale[ 2 ];

	float dnX = floorf( x / ( mesh_resx - 1 ) );
	float dnZ = floorf( z / ( mesh_resz - 1 ) );

    float dx = ( x - ( dnX * ( mesh_resx - 1 ) ) ) / ( mesh_resx - 1 );
    float dz = ( z - ( dnZ * ( mesh_resz - 1 ) ) ) / ( mesh_resz - 1 );

    int nX = int( dnX );
    int nZ = int( dnZ );

    float y, y0;

    if ( dx + dz <= 1.0f ) 
    {
        y0 = getNHeight( nX, nZ );

        y = y0 + ( getNHeight( nX + 1, nZ ) - y0 ) * dx
            + ( getNHeight( nX, nZ + 1 ) - y0 ) * dz;
    }
    else
    {
        y0 = getNHeight( nX + 1, nZ + 1 );

        y = y0	+ ( getNHeight( nX + 1, nZ ) - y0 ) * ( 1.0f - dz ) +
            ( getNHeight( nX, nZ + 1 ) - y0 ) * ( 1.0f - dx );
    }
#endif

	float py = y * mesh_scale[ 1 ] + mesh_origin[ 1 ];
    return( py );
}






