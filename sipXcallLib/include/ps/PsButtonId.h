//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PsButtonId_h_
#define _PsButtonId_h_

enum PsButtonId
{
   DIAL_0 = '0',
   DIAL_1 = '1',
   DIAL_2 = '2',
   DIAL_3 = '3',
   DIAL_4 = '4',
   DIAL_5 = '5',
   DIAL_6 = '6',
   DIAL_7 = '7',
   DIAL_8 = '8',
   DIAL_9 = '9',
   DIAL_POUND = '#',
   DIAL_STAR  = '*',

   FKEY_ACCEPT = 0x100,         // function keys
   FKEY_CANCEL,
   FKEY_CONFERENCE,
   FKEY_DO_NOT_DISTURB,
   FKEY_FORWARD,
   FKEY_HEADSET,
   FKEY_HOLD,
   FKEY_MENU,
   FKEY_MORE,
   FKEY_MUTE,
   FKEY_PARK,
   FKEY_PICKUP,
   FKEY_PRIVACY,
   FKEY_REDIAL,
   FKEY_SCROLL_UP,
   FKEY_SCROLL_DOWN,
   FKEY_SPEAKER,
   FKEY_SPEED_DIAL,
   FKEY_TRANSFER,
   FKEY_VOICE_MAIL,
   FKEY_VOL_UP,
   FKEY_VOL_DOWN,
   FKEY_ESC,
   FKEY_BACKSPACE,
   FKEY_TAB,
   FKEY_HOME,
   FKEY_END,
// FKEY_OTHER ...

   FKEY_USER1 = 0x140,          // user assignable keys
   FKEY_USER2,
   FKEY_USER3,
   FKEY_USER4,
   FKEY_USER5,
   FKEY_USER6,
   FKEY_USER7,
   FKEY_USER8,
   FKEY_USER9,
   FKEY_USER10,
   FKEY_USER11,
   FKEY_USER12,
   FKEY_USER13,
   FKEY_USER14,
   FKEY_USER15,
   FKEY_USER16,
   FKEY_USER17,
   FKEY_USER18,
   FKEY_USER19,
   FKEY_USER20,
// FKEY_USER...

   FKEY_CALL1 = 0x180,          // call<n>
   FKEY_CALL2,
   FKEY_CALL3,
   FKEY_CALL4,
   FKEY_CALL5,
   FKEY_CALL6,
   FKEY_CALL7,
   FKEY_CALL8,
   FKEY_CALL9,
   FKEY_CALL10,
// FKEY_CALL...

   FKEY_SKEY1 = 0x200,          // generic soft key<n>
   FKEY_SKEY2,
   FKEY_SKEY3,
   FKEY_SKEY4,
   FKEY_SKEY5,
   FKEY_SKEY6,
   FKEY_SKEY7,
   FKEY_SKEY8,
   FKEY_SKEY9,
   FKEY_SKEY10,
   FKEY_SKEY11,
   FKEY_SKEY12,
   FKEY_SKEY13,
   FKEY_SKEY14,
   FKEY_SKEY15,
   FKEY_SKEY16,
   FKEY_SKEY17,
   FKEY_SKEY18,
   FKEY_SKEY19,
   FKEY_SKEY20,
// FKEY_SKEY...

   FKEY_SKEY_L1 = 0x240,        // soft key<n> to the left of display
   FKEY_SKEY_L2,
   FKEY_SKEY_L3,
   FKEY_SKEY_L4,
// FKEY_SKEY_L...

   FKEY_SKEY_R1 = 0x280,        // soft key<n> to the right of display
   FKEY_SKEY_R2,
   FKEY_SKEY_R3,
   FKEY_SKEY_R4,
// FKEY_SKEY_R...

   FKEY_SKEY_B1 = 0x2c0,        // soft key<n> below the display
   FKEY_SKEY_B2,
   FKEY_SKEY_B3,
// FKEY_SKEY_B...

};
#endif  // _PsButtonId_h_
