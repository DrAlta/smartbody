
# ifndef SR_RANDOM_H
# define SR_RANDOM_H

#include <sb/SBTypes.h>
/** \file sr_random.h 
 * Random number generation */

//================================= SrRandom ====================================

/*! Random methods use a custom random generator with resolution of 2^32-1 */
class SrRandom
 { private:
    double _inf, _sup, _dif;
    char _type; // 'd', 'f', or 'i'

   public :

    /*! Default constructor sets the random generator to have
       limits [0,1] and type float */
    SBAPI SrRandom ();

    /*! Constructor with given limits in type double, with 53-bit resolution */
    SBAPI SrRandom ( double inf, double sup ) { limits(inf,sup); }
 
    /*! Constructor with given limits in type float, with 32-bit resolution */
    SBAPI SrRandom ( float inf, float sup ) { limits(inf,sup); }

    /*! Constructor with given integer limits. */
    SBAPI SrRandom ( int inf, int sup ) { limits(inf,sup); }

    /*! Sets limits in type double, with 53-bit resolution */
    SBAPI void limits ( double inf, double sup );

    /*! Sets limits in type float, with 32-bit resolution. */
    SBAPI void limits ( float inf, float sup, int br=15 );

    /*! Sets integer limits.. */
    SBAPI void limits ( int inf, int sup );

    /*! Returns the type on which the limits were set: 'd', 'f', or 'i' */
    SBAPI char type () { return _type; }

    /*! Returns the superior limit. */
    SBAPI double inf () { return _inf; }

    /*! Returns the inferior limit. */
    SBAPI double sup () { return _sup; }

    /*! Returns a random integer number in [inf,sup]. */
    SBAPI int geti ();
    
    /*! Returns a random float number in [inf,sup], with 32-bit resolution. */
    SBAPI float getf ();

    /*! Returns a random double number in [inf,sup], with 53-bit resolution. */
    SBAPI double getd ();

    /*! Set limits and returns one corresponding random value. */
    SBAPI int get ( int inf, int sup ) { limits(inf,sup); return geti(); }

    /*! Set limits and returns one corresponding random value, with 32-bit resolution. */
    SBAPI float get ( float inf, float sup ) { limits(inf,sup); return getf(); }

    /*! Set limits and returns one corresponding random value, with 53-bit resolution. */
    SBAPI double get ( double inf, double sup ) { limits(inf,sup); return getd(); }

    /*! Sets the starting point for generating random numbers. The seed is 1 by default. */
    SBAPI static void seed ( unsigned long seed );
    
    /*! Returns a random float in [0,1], with 32-bit resolution */
    SBAPI static float randf ();
 };

//============================== end of file ======================================

# endif  // SR_RANDOM_H

