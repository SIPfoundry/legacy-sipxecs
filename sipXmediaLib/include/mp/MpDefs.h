//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpDefs_h_
#define _MpDefs_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#ifdef __cplusplus
extern "C" {
#endif

// DEFINES

// MACROS

// CONSTANTS

/* //////////////////////////// TYPEDEFS ////////////////////////////////// */


// FORWARD DECLARATIONS
// Forward definitions for "circular" references:
typedef struct __MpBuf_tag       *MpBufPtr;
typedef struct __MpBufPool_tag   *MpBufPoolPtr;

// Buffer formats:
enum mpBufFormat {
        mpBufFUnknown = 0,
        mpBufFT12 = 1,        /* Raw UCB1200 data, high 12 bits of 16 */
        mpBufFRtpPkt = 2,     /* Raw RTP packet */
        mpBufFRtcpPkt = 3,    /* Raw RTCP packet */
        mpBufFG711M = 4,      /* G711 mu, mu-law */
        mpBufFG711A = 5,      /* G711 A, A-law */
        mpBufFrL16 = 6,       /* Linear 16, in "network byte order" */
};
typedef enum mpBufFormat mpBufFormat;


// STRUCTS

// MpBuf
// Used to carry media stream data between resources in a media processing
// flow graph.
//
// Key features:
// - Uses a reference count to determine when the buffer is no longer in
//   use and when it is OK to modify the data in the buffer.
// - Keeps track of the amount of data in the buffer both in terms of
//   microseconds (assuming normal playback/recording speeds) and length
//   (in bytes and also in samples).
// - Includes a format field describing the encoding of the contained data.

// Update, 4/22/99:
// Each buffer in a particular pool is the same size.
// Each pool is created with an option that specifies whether cache
//   alignment is required (the value of the option is 0 to mean
//   no alignment, or a power of two that is the size of the cache
//   alignment unit).
// Each buffer has an attribute that specifies a header size, which is
//   the offset in bytes to the first sample.  In particular, this is how
//   space will be reserved for the RTP header in both inbound and outbound
//   RTP packets.  This will be kept redundantly in a parallel field that
//   points to the first sample.
// When a buffer is acquired from a pool, the request will specify:
//  - the pool from which to take it, optionally via a convenience function
//    that provides "in the same pool as this other buffer";
//  - the header size (or offset to the first sample);
//  - the number of samples;
//  - the content format.
// The initial reference count will be 1.
// The start time, time length, and number of samples are not maintained
//   by the buffer abstraction, but are provided as a convenience for the
//   application.

typedef struct __MpBuf_tag
{
   int   refCnt;     // reference count, whose value is interpreted as:
                     // <0: an error has been detected with this buffer.
                     // =0: the buffer may be freed
                     // =1: the buffer is used by one entity, that entity
                     //     may modify the contents of the buffer and may
                     //     also modify selected fields in the buf header
                     // >1: the buffer is used by multiple entities, none
                     //     of which are permitted to modify it
   mpBufFormat format; // format of the data in the buffer (i.e., encoding)
   char* pStorage;   // pointer to the beginning of the data storage area
                     //  of the buffer.  Fixed.
   int   byteLen;    // maximum length (in bytes) that can be stored in
                     //  the buffer.  Fixed.
   int   offset;     // # bytes at beginning for non-sample (header) storage.
   int   curBytes;   // actual length (in bytes) of the data stored in
                     //  the buffer.
   int   usecs;      // actual length (in microseconds) of the data
                     //  stored in the buffer.
   int   startTime;  // time value used to synchronize multiple media
                     //  streams
   int   numSamples; // current number of audio data samples in buffer
   Sample* pSamples; // pointer to the first sample (offset beyond header)
   MpBufPoolPtr pPool; // Pointer to the MpBufPool from which this buffer
                     //  was allocated (needed to free the buffer)
} MpBuf;


// MpBufPool
// Allocator for data buffers and buffer headers.
typedef struct __MpBufPool_tag
{
   int      size;        // Amount of memory reserved for the pool (in bytes)
   int      allocCnt;    // Number of allocated buffers and buffer headers
   int      allocSize;   // Amount of memory currently allocated to buffers
                         //  and headers (in bytes)
   int      maxAllocCnt; // Max allocCnt (for the lifetime of the pool)
   int      maxAllocSize;// Max allocSize (for the lifetime of the pool)
   // other fields needed by the allocator
   int      eachLen;     // the number of bytes in each buffer's storage block.
   char    *data;        // pointer to the malloc'ed storage space.
   MpBufPtr buffers;     // malloc'ed array of buffer headers
   int      lastTaken;   // used by allocator to reduce searching
   int      cacheAlignment;// 0 if not aligned, otherwise boundary size.
   SEM_ID   mutex;       // mutex semaphore for thread safety
} MpBufPool;

/* //////////////////////////// EXTERNAL FUNCTIONS //////////////////////// */

// TYPEDEFS
// EXTERNAL VARIABLES
// EXTERNAL FUNCTIONS

// FUNCTIONS

// MpBuf methods
// MpBuf_getBuf    - Allocate a buffer header and an empty data buffer from
//                   the indicated pool. The reference count on the new buffer
//                   header will be set to 1 (other initial values TBD).
//                   Return NULL if the pool has no buffer available.
// MpBuf_addRef    - Add a reference (increment the reference count)
// MpBuf_delRef    - Delete a reference (decrement the reference count); when
//                   the count decrements from 1 to 0 the buffer becomes free.
// MpBuf_copy      - Make a copy of the buffer header and data.  The
//                   reference count on the buffer header for the new copy
//                   will be set to 1.  Return NULL if the header or buffer
//                   could not be allocated.
// MpBuf_getStorage - Get the pointer to the start of the allocated storage
//                   for the data buffer
// MpBuf_getSamples - Get the pointer to the start of the sample storage in
//                   the data buffer (beyond the offset reserved, eg. for RTP).
// MpBuf_getFormat - Return the buffer format
// MpBuf_setFormat - Set the buffer format (only if refCnt == 1).
// MpBuf_getOffset - Get the offset reserved before sample data (eg. RTP header)
// MpBuf_setOffset - Set the offset reserved before sample data (only if
//                   refCnt == 1).
// MpBuf_getNumSamples - Get the count of the samples in the data buffer
// MpBuf_setNumSamples - Set the count of the samples in the data buffer (only
//                   if refCnt == 1).
// MpBuf_getByteLen - Get the maximum length (in bytes) that can be stored
//                   in the buffer
// MpBuf_getUsecs  - Get the length of the data (in microseconds)
// MpBuf_setUsecs  - Set the length of the data (in microseconds)
// MpBuf_getStartTime  - Get the start time of the data (in microseconds)
// MpBuf_setStartTime  - Set the start time of the data (in microseconds)


Former methods, proposed for deletion:
-- MpBuf_free      - Free the buffer
-- MpBuf_delete    - Free the buffer and buffer header.  If force=1, then
--                   free the buffer even if the refCnt is >0. Otherwise,
--                   if the refCnt is >0, return an error
-- MpBuf_getData   - Return a ptr to the current position within the buffer
-- MpBuf_setData   - Set the current position within the buffer. Return -1 if
--                   refCnt != 1

Renamed methods:
-+ MpBuf_MpBuf... MpBuf_getBuf [no longer a creator]
-+ MpBuf_getData... MpBuf_getSamples
-+ MpBuf_setData... MpBuf_setOffset
-+ MpBuf_getDataStart... MpBuf_getStorage
-+ MpBuf_getLength... MpBuf_getNumSamples
-+ MpBuf_setLength... MpBuf_setNumSamples
-+ MpBuf_free... MpBuf_delRef

extern MpBufPtr MpBuf_getBuf(MpBufPoolPtr pPool, int initSamples,
                            int initOffset, int initFormat);
extern int      MpBuf_addRef(MpBufPtr thisObj);
extern int      MpBuf_delRef(MpBufPtr thisObj);
extern MpBufPtr MpBuf_copy(MpBufPtr thisObj, MpBufPoolPtr pPool);
extern Sample*  MpBuf_getSamples(MpBufPtr thisObj);
extern STATUS   MpBuf_setOffset(MpBufPtr thisObj, int offset);
extern char*    MpBuf_getStorage(MpBufPtr thisObj);
extern int      MpBuf_getFormat(MpBufPtr thisObj);
extern STATUS   MpBuf_setFormat(MpBufPtr thisObj, int format);
extern int      MpBuf_getNumSamples(MpBufPtr thisObj);
extern STATUS   MpBuf_setNumSamples(MpBufPtr thisObj, int numSamples);
extern int      MpBuf_getByteLen(MpBufPtr thisObj);
extern int      MpBuf_getUsecs(MpBufPtr thisObj);
extern STATUS   MpBuf_setUsecs(MpBufPtr thisObj, int usecs);
extern int      MpBuf_getStartTime(MpBufPtr thisObj);
extern STATUS   MpBuf_setStartTime(MpBufPtr thisObj, int usecs);

// MpBufPool functions
// MpBufPool_MpBufPool - Create and initialize the buffer pool.  Request a
//                   chunk of memory from the system for this purpose.
// MpBufPool_delete - Free the buffer pool.  OK if every refCnt is 0, or ERROR.
// MpBufPool_allocBuf - Allocate a buffer header and buffer from the
//                   indicated pool.  The reference count of the
//                   buffer header will be set to 1.  Return NULL if no
//                   buffer is available to be allocated.  (In the future
//                   we may allow this to acquire more space and carve it
//                   into more buffers...)
// MpBufPool_freeBuf - Free the buffer.  Called when the refCnt of the
//                   buffer is decremented from 1 to 0.

extern MpBufPoolPtr MpBufPool_MpBufPool(
   int poolSize,       // number of bytes to malloc (including headers?)
   int max_buffer_len, // bytes per buffer
   int numBuffers,     // used if poolSize is 0
   int cacheAlignment, // 0=>not aligned, otherwise power of 2 boundary
);
extern STATUS       MpBufPool_delete(MpBufPoolPtr thisObj);
extern MpBufPtr     MpBufPool_allocBuf(MpBufPoolPtr thisObj);
extern STATUS       MpBufPool_freeBuf(MpBufPoolPtr thisObj, MpBufPtr pBuf,
                                      int force);



#ifdef __cplusplus
}
#endif

#endif  // _MpDefs_h_
