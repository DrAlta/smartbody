/*  sr_polygons.h - part of Motion Engine and SmartBody-lib
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
 *  License along with SmarBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently UC Merced)
 */

# ifndef SR_POLYGONS_H
# define SR_POLYGONS_H

/** \file sr_polygons.h
 * maintains an array of polygons
 */
#include <sb/SBTypes.h>
# include <sr/sr_array.h>
# include <sr/sr_polygon.h>
# include <sr/sr_shared_class.h>

/*! \class SrPolygons sr_polygons.h
    \brief maintains an array of polygons

    SrPolygons keeps internally an array of polygons and provides
    methods for manipulating them. */   
class SrPolygons : public SrSharedClass
 { private :
    SrArray<SrPolygon*> _data;

   public :
    static const char* class_name;

    /*! Default constructor */
    SBAPI SrPolygons ();

    /*! Copy constructor */
    SBAPI SrPolygons ( const SrPolygons& p );

    /*! Virtual Destructor */
    SBAPI virtual ~SrPolygons ();

    /*! Returns true if the array has no polygons, and false otherwise. */
    SBAPI bool empty () const { return _data.empty(); }

    /*! Returns the capacity of the array. */
    SBAPI int capacity () const { return _data.capacity(); }

    /*! Returns the current size of the array. */
    SBAPI int size () const { return _data.size(); }

    /*! Changes the size of the array, filling new empty polygons in the new positions. */
    SBAPI void size ( int ns );

    /*! Makes the array empty; equivalent to size(0) */
    SBAPI void init () { size(0); }

    /*! Changes the capacity of the array. */
    SBAPI void capacity ( int nc );

    /*! Makes capacity to be equal to size. */
    SBAPI void compress () { _data.compress(); }

    /*! Swaps polygons with positions i and j, which must be valid positions. */
    SBAPI void swap ( int i, int j );

    /*! Returns a reference to polygon index i, which must be a valid index */
    SBAPI SrPolygon& get ( int i ) { return *_data[i]; }

    /*! Returns a const reference to polygon index i, which must be a valid index */
    SBAPI const SrPolygon& const_get ( int i ) const { return *_data[i]; }

    /*! Returns a const reference to the vertex j of polygon i. Indices must be valid. */
    SBAPI const SrVec2& const_get ( int i, int j ) const { return _data[i]->const_get(j); }

    /*! Returns a reference to the vertex j of polygon i. Indices must be valid. */
    SBAPI SrVec2& get ( int i, int j ) { return _data[i]->get(j); }

    /*! Equivalent to get(i) */
    SBAPI SrPolygon& operator[] ( int i ) { return get(i); }

    /*! Equivalent to get(i,j) */
    SBAPI SrVec2& operator() ( int i, int j ) { return get(i,j); }

    /*! Copy polygon p into position i */
    SBAPI void set ( int i, const SrPolygon& p ) { get(i)=p; }

    /*! Returns the last polygon */
    SBAPI SrPolygon& top () { return *_data.top(); }

    /*! Pop and frees last polygon if the array is not empty */
    SBAPI void pop () { delete _data.pop(); }

    /*! Allocates and appends one empty polygon */
    SBAPI SrPolygon& push () { return *(_data.push()=new SrPolygon); }

    /*! Inserts one polygon using copy operator */
    SBAPI void insert ( int i, const SrPolygon& x );

    /*! Allocates and inserts n empty polygons at position i */
    SBAPI void insert ( int i, int n );

    /*! Removes n polygons at position i */
    SBAPI void remove ( int i, int n=1 );

    /*! Extract (without deletion) and returns the pointer at position i */
    SBAPI SrPolygon* extract ( int i );

    /*! Returns true if there is a vertex closer to p than epsilon. In this case
        the indices of the closest vertex to p are returned in pid and vid.
        If no vertices exist, false is returned. */
    SBAPI bool pick_vertex ( const SrVec2& p, float epsilon, int& pid, int& vid ) const;

    /*! Returns the index of the first polygon containing p, or -1 if not found */
    SBAPI int pick_polygon ( const SrVec2& p ) const;

    /*! Returns true if there is an edge closer to p than epsilon. In this case
        the indices of the first vertex of the closest edge are returned in pid and vid.
        If no edge exist, false is returned. */
    SBAPI bool pick_edge ( const SrVec2& p, float epsilon, int& pid, int& vid ) const;

    /*! Returns the first polygon intersecting the given segment, or -1 otherwise. */
    SBAPI int intersects ( const SrVec2& p1, const SrVec2& p2 ) const;

    /*! Returns the first polygon intersecting p, or -1 otherwise. */
    SBAPI int intersects ( const SrPolygon& p ) const;

    /*! Returns the bounding box of the set of polygons */
    SBAPI void get_bounding_box ( SrBox &b ) const;

    /*! Copy operator */
    SBAPI void operator = ( const SrPolygons& p );

    /*! Outputs all elements of the array. */
    SBAPI friend SrOutput& operator<< ( SrOutput& o, const SrPolygons& p );

    /*! Inputs elements. */
    SBAPI friend SrInput& operator>> ( SrInput& in, SrPolygons& p );
 };

//================================ End of File =================================================

# endif // SR_SCENE_POLYGONS_H
