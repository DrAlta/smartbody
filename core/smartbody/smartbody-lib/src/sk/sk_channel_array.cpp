/*
 *  sk_channel_array.cpp - part of Motion Engine and SmartBody-lib
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
 *      Marcus Thiebaux, USC
 */

# include <SK/sk_channel_array.h>
# include <SK/sk_skeleton.h>

//========================= SkChannelArray::HashTable ============================

/* We use an internal specific implementation of a hash table for fast channel search.
   The hash table is created and used whenever methods SkChannelArray::search() and 
   SkChannelArray::map() are called (methods implementation in the end of this file)*/
class SkChannelArray::HashTable
 { public:
    struct Entry
     { int    jname_key;   // SkJointName::id()
       char   ctype_key;   // the SkChannel::Type enumerator [-127,128] (see sr.h)
       srword pos_data;    // the position of the key in the channel array max is 65,535
       srsint next;        // the index of the next colliding item in [-32768,32767]
       void set ( int jk, char ck, srword pos, srsint n )
        { jname_key=jk; ctype_key=ck; pos_data=pos; next=n; }
     };
    SrArray<Entry> table;
    int hash_size;
   public:
    HashTable () { hash_size=0; }
    void init ( int hsize );
    int collisions () const { return table.size()-hash_size; }
    int longest_entry () const;                       // for inspection purposes only
    int lookup ( int jname, char ctype ) const;       // returns position or -1 if not there
    int insert ( int jname, char ctype, srword pos ); // return -1 if already inserted
 };

//============================= SkChannelArray ============================


//  A persistent empty SkChannelArray returned by empty_channel_array().
SkChannelArray SkChannelArray::EMPTY_CHANNELS = SkChannelArray();


SkChannelArray::SkChannelArray () {
    _floats=0;
    _htable=0;
}

SkChannelArray::~SkChannelArray () {
    if ( _htable ) { delete _htable; }
}

void SkChannelArray::init() {
	_channels.size ( 0 );
	_floats = 0;
	if ( _htable ) {
		delete _htable; _htable=0;
	}
}

void SkChannelArray::_add ( SkJoint* j, SkJointName name, SkChannel::Type t, bool connect ) {
	if ( name.undefined() && j )
		name=j->name();

	_channels.push().set ( j, t );
	_channels.top().name = name;
	_channels.top().fmap = 0;     // fmap is the "float map" only used in method map()
	_floats += SkChannel::size(t);
	if ( _htable ) {
		delete _htable;
		_htable=0;
	}
}

bool SkChannelArray::insert ( int pos, SkJointName name, SkChannel::Type type )
 {
   if ( pos<0 || pos>_channels.size() ) return false;

   // add channel:
   _channels.insert ( pos );
   _channels[pos].set ( 0, type );
   _channels[pos].name = name;
   _channels[pos].fmap = 0;
   _floats += SkChannel::size ( type );
   if ( _htable ) { delete _htable; _htable=0; }

   return true;
 }

SkSkeleton* SkChannelArray::skeleton () const
 {
   if ( _channels.size()==0 ) return 0;
   return _channels[0].joint->skeleton();
 }

void SkChannelArray::get_active_channels ( SkSkeleton* sk, bool connect ) {
	init();
	add_active_channels( sk, connect );
}

void SkChannelArray::add_active_channels ( SkSkeleton* sk, bool connect ) {
	int i;
	int jsize = sk->joints().size();
	SkJoint* joint;

	for ( i=0; i<jsize; i++ ) {
		joint = sk->joints()[i];


		// position channels:
		if ( !joint->pos()->frozen(0) )
			add(joint,SkChannel::XPos,connect);
		if ( !joint->pos()->frozen(1) )
			add(joint,SkChannel::YPos,connect);
		if ( !joint->pos()->frozen(2) )
			add(joint,SkChannel::ZPos,connect);

		// rotation channels:
		if ( !joint->quat()->active() ) 
			continue;
		switch ( joint->rot_type() ) {
			case SkJoint::TypeQuat:
				add(joint,SkChannel::Quat,connect);
				break;
			case SkJoint::TypeSwingTwist:
				add(joint,SkChannel::Swing);
				if ( !joint->st()->twist_frozen() )
					add(joint,SkChannel::Twist,connect);
				break;
			case SkJoint::TypeEuler:
				if ( !joint->euler()->frozen(0) )
					add(joint,SkChannel::XRot,connect);
				if ( !joint->euler()->frozen(1) )
					add(joint,SkChannel::YRot,connect);
				if ( !joint->euler()->frozen(2) )
					add(joint,SkChannel::ZRot,connect);
				break;
		}
	}

	compress ();
}

int SkChannelArray::count_floats ()
 {
   int i, csize = _channels.size();
   _floats = 0;
   for ( i=0; i<csize; i++ )
    { _floats += _channels[i].size();
    }
   return _floats;
 }

int SkChannelArray::float_position ( int c ) const
 {
   if ( c>=_channels.size() ) c=_channels.size()-1;
   int i, floats=0;
   for ( i=0; i<c; i++ )
    { floats += _channels[i].size();
    }
   return floats;
 }

void SkChannelArray::get_values ( float* fp )
 {
   int i, csize = _channels.size();
   for ( i=0; i<csize; i++ )
    { fp += _channels[i].get ( fp );
    }
 }
 
void SkChannelArray::set_values ( const float* fp )
 {
   int i, csize = _channels.size();
   for ( i=0; i<csize; i++ )
    { fp += _channels[i].set ( fp );
    }
 }

//void SkChannelArray::get_random_values ( float* fp ) const
// {
//   int i, csize = _channels.size();
//   for ( i=0; i<csize; i++ )
//    { fp += _channels[i].get_random ( fp );
//    }
// }

bool SkChannelArray::get_used_channels ( const SrArray<SkPosture*>& postures, SrArray<int>* indices )
 {
   init ();
   if ( postures.size()<1 ) return 0;

   // check the actual channels being used:
   int i, j;
   SrBuffer<float>& firstpost = postures[0]->values;
   int firstpostsize = firstpost.size();
   SrArray<int> locindices;
   SrArray<int>& index = (indices? *indices:locindices);

   // init as -1 meaning indices not used (or 1 if one channel):
   index.size ( firstpost.size() ); // index has size==floats, not channels
   if ( firstpost.size()==1 )
    index[0] = 1;
   else
    index.setall ( -1 );

   // mark with 1 the indices of active channels, ie, those with changing values:
   for ( i=1; i<postures.size(); i++ )
    { if ( postures[i]->values.size()!=firstpost.size() ) return false;
      for ( j=0; j<firstpostsize; j++ )
       { if ( postures[i]->values[j]!=firstpost[j] ) index[j]=1;
       }
    }

   // now get only the used channels:
   SkChannelArray& ch = *postures[0]->channels();
   int csize;
   int postsize=0;
   i=0; j=0;
   for ( i=0; i<ch.size(); i++ )
    { csize = ch[i].size();//ch[i].size(ch[i].type);

      if ( index[j]<0 ) // not used: advance j to next value and continue
       { j+=csize; continue; }
      else
       { while ( csize-->0 ) // save in index the new indices of this channel
          { index[j++] = postsize++; }
         add ( ch[i] ); // get the channel
       }
    }

   return true;
 }

int SkChannelArray::linear_search ( SkJointName name, SkChannel::Type type ) const
 {
   int i;
   int chs = _channels.size();

   for ( i=0; i<chs; i++ )
    { if ( _channels[i].type==type && SkChannelArray::name(i)==name )
       { return i;
       }
    }

   return -1;
 }

int SkChannelArray::search ( SkJointName name, SkChannel::Type type ) {
	if( _channels.size()==0 ) {  // rebuild_hash_table() will fail (attempt a modulo 0 operation) in this case
		return -1;               // Besides, there is no possible match
	}
	if ( !_htable ) {
		rebuild_hash_table();
	}
    return _htable->lookup ( name.id(), (char)type );
}

void SkChannelArray::rebuild_hash_table ()
 {
   if ( !_htable ) _htable = new HashTable;

   // first rebuild the "float mapping":
   int i, _floats=0;
   for ( i=0; i<_channels.size(); i++ )
    { _channels[i].fmap = _floats;
      _floats += _channels[i].size();
    }

   // now build the hash table:
   _htable->init ( _channels.size()*2 );
   for ( i=0; i<_channels.size(); i++ )
    { _htable->insert ( _channels[i].name.id(), (char)_channels[i].type, (srword)i );
      // duplicated entries will not be inserted, but they should not exist.
    }
    
   //sr_out<<"Channel Array size:"<<size();
   //sr_out<<" htable size:"<<_htable->table.size()<<" longest:"<<_htable->longest_entry()<<srnl;
 }

int SkChannelArray::connect ( SkSkeleton* s )
 {
   int i;
   int count = 0;
   int chsize = _channels.size();
   SkJoint* joint=0;

   if ( !s )
    { for ( i=0; i<chsize; i++ ) _channels[i].joint = 0;
      return 0;
    }

   for ( i=0; i<chsize; i++ )
    { joint = s->search_joint(_channels[i].name);
      _channels[i].joint = 0;
      if ( joint )
       { if ( SkChannel::valid(type(i),joint->rot_type()) )
          { _channels[i].joint = joint;
            count++;
          }
       }
    }

   return count;
 }

// Without using the hash table, with 53 channels, a test performed 1431 comparisons
// to accomplish the mapping.
// With a hash size of 53*2=106, the hash table had longest entry==2, and 111 elements,
// meaning only 111-106==5 colliding elements!
// Therefore the comparisons number should drop to (53-5)+2*5==58!
void SkChannelArray::map ( SkChannelArray& ca, SrBuffer<int>& m )
 {
   // we rebuild here (instead of in search()) to also have _floats recalculated:
   if ( !_htable ) rebuild_hash_table();
 
   m.size ( _floats );

   int c1, c2, cmap, csize, v, i=0;
   for ( c1=0; c1<size(); ++c1 ) // for each channel c1
    { 
      c2 = ca.search ( name(c1), type(c1) ); // search it in ca
	  if ( c2<0 )
       { cmap=-1; } // not there, mark as -1
      else
       { cmap=ca._channels[c2].fmap; } // found, get its float mapping

      csize = _channels[c1].size();
      for ( v=0; v<csize; ++v )
        m[i++] = cmap<0? cmap:cmap+v;
    }
 }

/* Old O(n^2) implementation:
void SkChannelArray::map ( const SkChannelArray& ca, SrBuffer<int>& m )
 {
   if ( !_htable ) rebuild_hash_table();
 
   m.size ( _floats );

   int c1, c2, mfloat, cmap, csize, v, i=0;
   for ( c1=0; c1<size(); c1++ ) // for each channel c1
    { 
      cmap = -1;
      mfloat = 0;
      for ( c2=0; c2<ca.size(); c2++ )
       { if ( name(c1)==ca.name(c2) && type(c1)==ca.type(c2) ) // a match
           { cmap=mfloat; break; }
         mfloat += ca.const_get(c2).size();
       }

      csize = _channels[c1].size();
      for ( v=0; v<csize; v++ )
        m[i++] = cmap<0? cmap:cmap+v;
    }
 }*/

void SkChannelArray::merge ( SkChannelArray& ca )
 {
   int c, id;
   for ( c=0; c<ca.size(); c++ ) // for each channel c in ca
    { id = search ( ca.name(c), ca.type(c) );
      if ( id<0 ) // missing channel found
       { add ( ca.name(c), ca.type(c) ); // add it
       }
    }
 }

void SkChannelArray::operator = ( const SkChannelArray& a )
 {
   _channels = a._channels;
   _floats = a._floats;
   if ( _htable ) { delete _htable; _htable=0; }
 }

bool SkChannelArray::operator == ( const SkChannelArray& a )
 {
   if ( size()!=a.size() ) return false;
   
   int i;
   for ( i=0; i<size(); i++ )
    { if ( _channels[i].type!=a._channels[i].type ||
           _channels[i].name!=a._channels[i].name ) return false;
    }
   return true;
 }

SrOutput& operator<< ( SrOutput& o, const SkChannelArray& ca )
 {
   int i;
   o << "channels " << ca.size() << srnl;
   for ( i=0; i<ca.size(); i++ )
    { o << (const char*)ca.name(i) << srspc
        << ca.const_get(i).type_name() << srnl;
    }
   return o;
 }
 
SrInput& operator>> ( SrInput& in, SkChannelArray& ca )
 {
   in.get_token(); // read "channels"

   int n = in.getn().atoi(); // number of channels
   if ( in.last_error()==SrInput::UnexpectedToken ) return in;

   ca.init ();
   ca._channels.capacity(256); // reserve 256 entries

   SrString name;
   while ( (n--) > 0 )
    { in.get_token ( name );
      in.get_token ();
      ca.add ( SkJointName(name), SkChannel::get_type(in.last_token()) );
    }         

   ca.compress();

   return in;
 }

//========================= SkChannelArray::HashTable ============================

static int hash ( int jname, char ctype, int size )
 {
   int h = jname + (int)ctype;
   h = SR_ABS(h);
   h = h%size;
   return h;
 }

void SkChannelArray::HashTable::init ( int hsize )
 {
   hash_size = hsize;
   table.capacity ( hsize );
   table.size ( hsize );
   int i;
   for ( i=0; i<hsize; i++ )
    table[i].set ( -1/*joint*/, -1/*channel*/, 0/*pos*/, -1/*next*/ );
 }

int SkChannelArray::HashTable::longest_entry () const
 {
   int i, j, len, longest=0;
   for ( i=0; i<hash_size; i++ )
    { if ( table[i].jname_key<0 ) continue; // empty
      len = 1;
      j = table[i].next;
      while ( j>=0 ) { len++; j=table[j].next; }
      if ( len>longest ) longest=len;
    }
   return longest;
 }

int SkChannelArray::HashTable::lookup ( int jname, char ctype ) const
 {
   int id = ::hash ( jname, ctype, hash_size );

   if ( table[id].jname_key<0 ) return -1; // empty entry, not found
   while ( true )
    { if ( table[id].jname_key==jname && table[id].ctype_key==ctype ) // found
       return (int)table[id].pos_data;
      // else check next colliding entry:
      if ( table[id].next<0 ) return -1; // no more entries, not found
      id = table[id].next;
    }
 }

int SkChannelArray::HashTable::insert ( int jname, char ctype, srword pos )
 {
   int id = ::hash ( jname, ctype, hash_size );

   if ( table[id].jname_key<0 ) // empty entry, just take it
    { table[id].set ( jname, ctype, pos, -1/*next*/ );
      return id;
    }

   while ( true )
    { if ( table[id].jname_key==jname && table[id].ctype_key==ctype ) return -1; // already there

      // else check next colliding entry:
      if ( table[id].next<0 ) // no more entries, add one:
       { table[id].next = table.size();
         table.push().set ( jname, ctype, pos, -1/*next*/ );
         return table.size()-1;
       }
      
      id = table[id].next;
    }
 }

//============================ End of File ============================

