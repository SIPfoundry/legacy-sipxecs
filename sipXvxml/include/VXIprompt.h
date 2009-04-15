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
 ************************************************************************
 */

#ifndef _VXIPROMPT_H
#define _VXIPROMPT_H

#include "VXItypes.h"                  /* For VXIchar definition */
#include "VXIvalue.h"                  /* For VXIMap */

#include "VXIheaderPrefix.h"
#ifdef VXIPROMPT_EXPORTS
#define VXIPROMPT_API SYMBOL_EXPORT_DECL
#else
#define VXIPROMPT_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name VXIprompt
 * @memo Prompt Interface
 * @version 1.0
 * @doc
 * Abstract interface for Prompting functionality. Prompts are handled
 * as a series of audio segments, where segments may be paths to
 * on-disk audio files or text, URIs to remote audio or text,
 * in-memory binary audio, in-memory text for playback via a
 * Text-to-Speech engine, or in-memory text for playback using
 * concatenative audio routines (123 played as "1.ulaw 2.ulaw
 * 3.ulaw"). <p>
 *
 * The Prompt interface the handles prefetching, caching, and
 * streaming audio as required to provide good response times and low
 * CPU and network overhead. <p>
 *
 * There is one prompt interface per thread/line.  
 */

/*@{*/
/**
 * Keys identifying properties in VXIMap for PlayFiller( ), Prefetch( ),
 * and Queue( ).
 *
 * Note that VXIinet properties should also be included, these are
 * simply passed through to VXIinet for fetches as-is.
 */
#define PROMPT_AUDIO_REFS        L"vxi.prompt.audioReferences" /* VXIMap     */
#define PROMPT_LANGUAGE          L"vxi.prompt.language"        /* VXIString  */
#define PROMPT_RECORDING_SOURCE  L"vxi.prompt.recordingSource" /* VXIString  */
#define PROMPT_TTS_VOICE_NAME    L"vxi.prompt.tts.voiceName"   /* VXIString  */

/* Property defaults */
#define PROMPT_AUDIO_REFS_DEFAULT             NULL /* No default */
#define PROMPT_LANGUAGE_DEFAULT               L"en-US" /* US English */
#define PROMPT_RECORDING_SOURCE_DEFAULT       L"" /* use platform default */
#define PROMPT_TTS_VOICE_NAME_DEFAULT         L"" /* use TTS engine default */

/**
 * Keys identifying properties in the VXIMap for an individual audio
 * reference, used to store data to play that will be accessed later
 * via a URI (see PROMPT_AUDIO_REFS_SCHEME below).  Each audio
 * reference is set as a property within PROMPT_AUDIO_REFS where the
 * key name within PROMPT_AUDIO_REFS must match the key name used in
 * the audio reference URI, and where the value for each key is a
 * VXIMap. Those children VXIMaps have the following properties, which
 * match the arguments for Queue( ) except for "data" which is a more
 * flexible version of the "text" argument for Queue( ) (supports
 * providing data of types other then just text).
 *
 * Note that PROMPT_[...] properties as defined for Queue( ) above and
 * VXIinet properties may also be included as override the properties
 * for the original PlayFiller( ), Prefetch( ), or Queue( ) request.  
 */
#define PROMPT_AUDIO_REF_TYPE              \
    L"vxi.prompt.audioReference.type"            /* VXIString, omit for
                                                    VXIContent data */
#define PROMPT_AUDIO_REF_SRC               \
    L"vxi.prompt.audioReference.src"             /* VXIString, omit if
						    specifying data */
#define PROMPT_AUDIO_REF_DATA              \
    L"vxi.prompt.audioReference.data"            /* VXIInteger, VXIFloat,
                                                    VXIString, or VXIContent,
						    omit if specifying src */
  
/**
 * URI scheme for audio references, the text following the colon must
 * be the key name for the referenced audio as present in the
 * PROMPT_AUDIO_REFS property. (Note: do not append "//" unless it is
 * actually part of the audio reference key name.) 
 */
#define PROMPT_AUDIO_REFS_SCHEME           L"x-vxiprompt-ref:"
#define PROMPT_AUDIO_REFS_SCHEME_LENGTH    16

/**
 * Result codes for interface methods
 * 
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXIpromptResult {
  /* Fatal error, terminate call     */
  VXIprompt_RESULT_FATAL_ERROR       =  -100, 
  /* I/O error                       */
  VXIprompt_RESULT_IO_ERROR           =   -8,
  /* Out of memory                   */
  VXIprompt_RESULT_OUT_OF_MEMORY      =   -7, 
  /* System error, out of service    */
  VXIprompt_RESULT_SYSTEM_ERROR       =   -6, 
  /* Errors from platform services   */
  VXIprompt_RESULT_PLATFORM_ERROR     =   -5, 
  /* Return buffer too small         */
  VXIprompt_RESULT_BUFFER_TOO_SMALL   =   -4, 
  /* Property name is not valid      */
  VXIprompt_RESULT_INVALID_PROP_NAME  =   -3, 
  /* Property value is not valid     */
  VXIprompt_RESULT_INVALID_PROP_VALUE =   -2, 
  /* Invalid function argument       */
  VXIprompt_RESULT_INVALID_ARGUMENT   =   -1, 
  /* Success                         */
  VXIprompt_RESULT_SUCCESS            =    0,
  /* Normal failure, nothing logged  */
  VXIprompt_RESULT_FAILURE            =    1,
  /* Non-fatal non-specific error    */
  VXIprompt_RESULT_NON_FATAL_ERROR    =    2, 
  /* URI fetch timeout               */
  VXIprompt_RESULT_FETCH_TIMEOUT      =   50,
  /* URI fetch error                 */
  VXIprompt_RESULT_FETCH_ERROR        =   51,
  /* Bad say-as class                */
  VXIprompt_RESULT_BAD_SAYAS_CLASS    =   52,
  /* TTS access failure              */
  VXIprompt_RESULT_TTS_ACCESS_ERROR   =   53,
  /* TTS unsupported language        */
  VXIprompt_RESULT_TTS_BAD_LANGUAGE   =   54,
  /* TTS unsupported document type   */
  VXIprompt_RESULT_TTS_BAD_DOCUMENT   =   55,
  /* TTS document syntax error       */
  VXIprompt_RESULT_TTS_SYNTAX_ERROR   =   56,
  /* TTS generic error               */
  VXIprompt_RESULT_TTS_ERROR          =   57,
  /* Resource busy, such as TTS      */
  VXIprompt_RESULT_RESOURCE_BUSY      =   58,
  /* HW player unsupported MIME type */
  VXIprompt_RESULT_HW_BAD_TYPE        =   59,
  /* Generic HW player error          */
  VXIprompt_RESULT_HW_ERROR           =   60,
  /* Operation is not supported      */
  VXIprompt_RESULT_UNSUPPORTED        =  100
} VXIpromptResult;


/*
** ==================================================
** VXIpromptInterface Interface definition
** ==================================================
*/
/** @name VXIpromptInterface
 ** @memo VXIprompt interface for prompting
 **
 */
typedef struct VXIpromptInterface {
  /**
   * @name GetVersion
   * @memo Get the VXI interface version implemented
   *
   * @return  VXIint32 for the version number. The high high word is 
   *          the major version number, the low word is the minor version 
   *          number, using the native CPU/OS byte order. The current
   *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
   */ 
  VXIint32 (*GetVersion)(void);
  
  /**
   * @name GetImplementationName
   * @memo Get the name of the implementation
   *
   * @return  Implementation defined string that must be different from
   *          all other implementations. The recommended name is one
   *          where the interface name is prefixed by the implementor's
   *          domain name in reverse order, such as "com.xyz.rec" for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   */
  const VXIchar* (*GetImplementationName)(void);

  /**
   * @name BeginSession
   * @memo Reset for a new session.
   *
   * @doc
   * This must be called for each new session, allowing for
   * call specific handling to occur. For some implementations, this
   * can be a no-op.  For others runtime binding of resources or other
   * call start specific handling may be done on this call.
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for the new session
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*BeginSession)(struct VXIpromptInterface  *pThis,  
                                  VXIMap                     *args);

  /**
   * @name EndSession
   * @memo Performs cleanup at the end of a call session.
   *
   * @doc
   * This must be called at the termination of a call, allowing for
   * call specific termination to occur. For some implementations, this 
   * can be a no-op. For others runtime resources may be released or 
   * other adaptation may be completed.
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for ending the session
   *
   * @result VXIprompt_RESULT_SUCCESS on success
   */
  VXIpromptResult (*EndSession)(struct VXIpromptInterface  *pThis, 
                                VXIMap                     *args);

  /**
   * @name Play
   * @memo Start playing queued segments, blocking if bargein is false, 
   * otherwise non-blocking
   *
   * @doc
   * Segments queued after this is called will not be played until
   * this is called again.  It is possible errors may occur after this
   * function has returned: Wait( ) will return the appropriate error
   * if one occurred.
   * 
   * Note that this stops the current PlayFiller( ) operation although
   * possibly after some delay, see PlayFiller( ) for more information.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*Play)(struct VXIpromptInterface  *pThis);
  
  /**
   * @name Stop
   * @memo Stop playing queued segments, non-blocking
   *
   * @doc
   * Segments queued after this is called will not be played until
   * this is called again.  It is possible errors may occur after this
   * function has returned: Wait( ) will return the appropriate error
   * if one occurred.
   * 
   * Note that this stops the current PlayFiller( ) operation although
   * possibly after some delay, see PlayFiller( ) for more information.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*Stop)(struct VXIpromptInterface  *pThis);
  
  /**
   * @name PlayFiller
   * @memo Start the special play of a filler segment, non-blocking
   * @doc
   * This plays a standard segment in a special manner in order to
   * satisfy "filler" needs. A typical example is the VoiceXML
   * fetchaudio attribute, used to specify filler audio that gets
   * played while a document fetch is being performed and then
   * interrupted once the fetch completes.
   *
   * The filler segment is played to the caller once all active Play( ) 
   * operations, if any, have completed. If Play( ), Wait( ), or
   * PlayFiller( ) is called before the filler segment starts playing
   * it is canceled and never played. If one of those functions is
   * instead called after the filler segment starts playing, the
   * filler segment is stopped once the minimum playback duration 
   * expires.
   * 
   * Note that this does not trigger the play of segments that have
   * been queued but not yet played via Play( ).
   * 
   * @param type        [IN] Type of segment, either a MIME content type,
   *                    a say-as class name, or NULL to automatically detect
   *                    a MIME content type (only valid when src is 
   *                    non-NULL). The supported MIME content types and 
   *                    say-as class names are implementation dependent.
   * @param src         [IN] URI or platform dependent path to the content,
   *                    pass NULL when specifying in-memory text. As
   *                    a special case, this may use a special scheme as
   *                    defined by PROMPT_AUDIO_REFS_SCHEME above in order
   *                    to play in-memory binary audio.
   * @param text        [IN] Text (possibly with markup) to play via TTS 
   *                    or say-as classes, pass NULL when src is non-NULL.
   *                    The format of text for say-as class playback is
   *                    determined by each class implementation. The
   *                    format of text for TTS playback may be W3C
   *                    SSML (type set to VXI_MIME_SSML) or simple wchar_t 
   *                    text (type set to VXI_MIME_UNICODE_TEXT), the 
   *                    implementation may support other formats as well.
   * @param properties  [IN] Properties to control the fetch, queue, and
   *                    play, as specified above. May be NULL.
   * @param minPlayMsec [IN] Minimum playback duration for the
   *                    filler prompt once it starts playing, in
   *                    milliseconds. This is used to "lock in" a play
   *                    so that no less then this amount of audio is
   *                    heard by the caller once it starts playing,
   *                    avoiding confusion from audio that is played
   *                    for an extremely brief duration and then cut
   *                    off. Note that the filler prompt may never
   *                    be played at all, however, if canceled before
   *                    it ever starts playing as described above.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*PlayFiller)(struct VXIpromptInterface *pThis,
                                const VXIchar             *type,
                                const VXIchar             *src,
                                const VXIchar             *text,
                                const VXIMap              *properties,
                                VXIlong                    minPlayMsec);

  /**
   * @name Prefetch
   * @memo Prefetch a segment, non-blocking
   * @doc
   * This fetches the segment in the background, since this returns
   * before the fetch proceeds failures during the fetch will not be
   * reported (invalid URI, missing file, etc.). It is critical to
   * call this prior to Queue( ) (possibly multiple times with
   * increasing VXIinet prefetch priorities as the time for playback
   * gets closer) in order to minimize prompt gaps due to fetch
   * latencies.
   * 
   * @param type        [IN] Type of segment, either a MIME content type,
   *                    a say-as class name, or NULL to automatically detect
   *                    a MIME content type (only valid when src is 
   *                    non-NULL). The supported MIME content types and 
   *                    say-as class names are implementation dependent.
   * @param src         [IN] URI or platform dependent path to the content,
   *                    pass NULL when specifying in-memory text
   * @param text        [IN] Text (possibly with markup) to play via TTS 
   *                    or say-as classes, pass NULL when src is non-NULL.
   *                    The format of text for say-as class playback is
   *                    determined by each class implementation. The
   *                    format of text for TTS playback may be W3C
   *                    SSML (type set to VXI_MIME_SSML) or simple wchar_t 
   *                    text (type set to VXI_MIME_UNICODE_TEXT), the 
   *                    implementation may support other formats as well.
   * @param properties  [IN] Properties to control the fetch, queue, and
   *                    play, as specified above. May be NULL.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*Prefetch)(struct VXIpromptInterface *pThis,
                              const VXIchar             *type,
                              const VXIchar             *src,
                              const VXIchar             *text,
                              const VXIMap              *properties);
  
  /**
   * @name Queue
   * @memo Queue a segment for playing, blocking
   * @doc
   * The segment does not start playing until the Play( ) method is
   * called. This call blocks until the prompt's data is retrieved
   * (for streaming plays until the data stream starts arriving) so
   * that the caller can be assured the segment is available for
   * playback (important for supporting play with fallback).
   * 
   * @param type        [IN] Type of segment, either a MIME content type,
   *                    a say-as class name, or NULL to automatically detect
   *                    a MIME content type (only valid when src is 
   *                    non-NULL). The supported MIME content types and 
   *                    say-as class names are implementation dependent.
   * @param src         [IN] URI or platform dependent path to the content,
   *                    pass NULL when specifying in-memory text. As
   *                    a special case, this may use a special scheme as
   *                    defined by PROMPT_AUDIO_REFS_SCHEME above in order
   *                    to play in-memory binary audio.
   * @param text        [IN] Text (possibly with markup) to play via TTS 
   *                    or say-as classes, pass NULL when src is non-NULL.
   *                    The format of text for say-as class playback is
   *                    determined by each class implementation. The
   *                    format of text for TTS playback may be W3C
   *                    SSML (type set to VXI_MIME_SSML) or simple wchar_t 
   *                    text (type set to VXI_MIME_UNICODE_TEXT), the 
   *                    implementation may support other formats as well.
   * @param properties  [IN] Properties to control the fetch, queue, and
   *                    play, as specified above. May be NULL.
   *
   * @param bargein     [IN] bargein disabled or enabled to control the play of prompt play.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*Queue)(struct VXIpromptInterface *pThis,
                           const VXIchar             *type,
                           const VXIchar             *src,
                           const VXIchar             *text,
                           const VXIMap              *properties, 
                           int                       bargein);
  
  /**
   * @name Wait
   * @memo Wait until all played segments finish playing, blocking
   * @doc
   * Note that this stops the current PlayFiller( ) operation although
   * possibly after some delay, see PlayFiller( ) for more information.
   *
   * @param playResult [OUT] Most severe error code resulting from
   *                   a Play( ) operation since the last Wait( )
   *                   call, since Play( ) is asynchronous errors 
   *                   may have occurred after one or more calls to
   *                   it have returned. Note that this ignores any
   *                   errors resulting from PlayFiller( ) operations.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*Wait)(struct VXIpromptInterface *pThis,
                          VXIpromptResult           *playResult);

  /**
   * Add listener to the stream queue player.
   *
   * @result VXIprompt_RESULT_SUCCESS on success 
   */
  VXIpromptResult (*AddPlayerListener)(struct VXIpromptInterface  *pThis, 
                                  void* pListener);


} VXIpromptInterface;

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */
