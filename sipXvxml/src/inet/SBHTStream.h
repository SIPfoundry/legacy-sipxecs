/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * Enhanced W3C Sample Code Library libwww Stream to Chunk Converter
 *
 * Libwww Stream for SBinet. Based on HTSChunk.c in libwww. For now we
 * still write to chunks, eventually we will buffer here. The idea is
 * to make a blocking stream but the libwww event model does not
 * handle this well.
 *
 * Original W3C Libwww notes:
 *
 * This stream converts a Stream obejct into a Chunk object. Chunks
 * are dynamic streams so this is in other words a conversion from a
 * stream based model to a dynamic data buffer model for handling a
 * downloaded object. It is for the caller of this stream to free the
 * chunk.
 *
 * If max_size is 0 then we use a default size, if -1 then there is no
 * limit.
 *
 * This module is implemented by HTSChunk.c, and it is a part of the
 * W3C Sample Code Library.
 *
 *****************************************************************************
 ****************************************************************************/


/****************License************************************************
 *
 * (c) COPYRIGHT MIT 1995.
 * Please first read the full copyright statement in the file COPYRIGH.
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
 */

#ifndef SBHTSTREAM_H
#define SBHTSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HTChunk.h"
#include "HTStream.h"

extern HTStream * SBHTStream (HTRequest * 	request,
			      HTChunk **	chunk,
			      int 		max_size);

#ifdef __cplusplus
}
#endif

/*

End of definition module
*/

#endif /* SBHTSTREAM_H */
