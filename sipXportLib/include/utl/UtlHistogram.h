//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _UtlHistogram_h_
#define _UtlHistogram_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Record and dump counts in a series of bins.
 *
 * Recorded values are integers.  The histogram has a specified number
 * of bins, each counting the number of values over a range, each of
 * which has the same specified width.
 *
 * The number of bins implemented is 2 more than the specified number,
 * to allow one bin for values lower than the range and one for values
 * higher than the range.
 *
 * The minimum of the range of bin 0 is Base, and each normal bin has
 * a range of Size values.
 *
 * The first normal bin is numbered 0, so the bin for too-low values
 * is numbered -1.  Thus, the bins record:
 *
 * Bin -1 counts values less than "Base".
 * Bin 0 counts values from "Base" to "Base + Size - 1".
 * Bin 1 counts values from "Base + Size" to "Base + 2*Size - 1".
 * Bin n counts values from "Base + n*Size" to "Base + (n+1)*Size - 1".
 * Bin NoOfBins-1 counts values from "Base + (NoOfBins-1)*Size" to
 *         "Base + NoOfBins*Size -1".
 * Bin NoOfBins counts values greater than or equal to "Base + NoOfBins*Size".
 *
 * The values of all the bins can be extracted in a string by setting
 * outputFormat and outputWidth when the histogram is created.
 * Calling show() formats each bin's value via
 * sprintf(buffer, outputFomat, bin-value), concatenates them
 * together, and returns the result as a UtlString.
 * outputFormat must be a format string for formatting a single int,
 * and it must always generate at most outputWidth characters.
 *
 * The outputFormat string must remain valid as long as the histogram
 * object exists; the returned UtlString must be freed by the caller.
 */
class UtlHistogram
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   UtlHistogram(unsigned int bins, int base, unsigned int size,
                const char* outputFormat = "", unsigned int outputWidth = 0);

   /**
    * Destructor
    */
   ~UtlHistogram();

/* ============================ MANIPULATORS ============================== */

   /**
    * Record a value.
    * Returns the number of counts in the histogram.
    */
   unsigned int tally(int);

   /**
    * Clear the histogram.
    */
   void clear();

/* ============================ ACCESSORS ================================= */

   /**
    * Get the specified number of bins (which is 2 less than the total number
    * of bins).
    */
   unsigned int getNoOfBins();

   /**
    * Get the lowest value for bin 0.
    */
   unsigned int getBase();

   /**
    * Get the size of each bin.
    */
   unsigned int getBinSize();

   /**
    * Get the total count.
    */
   unsigned int getCount();

   /**
    * Get the count in bin i.
    * i ranges from -1 to NoOfBins+1.
    */
   unsigned int operator[](unsigned int i);

/* ============================ INQUIRY =================================== */

   /**
    * Get a string containing the formatted values from the bins.
    * The caller must free the returned UtlString.
    */
   UtlString* show();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   // Number of bins.
   unsigned int mNoOfBins;
   // Lowest value in bin 0.
   int mBase;
   // Size of each bin:
   unsigned int mBinSize;
   // Pointer to an array of mNoOfBins int's.
   unsigned int* mpBins;
   // Total number of counts.
   unsigned int mCount;

   // Format used to output values.
   const char* mOutputFormat;
   // Width of output that format will generate.
   unsigned int mOutputWidth;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlHistogram_h_
