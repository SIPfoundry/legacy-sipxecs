//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _LinePresenceBase_h_
#define _LinePresenceBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;

/**
 * The LinePresenceBase class is a pure virtual class.
 * Objects of subclasses can use this interface to receive notifications
 * of changes of status of a URI.
 * The subject URI is contained in the object, and can be retrieved with
 * getUri().
 * This interface can also be used to retrieve status information
 * from the object.
 *
 * Status consists of three boolean values, "present", "on-hook", and
 * "signed-in".
 */
class LinePresenceBase {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/**
 * Line Presence state type enumerations.  These are used collectively to
 * represent the possible states of PRESENT, (NOT)PRESENT, ON_HOOK
 * and (NOT)ON_HOOK.
 */
   enum ePresenceStateType {
      PRESENT     = 1,     /**< present state */
      ON_HOOK     = 2,     /**< on/off hook state */
      SIGNED_IN   = 4      /**< ACD SIGNED-IN state */
   };


/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   LinePresenceBase(void) {}

   /**
    * Destructor
    */
   virtual ~LinePresenceBase() {}

/* ============================ MANIPULATORS ============================== */
   /**
    * Update the presence / line state for this line.
    *
    * @param type The type of presence state to be updated.
    *
    * @param state The state value to be updated.
    */
   virtual void updateState(ePresenceStateType type, bool state) = 0;


/* ============================ ACCESSORS ================================= */
   /**
    * Return the AOR for this line.
    */
   virtual Url* getUri(void) = 0;

   /**
    * Return the presence / line state information for this line.
    */
   virtual bool getState(ePresenceStateType type) = 0;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif  // _LinePresenceBase_h_
