#ifndef GWIZ_CAMERA_H
#define GWIZ_CAMERA_H

////////////////////////////////
#include "gwiz_math.h"

////////////////////////////////

namespace gwiz	{

class gw_viewport_t	{
	
#if 0
	struct child_link_t {
		gw_viewport_t * child_p;
		child_link_t * next_p;
	};
#endif
	
	public:
	
	gw_viewport_t( void ) {
		border = 0;
		ext_posx = 0;
		ext_posy = 0;
		ext_width = 10;
		ext_height = 10;
		posx = 0;
		posy = 0;
		width = 10;
		height = 10;
		aspect = 1.0;
		mousex = 0;
		mousey = 0;
		mouse_bound = 0;
		norm_mousex = 0.0;
		norm_mousey = 0.0;
		norm_posx = 0.0;
		norm_posy = 0.0;
		norm_width = 1.0;
		norm_height = 1.0;
		parent_p = NULL;
	}
	~gw_viewport_t( void )	{
	}
	
	void set_border( int b = 0 )	{
		border = b;
		resize();
	}
	
	void set_size( int x, int y, int w, int h, int b = 0 )	{
		border = b;
		ext_posx = posx = x;
		ext_posy = posy = y;
		ext_width = width = w;
		ext_height = height = h;
		aspect = (float)width/(float)height;
		norm_posx = 0.0;
		norm_posy = 0.0;
		norm_width = 1.0;
		norm_height = 1.0;
	}
	
	void set_norm( float nx, float ny, float nw, float nh, int b = 0 )	{
		border = b;
		norm_posx = nx;
		norm_posy = ny;
		norm_width = nw;
		norm_height = nh;
	}
	
	void resize( int x, int y, int w, int h )	{
		ext_posx = x;
		ext_posy = y;
		ext_width = w;
		ext_height = h;
		posx = (int)( norm_posx * (float)w ) + x + border;
		posy = (int)( norm_posy * (float)h ) + y + border;
		width = (int)( norm_width * (float)w ) - 2 * border;
		height = (int)( norm_height * (float)h ) - 2 * border;
		aspect = (float)width/(float)height;
	}
	
	void resize( int w, int h )	{
		resize( 0, 0, w, h );
	}
	
	void resize( gw_viewport_t& parent )	{
		parent_p = &parent;
		resize( parent.posx, parent.posy, parent.width, parent.height );
	}
	
	void resize( void )	{
		if( parent_p )	{
			resize( parent_p->posx, parent_p->posy, parent_p->width, parent_p->height );
		}
		else	{
			resize( ext_posx, ext_posy, ext_width, ext_height );
		}
	}

	int set_mouse( int window_x, int window_y )	{
		mousex = window_x - posx;
		mousey = window_y - posy;
		norm_mousex = (float)mousex / (float)( width - 1 );
		norm_mousey = (float)mousey / (float)( height - 1 );
		if( ( mousex < 0 )||( mousex > ( width - 1 ) )||
			( mousey < 0 )||( mousey > ( height - 1 ) ) 
		)	{
			mouse_bound = 0;
		}
		else	{
			mouse_bound = 1;
		}
		return( mouse_bound );
	}
	
	int 	border;
	int 	ext_posx, ext_posy;
	int 	ext_width, ext_height;
	
	int		posx, posy;
	int		width, height;
	float	aspect;
	
	int		mousex, mousey;
	int 	mouse_bound;
	float	norm_mousex, norm_mousey;

	float	norm_posx, norm_posy;
	float	norm_width, norm_height;

	gw_viewport_t* parent_p;
};

////////////////////////////////

class gw_screen_t	{
	
	public:
	
	gw_screen_t( void ) {
		width = 1.0;
		height = 1.0;
		aspect = 1.0;
	}
	~gw_screen_t( void )	{
	}
	
	void set_center( vector_t set_pos, quat_t set_rot, float set_w, float set_h ) {
		
		width = set_w;
		height = set_h;
		aspect = width/height;
		pos = set_pos;
		rot = set_rot;
		update_cache();
	}
	
	void set_corner( vector_t set_bl_pos, quat_t set_rot, float set_w, float set_h ) {
		
		width = set_w;
		height = set_h;
		aspect = width/height;
		rot = set_rot;
		pos = set_bl_pos + rot * vector_t( width * 0.5, height * 0.5, 0.0 );
		update_cache();
	}
	
	void set_corners( vector_t set_bl, vector_t set_br, vector_t set_tl ) {

		width = ( set_br - set_bl ).length();
		height = ( set_tl - set_bl ).length();
		aspect = width/height;
		
		vector_t X_axis = ( set_br - set_bl ).normal();
		vector_t Y_axis = ( set_tl - set_bl ).normal();
		vector_t Z_axis = X_axis.cross( Y_axis ).normal();
		pos = set_bl + X_axis * ( width * 0.5 ) + Y_axis * ( height * 0.5 );
		
		matrix_t linear_mat( X_axis, Y_axis, Z_axis );
		rot = linear_mat.quat( COMP_M_TRSH );
		update_cache();
	}
	
	void update_cache( void )	{
		
		world_mat.identity().translate( pos ).rotate( rot );
		inv_world_mat = -world_mat;

		world_mat.get_col_major( world_mat_arr );
		inv_world_mat.get_col_major( inv_world_mat_arr );
	}
	
	float	width;
	float	height;
	float	aspect;
	
	vector_t	pos; // center of screen
	quat_t		rot; // orientation of screen
	
	matrix_t	world_mat;
	matrix_t	inv_world_mat;

	float_t	world_mat_arr[ 16 ];
	float_t	inv_world_mat_arr[ 16 ];
};

////////////////////////////////

class gw_camera_t	{
	
	public:
	
	gw_camera_t( void ) {
		near = 0.1;
		far = 100.0;
		aspect = 1.0;
		world_mat.identity();
		inv_world_mat.identity();
		frust_mat.identity();
		inv_frust_mat.identity();
	}
	~gw_camera_t( void )	{
	}
	
	void set_clipping( float set_near, float set_far )	{
		near = set_near;
		far = set_far;
	}
	
	void set_placement( vector_t set_pos, quat_t set_rot )	{
		pos = set_pos;
		rot = set_rot;
	}
	
	void set_pivot( float set_distance, vector_t set_center, quat_t set_rot )	{
		pos = set_center + set_rot * vector_t( 0.0, 0.0, set_distance );
		rot = set_rot;
	}
	
	void set_orthographic( float L, float R, float B, float T ) {
		
		aspect = ( R - L ) / ( T - B );
		frust_pos = pos;
		frust_rot = rot;

		frust_mat.identity();
		frust_mat.set( 0, 0, 2.0 / ( R - L ) );
		frust_mat.set( 1, 1, 2.0 / ( T - B ) );
		frust_mat.set( 2, 2, -2.0 / ( far - near ) );
		frust_mat.set( 3, 0, -( R + L ) / ( R - L ) );
		frust_mat.set( 3, 1, -( T + B ) / ( T - B ) );
		frust_mat.set( 3, 2, -( far + near ) / ( far - near ) );

		update_cache();
	}
	
	void set_perspective( float fovy, float set_aspect )	{

		aspect = set_aspect;
		frust_pos = pos;
		frust_rot = rot;

		frust_mat.frustum( fovy, aspect, near, far );
		update_cache();
	}
	
	void set_perspective( gw_screen_t& scr, vector_t eye_offset )	{
		
		aspect = scr.aspect;
		frust_pos = pos + rot * eye_offset;
		frust_rot = scr.rot;

		vector_t loc_pos = scr.inv_world_mat * frust_pos;
		
		float D = loc_pos.z();
		float NoD = near / D;
		
		float L = NoD * ( -0.5 * scr.width - loc_pos.x() );
		float R = NoD * ( 0.5 * scr.width - loc_pos.x() );
		
		float B = NoD * ( -0.5 * scr.height - loc_pos.y() );
		float T = NoD * ( 0.5 * scr.height - loc_pos.y() );
		
		frust_mat.frustum( L, R, B, T, near, far );
		update_cache();
	}

	void set_perspective( gw_screen_t& scr, float eye_dx )	{
	
		set_perspective( scr, vector_t( eye_dx, 0.0, 0.0 ) );
	}
	
	void update_cache( void )	{
	
		world_mat.identity().translate( frust_pos ).rotate( frust_rot );
		inv_world_mat = -world_mat;
		inv_frust_mat = -frust_mat;

		world_mat.get_col_major( world_mat_arr );
		inv_world_mat.get_col_major( inv_world_mat_arr );
	
		frust_mat.get_col_major( frust_mat_arr );
		inv_frust_mat.get_col_major( inv_frust_mat_arr );
	}
	
	vector_t	pos;
	quat_t		rot;
	
	float	near;
	float	far;
	float	aspect;

	vector_t	frust_pos;
	quat_t		frust_rot;
	
	matrix_t	world_mat;
	matrix_t	inv_world_mat;

	matrix_t	frust_mat;
	matrix_t	inv_frust_mat;
	
	float_t	world_mat_arr[ 16 ];
	float_t	inv_world_mat_arr[ 16 ];
	
	float_t	frust_mat_arr[ 16 ];
	float_t	inv_frust_mat_arr[ 16 ];
};

} // namespsace

////////////////////////////////
#endif
