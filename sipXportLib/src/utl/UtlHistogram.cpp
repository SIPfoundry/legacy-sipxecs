//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlHistogram.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlHistogram::UtlHistogram(unsigned int bins, int base, unsigned int size,
                           const char* outputFormat,
                           unsigned int outputWidth)
{
   if (bins == 0)
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "UtlHistogram::UtlHistogram bins must be at least 1");
      bins = 1;
   }
   mNoOfBins = bins;

   mBase = base;

   if (size == 0)
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "UtlHistogram::UtlHistogram size must be at least 1");
      size = 1;
   }
   mBinSize = size;

   mpBins = new unsigned int[mNoOfBins + 2];
   mCount = 0;

   mOutputFormat = outputFormat;
   mOutputWidth = outputWidth;
}

// Destructor
UtlHistogram::~UtlHistogram()
{
   delete[] mpBins;
}


/* ============================ MANIPULATORS ============================== */

unsigned int UtlHistogram::tally(int value)
{
   // Calculate the bin number.
   unsigned int bin = (value - mBase) / mBinSize;
   // Increment the appropriate bin.
   ++mpBins[bin < 0 ? 0 :
            bin > mNoOfBins ? mNoOfBins + 1 :
            bin + 1];
   // Increment the total count.
   mCount++;

   return mCount;
}

void UtlHistogram::clear()
{
   // Clear the bins.
   // Remember that there are mNoOfBins + 2 bins.
   for (unsigned int i = 0; i < mNoOfBins + 2; i++)
   {
      mpBins[i] = 0;
   }
   // Clear the total count.
   mCount = 0;
}

/* ============================ ACCESSORS ================================= */


/**
 * Get the number of bins.
 */
unsigned int UtlHistogram::getNoOfBins()
{
   return mNoOfBins;
}

/**
 * Get the lowest value for bin 0.
 */
unsigned int UtlHistogram::getBase()
{
   return mBase;
}

/**
 * Get the size of each bin.
 */
unsigned int UtlHistogram::getBinSize()
{
   return mBinSize;
}

/**
 * Get the total count.
 */
unsigned int UtlHistogram::getCount()
{
   return mCount;
}

/**
 * Get the count in bin i.
 */
unsigned int UtlHistogram::operator[](unsigned int i)
{
   // Normalize i to index mpBins[].
   i++;
   return
      i >= 0 && i < mNoOfBins+2 ?
      mpBins[i] :
      0;
}

/* ============================ INQUIRY =================================== */

/**
 * Get a string containing the formatted values from the bins.
 */
UtlString* UtlHistogram::show()
{
   unsigned int width;

   // Buffer into which to sprintf the values.
   char* buffer = new char[(mNoOfBins+2) * mOutputWidth + 1];

   // Translate the values.
   for (unsigned int i = 0, j = 0; i < mNoOfBins + 2; i++, j += width)
   {
      width = sprintf(&buffer[j], mOutputFormat, mpBins[i]);
      if (width > mOutputWidth)
      {
         OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                       "UtlHistogram::show output from format '%s' "
                       "had width %d != declared width %d",
                       mOutputFormat, width, mOutputWidth);
      }
   }
   // Since there is always at least one bin, sprintf was called at least
   // once and it ended the string with a NUL.

   // Construct the UtlString.
   UtlString* value = new UtlString(buffer);
   delete[] buffer;

   return value;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
