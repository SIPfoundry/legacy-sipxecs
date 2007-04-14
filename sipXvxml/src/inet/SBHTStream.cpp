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

/* Library include files */
#include "wwwsys.h"
#include "WWWUtil.h"
#include "WWWCore.h"

#include "SBHTStream.h"					 /* Implemented here */

/* Override definition of Libwww PUBLIC function prefix, Libwww
   definition is empty, for the code below we need extern "C" since this
   is a C++ source file */
#ifdef PUBLIC
#undef PUBLIC
#endif
#define PUBLIC extern "C"

#define HT_MAXSIZE	0x10000		      /* Max size of allocated chunk */
#define HT_MAXGROWSIZE	0x4000		 /* Increment buffer by no more than */

struct _HTStream {
    HTStreamClass *	isa;
    HTRequest *		request;
    HTChunk *		chunk;

    int			cur_size;
    int			max_size;
    BOOL		give_up;
    BOOL		ignore;
    BOOL		ensure;
};

/* ------------------------------------------------------------------------- */

PRIVATE int SBHT_flush (HTStream * me)
{
    return HT_OK;			/* Nothing to flush */
}

/*
**	We don't free the chunk as this is for the caller to do
*/
PRIVATE int SBHT_free (HTStream * me)
{
  //    HTTRACE(STREAM_TRACE, "Chunkstream. FREEING...\n");
    HT_FREE(me);
    return HT_OK;
}

/*
**	We don't free the chunk as this is for the caller to do
*/
PRIVATE int SBHT_abort (HTStream * me, HTList * errorlist)
{
  //    HTTRACE(STREAM_TRACE, "Chunkstream. ABORTING...\n");
    HT_FREE(me);
    return HT_ERROR;
}

PRIVATE int SBHT_putBlock (HTStream * me, const char * b, int l)
{
    me->cur_size += l;

    /*
    ** If we get a buffer overflow and we are going to PUT or POST the document
    ** then ask the user whether it is OK to proceed buffering. Otherwise we
    ** must give up the request. In all other cases we stop if the buffer fills
    ** up.
    */
    if (!me->ignore && me->max_size > 0 && me->cur_size > me->max_size) {
      me->give_up = YES;
    } else if (!me->ensure) {
	HTParentAnchor * anchor = HTRequest_anchor(me->request);
	int cl = HTAnchor_length(anchor);
	if (cl > 0) HTChunk_ensure(me->chunk, cl);
	me->ensure = YES;
    }
    if (!me->give_up) {
	HTChunk_putb(me->chunk, b, l);
	return HT_OK;
    }    
    /* Note: If we ever return error from here, bad things happen in libwww, esp the cache.
     */
    return HT_ERROR;
}

PRIVATE int SBHT_putCharacter (HTStream * me, char c)
{
    return SBHT_putBlock(me, &c, 1);
}

PRIVATE int SBHT_putString (HTStream * me, const char * s)
{
    return SBHT_putBlock(me, s, (int) strlen(s));
}

HTStreamClass SBHTStreamClass = {
    (char*)"StreamToChunk",
    SBHT_flush,
    SBHT_free,
    SBHT_abort,
    SBHT_putCharacter,
    SBHT_putString,
    SBHT_putBlock
};

PUBLIC HTStream * SBHTStream (HTRequest * 	request,
			      HTChunk **	chunk,
			      int 		max_size)
{
    if (request) {
	HTStream * me;
	*chunk = NULL;
	if ((me = (HTStream  *) HT_CALLOC(1, sizeof(HTStream))) == NULL)
	    HT_OUTOFMEM((char*)"HTStreamToChunk");
	me->isa = &SBHTStreamClass;
	me->request = request;
	/* There is a bug here in the original HTSChunk.c  !max_size instead of max_size - dg */
	me->max_size = (max_size) ? max_size : HT_MAXSIZE;
	/* Use chunk for now, until we get blocking working */
	me->chunk = *chunk = HTChunk_new(me->max_size > 0 ?
					 HTMIN(me->max_size, HT_MAXGROWSIZE) :
					 HT_MAXGROWSIZE);
	/*
	HTTRACE(STREAM_TRACE, "ChunkStream. Chunk %p created with max size %d\n" _ 
		    me->chunk _ me->max_size);
	*/
	return me;
    }
    return HTErrorStream();
}

