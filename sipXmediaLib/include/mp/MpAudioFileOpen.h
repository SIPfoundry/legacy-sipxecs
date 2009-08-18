//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef MP_AUDIO_FILE_OPEN_H
#define MP_AUDIO_FILE_OPEN_H

#include "mp/MpAudioAbstract.h"
#include <os/istream>

MpAudioAbstract *MpOpenFormat(istream &file);

#endif
