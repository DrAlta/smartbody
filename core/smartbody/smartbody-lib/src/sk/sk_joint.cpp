/*
 *  sk_joint.cpp - part of Motion Engine and SmartBody-lib
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
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Andrew n marshall, USC
 */

# include <math.h>

# include <SR/sr_model.h>

# include <SK/sk_joint.h>
# include <SK/sk_skeleton.h>

//============================= SkJoint ============================

SkJoint::SkJoint ( SkSkeleton* sk, SkJoint* parent, RotType rtype, int i )
          : _pos ( 0 )
 {
   _visgeo = 0;
   _colgeo = 0;

   _coldetid = -1;

   _parent = parent;

   _lmat_uptodate = 0;
   _name = 0;
   _index = i;
   _skeleton = sk;

   _pos._joint = this; // as we initialized it with null (to avoid a warning)
   _quat = 0;
   _rtype =-1;
   rot_type ( rtype );

   // TODO: verify we are a parent/ancestor of the parent
   if( parent )
	   parent->_children.push() = this;
 }

SkJoint::~SkJoint()
 {
   if ( _visgeo ) _visgeo->unref();
   if ( _colgeo ) _colgeo->unref();

   delete _quat;
 }

void SkJoint::visgeo ( SrModel* m )
 { 
   if ( m ) m->ref();
   if ( _visgeo ) _visgeo->unref();
   _visgeo = m;
 }

void SkJoint::colgeo ( SrModel* m )
 { 
   if ( m ) m->ref();
   if ( _colgeo ) _colgeo->unref();
   _colgeo = m;
 }

void SkJoint::offset ( const SrVec& o )
 {
   if ( o==_offset ) return;
   _offset=o;
   set_lmat_changed ();
 }

void SkJoint::rot_type ( RotType t )
 {
   if ( char(t)==_rtype ) return; // no change required
   
   delete _quat;
   _rtype = (char)t;

   set_lmat_changed ();

   switch (_rtype)
    { case TypeEuler: _quat = new SkJointEuler(this); break;
      case TypeSwingTwist: _quat = new SkJointSwingTwist(this); break;
      default: _rtype = TypeQuat;
               _quat = new SkJointQuat(this);
    }
 }

void SkJoint::init_pos ()
 {
   if ( !pos()->frozen(0) ) pos()->value(0,0);
   if ( !pos()->frozen(1) ) pos()->value(1,0);
   if ( !pos()->frozen(2) ) pos()->value(2,0);
 }

void SkJoint::init_rot ()
 {
   if ( !quat()->active() ) return;
   switch (_rtype)
    { case TypeEuler: if ( !euler()->frozen(0) ) euler()->value(0,0);
                      if ( !euler()->frozen(1) ) euler()->value(1,0);
                      if ( !euler()->frozen(2) ) euler()->value(2,0); break;
      case TypeSwingTwist: st()->swing ( 0, 0 );
                           if ( !st()->twist_frozen() ) st()->twist ( 0 ); break;
      default: quat()->value ( SrQuat::null ); break;
    }
 }

void SkJoint::update_lmat ()
 {
   if ( _lmat_uptodate ) return;
   _lmat_uptodate = 1;

   // update the 3x3 rotation submatrix if required:
   if ( !_quat->_jntsync )
    { _quat->_jntsync = 1;
      SrQuat q = _quat->value();

      float x2  = q.x+q.x;
      float x2x = x2*q.x;
      float x2y = x2*q.y;
      float x2z = x2*q.z;
      float x2w = x2*q.w;
      float y2  = q.y+q.y;
      float y2y = y2*q.y;
      float y2z = y2*q.z;
      float y2w = y2*q.w;
      float z2  = q.z+q.z;
      float z2z = z2*q.z;
      float z2w = z2*q.w;

      _lmat[0] = 1.0f - y2y - z2z; _lmat[1] = x2y + z2w;        _lmat[2]  = x2z - y2w;
      _lmat[4] = x2y - z2w;        _lmat[5] = 1.0f - x2x - z2z; _lmat[6]  = y2z + x2w;
      _lmat[8] = x2z + y2w;        _lmat[9] = y2z - x2w;        _lmat[10] = 1.0f - x2x - y2y;

      if (_lmat[0]==0 && _lmat[1]==0 && _lmat[2]==0) _lmat=SrMat::id; // to avoid a null matrix
    }

   // now update offset + translation:
   _lmat[12] = _pos.value(0) + _offset.x;
   _lmat[13] = _pos.value(1) + _offset.y;
   _lmat[14] = _pos.value(2) + _offset.z;
 }

void SkJoint::update_gmat ()
 {
   const SrMat& pmat = _parent? _parent->_gmat : SrMat::id;

   update_lmat ();

   _gmat.mult ( _lmat, pmat );

   int i;
   for ( i=0; i<_children.size(); i++ )
    { _children[i]->update_gmat();
    }
 }

void SkJoint::update_gmat ( SrArray<SkJoint*>& end_joints )
 {
   const SrMat& pmat = _parent? _parent->_gmat : SrMat::id;

   update_lmat ();

   _gmat.mult ( _lmat, pmat );

   int i;

   for ( i=0; i<end_joints.size(); i++ )
    if ( this==end_joints[i] ) return;

   for ( i=0; i<_children.size(); i++ )
    { _children[i]->update_gmat(end_joints);
    }
 }

void SkJoint::update_gmat_local ()
 {
   update_lmat ();
   _gmat.mult ( _lmat, _parent? _parent->_gmat : SrMat::id );
 }

void SkJoint::update_gmat_up ( SkJoint* stop_joint )
 {
   SrArray<SkJoint*> joints;
   joints.capacity ( 64 );
   
   SkJoint* j = this;
   do { joints.push() = j;
        j = j->_parent;
      } while ( j!=0 && j!=stop_joint );
      
   while ( joints.size()>0 )
    { joints.pop()->update_gmat_local();
    }
 }

void SkJoint::set_lmat_changed ()
 {
   _lmat_uptodate = 0;
   _skeleton->invalidate_global_matrices();
 }

static void _unite ( const SkJoint* j, SrModel& m, SrModel& tmp, bool ifvisgeo )
 {
   const SrModel* geo=0;

   if (  ifvisgeo && j->visgeo() ) geo = j->visgeo();
   if ( !ifvisgeo && j->colgeo() ) geo = j->colgeo();

   if ( geo )
    { tmp = *geo;
      tmp.apply_transformation ( j->gmat() );
      m.add_model ( tmp );
    }

   int i;
   for ( i=0; i<j->num_children(); i++ )
    { _unite ( j->child(i), m, tmp, ifvisgeo );
    }
 }

void SkJoint::unite_visgeo ( SrModel& m )
 {
   update_gmat();
   m.init ();
   SrModel tmp;
   _unite ( this, m, tmp, true );
 }

void SkJoint::unite_colgeo ( SrModel& m )
 {
   update_gmat();
   m.init ();
   SrModel tmp;
   _unite ( this, m, tmp, false );
 }

//============================ End of File ============================
