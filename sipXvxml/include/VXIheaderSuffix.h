/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************
 *
 * 
 *
 * Settings which should end all public headers
 *
 ************************************************************************
 */

/* Reset the structure packing conventions */

#if defined(_MSC_VER)            /* Microsoft Visual C++ */
  #pragma pack(pop)
#elif defined(__BORLANDC__)      /* Borland C++ */
  #pragma option -a.
#elif defined(__WATCOMC__)       /* Watcom C++ */
  #pragma pack(pop)
#endif
