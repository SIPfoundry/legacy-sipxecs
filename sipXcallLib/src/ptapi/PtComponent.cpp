//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "ptapi/PtComponentGroup.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

PtComponent::PtComponent()
: mGroupType(PtComponentGroup::OTHER)
{
        mType = UNKNOWN;

        memset(mpName, 0, 21);
        strcpy(mpName, "unknown");
}

// Constructor
PtComponent::PtComponent(int componentType)
: mGroupType(PtComponentGroup::OTHER)
{
        mType = componentType;

        memset(mpName, 0, 21);
        switch (mType)
        {
        case BUTTON:
                strcpy(mpName, "button");
                break;
        case DISPLAY:
                strcpy(mpName, "display");
                break;
        case GRAPHIC_DISPLAY:
                strcpy(mpName, "graphic_display");
                break;
        case HOOKSWITCH:
                strcpy(mpName, "hookswitch");
                break;
        case LAMP:
                strcpy(mpName, "lamp");
                break;
        case MICROPHONE:
                strcpy(mpName, "microphone");
                break;
        case RINGER:
                strcpy(mpName, "ringer");
                break;
        case SPEAKER:
                strcpy(mpName, "speaker");
                break;
        case EXTERNAL_SPEAKER:
                strcpy(mpName, "external_speaker");
                break;
        case TEXT_DISPLAY:
                strcpy(mpName, "text_display");
                break;
        default:
        case UNKNOWN:
                strcpy(mpName, "unknown");
                break;
        }

}

PtComponent::PtComponent(const char*& rName)
: mGroupType(PtComponentGroup::OTHER)
{
   if (rName)
   {
           int len = strlen(rName);

                int i = 0;
                while(i < len)
                {
                        mpName[i] = tolower(rName[i]);
                        i++;
                }
                mpName[len] = 0;

                if(strcmp(mpName, "button"))
                        mType = BUTTON;
                else if(strcmp(mpName, "display"))
                        mType = DISPLAY;
                else if(strcmp(mpName, "graphic_display"))
                        mType = GRAPHIC_DISPLAY;
                else if(strcmp(mpName, "hookswitch"))
                        mType = HOOKSWITCH;
                else if(strcmp(mpName, "lamp"))
                        mType = LAMP;
                else if(strcmp(mpName, "microphone"))
                        mType = MICROPHONE;
                else if(strcmp(mpName, "ringer"))
                        mType = RINGER;
                else if(strcmp(mpName, "speaker"))
                        mType = SPEAKER;
                else if(strcmp(mpName, "external_speaker"))
                        mType = EXTERNAL_SPEAKER;
                else if(strcmp(mpName, "text_display"))
                        mType = TEXT_DISPLAY;
                else
                        mType = UNKNOWN;
        }
        else
        {
                strcpy(mpName, "unknown");
                mType = UNKNOWN;
        }

}

// Copy constructor
PtComponent::PtComponent(const PtComponent& rPtComponent)
{
        mType = rPtComponent.mType;
        mGroupType = rPtComponent.mGroupType;
        if (rPtComponent.mpName)
        {
                // int len = strlen(rPtComponent.mpName);
                strcpy(mpName, rPtComponent.mpName);
        }
        else
        {
                strcpy(mpName, "unknown");
        }

}

// Destructor
PtComponent::~PtComponent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtComponent&
PtComponent::operator=(const PtComponent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mType = rhs.mType;
        mGroupType = rhs.mGroupType;

        if (rhs.mpName)
        {
                // int len = strlen(rhs.mpName);

                strcpy(mpName, rhs.mpName);
        }
        else
        {
                strcpy(mpName, "unknown");
        }

   return *this;
}

/* ============================ ACCESSORS ================================= */
PtStatus PtComponent::getName(char* rpName, int maxLen)
{
        if (rpName && maxLen > 0)
        {
                if (mpName)
                {
                        int bytes = strlen(mpName);
                        bytes = (bytes > maxLen) ? maxLen : bytes;

                        memset(rpName, 0, maxLen);
                        strncpy (rpName, mpName, bytes);
                        return PT_SUCCESS;
                }
        }

        return PT_RESOURCE_UNAVAILABLE;
}

PtStatus PtComponent::getType(int& rType)
{
        rType = mType;
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
void PtComponent::setGroupType(int groupType)
{
        mGroupType = groupType;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
