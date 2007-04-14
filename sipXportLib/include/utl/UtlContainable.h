//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UtlContainable_h_
#define _UtlContainable_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * An UtlContainable object is an abstract object that serves as the base 
 * class for anything that can be contained in one of the UtlContainer- 
 * derived classes.  One of the largest values of a UtlContainable-derived 
 * object is the ability for any UtlContainer to destroy objects, sort 
 * objects, etc.
 */
class UtlContainable 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Destructor
     */
    virtual ~UtlContainable();

    /**
     * Get the ContainableType for a UtlContainable-derived class.
     */
    virtual UtlContainableType getContainableType() const = 0 ;

    static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */

    /// Calculate a hash code for this object.
    virtual unsigned hash() const = 0 ;
    /**<
     * The hash method should return a value that is a function of the key used to
     * locate the object.  As much as possible, hash values should be uniformly
     * distributed over the range of legal unsigned values.
     * 
     * \par Requirements
     * if A.isEqual(B) then A.hash() must == B.hash()
     */

    /// Provides a hash function that uses the object pointer as the hash value.
    unsigned directHash() const;
    /**<
     * This may be used by any UtlContainable class for which generating a value hash
     * is difficult or not meaningful.  Note that pointer values, since they are not
     * uniformly distributed, probably make poor hash codes so this should not be used
     * normally.  @see stringHash
     *
     * To use this, define your hash function as just:<pre>
     * unsigned int Foo::hash() const
     * {
     *    return directHash();
     * }
     * </pre>
     *
     * If you use directHash as the hash method, you probably want to
     * use pointer comparison as the compareTo method:<pre>
     * int Foo::compareTo(UtlContainable const* inVal) const
     * {
     *    int result;
     * 
     *    result =
     *       this > other ? 1 :
     *       this < other ? -1 :
     *       0;
     *
     *    return result;
     * }
     * </pre>
     */
    
    /// Provides a hash function appropriate for null-terminated string values.
    static unsigned stringHash(char const* value);
    /**<
     * To use this, define your hash function as just:<pre>
     * Foo hash()
     * {
     *    return stringHash(value);
     * }
     * </pre>
     */

    /// Compare this object to another object. 
    virtual int compareTo(UtlContainable const *) const = 0  ;    
    /**<
     * Results of comparison to an object not of the same UtlContainableType
     * may be undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     *
     * \par Requirements
     * - if A.compareTo(B) == 0 then B.compareTo(A) must be == 0
     * - if A.compareTo(B) < 0 then B.compareTo(A) must be > 0
     * - if A.compareTo(B) > 0 then B.compareTo(A) must be < 0
     * - if A.compareTo(B) < 0 and B.compareTo(C) < 0 then A.compareTo(C) must be < 0
     * etc.
     * 
     * Results for comparing with a non-like object are undefined,
     * other than that it must return non-equal.
     * Note that a copy of an object might not compare equal to the
     * original, unless the definition of its type requires it.
     */

    /// Test this object to another object for equality.
    virtual UtlBoolean isEqual(UtlContainable const *) const;
    /**<
     * Results for objects not of the same UtlContainableType may be undefined.
     *
     * A default implementation of this is provided that should be adequate for any
     * UtlContainableType.
     */

    /// Determine if this object is of the specified UtlContainableType.
    virtual UtlBoolean isInstanceOf(const UtlContainableType type) const ; 
    /**<
     * Determine if this object is an instance of the designated runtime
     * class identifer.  For example:
     * <pre>
     * if (pMyObject->isInstanceOf(UtlInt::TYPE))
     * {
     *     ...
     * }
     * </pre>
     */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlContainable_h_
