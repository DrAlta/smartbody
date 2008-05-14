/*
 *  sk_skeleton.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently UC Merced)
 *      Andrew n marshall, USC
 *      Marcus Thiebaux, USC
 */

# include <SR/sr_model.h>

# include <SK/sk_skeleton.h>
# include <SK/sk_posture.h>

//============================ SkSkeleton ============================

SkSkeleton::SkSkeleton ()
 {
   _name = "noname";
   _root = 0;
   _coldetid = -1;       // index used in collision detection
   _gmat_uptodate = false;
   _channels = new SkChannelArray;
   _channels->ref();
 }

SkSkeleton::~SkSkeleton ()
 {
   init ();
   _channels->unref();
 }

void SkSkeleton::init ()
 {
   _coldet_free_pairs.size(0);
   _channels->init();
   while ( _postures.size()>0 ) _postures.pop()->unref();
   while ( _joints.size()>0 ) delete _joints.pop();
   _jhash.init(0);
   _root = 0;
   _gmat_uptodate = false;
 }

SkJoint* SkSkeleton::add_joint ( SkJoint::RotType rtype, int parentid )
{
    _jhash.init(0);

    SkJoint* parent=0;
	if ( parentid<0 ) {
		if( _joints.size()>0 )
			parent=_joints.top();
		else
			parent=NULL;
	} else {
		parent = _joints.get( parentid );
	}
   
    SkJoint* j = new SkJoint ( this, parent, rtype, _joints.size() );
    _joints.push() = j;
   
    if ( parent==NULL )
		_root=j;
   
    return j;
}

SkJoint* SkSkeleton::insert_new_root_joint ( SkJoint::RotType rtype )
{
    _jhash.init(0);
	SkJoint* old_root = _root;

    _root = new SkJoint ( this, 0, rtype, _joints.size() );
    _joints.push() = _root;

	if( old_root ) {
		old_root->_parent = _root;

		/* thiebaux 6/19/2006 */
		_root->_children.push( old_root );
	}

    return _root;
}

SkJoint* SkSkeleton::linear_search_joint ( const char* n ) const
 {
   if ( !SkJointName::exist(n) ) return 0;
   
   int i;
   SkJointName name(n);
   for ( i=0; i<_joints.size(); i++ )
    { if ( _joints[i]->name()==name ) return _joints[i];
    }
   return 0;
 }
 
SkJoint* SkSkeleton::search_joint ( const char* n )
 {
   if ( !SkJointName::exist(n) ) return 0;

   // Build the table in case it was not already built: 
   if ( _jhash.elements()==0 )
    { int i, jsize = _joints.size();
      _jhash.init ( jsize*2 );
      for ( i=0; i<jsize; i++ )
       { _jhash.insertstat ( _joints[i]->name(), _joints[i] );
         // only the first entry of duplicated names is inserted
       }
      //sr_out<<"Skeleton Hash Table Longest Entry: "<<_jhash.longest_entry()<<srnl;
      //sr_out<<"Skeleton Hash Table Size: "<<_jhash.size()<<srnl;
    }
    
   // ok, now search the name:
   return (SkJoint*) _jhash.lookup(n);
 }

void SkSkeleton::update_global_matrices ()
 {
   if ( _gmat_uptodate ) return;
   _root->update_gmat();
   _gmat_uptodate = true;
 }

void SkSkeleton::compress ()
 {
   _coldet_free_pairs.compress();
   _channels->compress();
   _postures.compress();
   _joints.compress();

   int i;
   for ( i=0; i<_joints.size(); i++ )
     _joints[i]->_children.compress();
 }

void SkSkeleton::set_geo_local ()
 {
   int i;
   SkJoint* j;
   
   for ( i=0; i<_joints.size(); i++ ) _joints[i]->init_values();
   update_global_matrices ();

   SrMat mat;
   for ( i=0; i<_joints.size(); i++ )
    { j = _joints[i];
      mat = j->gmat();
      mat.invert();
      if ( j->_visgeo ) j->_visgeo->apply_transformation(mat);
      if ( j->_colgeo ) j->_colgeo->apply_transformation(mat);
    }

   update_global_matrices ();
}

//============================ End of File ============================
