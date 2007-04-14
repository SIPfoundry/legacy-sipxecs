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

#ifndef _VXIREC_H
#define _VXIREC_H

#include "VXItypes.h"                  /* For VXIchar definition */
#include "VXIvalue.h"                  /* For VXIMap */

#include "VXIheaderPrefix.h"
#ifdef VXIREC_EXPORTS
#define VXIREC_API SYMBOL_EXPORT_DECL
#else
#define VXIREC_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct VXIrecGrammar;
#else
typedef struct VXIrecGrammar { void * dummy; } VXIrecGrammar;
#endif

#define VXIREC_MIMETYPE_ULAW    VXI_MIME_ULAW
#define VXIREC_MIMETYPE_NLSML   L"application/grammar+nlsml"

/**
 * @name VXIrec
 * @memo Recognizer Interface
 * @version 1.0
 * @doc
 * Abstract interface for recognition functionality required by
 * VoiceXML. Recognition is performed against VXIrecGrammars, abstract
 * grammar types managed by the interface. The exact grammar formats
 * handled are implementation dependant. <p>
 *
 * If an asynchronous problem/error occurs in the platform's
 * recognizer, the VXI is notified through the VXIrecResult code for
 * the current or next function call. <p>
 *
 * There is one recognizer interface per thread/line.
 */

/*@{*/
/**
 * Keys identifying properties in VXIMap
 *
 * For LoadGrammarURI( ) and LoadGrammarString( ), the only standard
 * properties are VXIinet properties that are simply passed through to
 * VXIinet for fetches as-is. The implementation may define additional
 * properties. <p>
 *
 * For Recognize( ) and Record( ), the following standard properties
 * are defined.  The implementation may define additional properties.
 */


#define REC_CONFIDENCE_LEVEL        L"vxi.rec.confidenceLevel"   /* VXIFloat 0.0 - 1.0 */
#define REC_DTMF_TIMEOUT            L"vxi.rec.timeout"           /* VXIInteger */
#define REC_DTMF_TIMEOUT_INTERDIGIT L"vxi.rec.timeoutInterdigit" /* VXIInteger */
#define REC_DTMF_TIMEOUT_TERMINATOR L"vxi.rec.timeoutTerminator" /* VXIInteger */
#define REC_DTMF_TERMINATOR_CHAR    L"vxi.rec.terminatorChar"    /* VXIString with one character */
#define REC_INPUT_MODES             L"vxi.rec.inputModes"        /* VXIInteger, VXIrecInputMode value */
#define REC_RESULT_SAVE_WAVEFORM    L"vxi.rec.saveWaveform"      /* VXIInteger, 0 for disabled, 1 for enabled */
#define REC_RESULT_NBEST_SIZE       L"vxi.rec.resultNbestSize"   /* VXIInteger */
#define REC_SENSITIVITY             L"vxi.rec.sensitivity"       /* VXIFloat 0.0 - 1.0 */
#define REC_TIMEOUT                 L"vxi.rec.timeout"           /* VXIInteger */
#define REC_TIMEOUT_COMPLETE        L"vxi.rec.timeoutComplete"   /* VXIInteger */
#define REC_TIMEOUT_INCOMPLETE      L"vxi.rec.timeoutIncomplete" /* VXIInteger */
#define REC_SPEED_VS_ACCURACY       L"vxi.rec.speedVsAccuracy"   /* VXIFloat 0.0-1.0 */
#define REC_LANGUAGE                L"vxi.rec.language"          /* VXIString */

#define REC_MAX_RECORDING_TIME      L"vxi.rec.maxRecordingTime"  /* VXIInteger */
#define REC_TERMINATED_ON_DTMF      L"vxi.rec.terminatedOnDTMF"  /* VXIInteger, 0 for disabled, 1 for enabled */
#define REC_RECORD_MIME_TYPE        L"vxi.rec.recordingMimeType" /* VXIString */  
#define REC_RECORD_BEEP_URL         L"vxi.rec.recordingBeepURL"  /* VXIString */  

/**
 * Input modes as set in the REC_INPUT_MODE property defined above and
 * as returned in VXIrecResult structures
 */
typedef enum VXIrecInputMode {
  REC_INPUT_MODE_DTMF         = 2,  /* Only DTMF input accepted       */
  REC_INPUT_MODE_SPEECH       = 4,  /* Only speech input accepted     */
  REC_INPUT_MODE_DTMF_SPEECH  = 6   /* DTMF and speech input accepted */
} VXIrecInputMode;

/**
 * MIME content types for LoadGrammarURI( ) and LoadGrammarString( ),
 * the implementation usually supports additional MIME types 
 */
#define REC_MIME_GENERIC_DTMF   L"text/x-grammar-dtmf"
#define REC_MIME_CHOICE         L"text/x-grammar-choice"
#define REC_MIME_CHOICE_DTMF    L"text/x-grammar-choice-dtmf"
#define REC_MIME_OPTION         L"text/x-grammar-option"
#define REC_MIME_OPTION_DTMF    L"text/x-grammar-option-dtmf"

/**
 * Status codes for recognition results
 */
typedef enum VXIrecStatus { 
  REC_STATUS_SUCCESS      = 0,  /* Recognition returned a hypothesis      */
  REC_STATUS_FAILURE      = 1,  /* Speech detected, no likely hypothesis  */
  REC_STATUS_TIMEOUT      = 2,  /* No speech was detected                 */
  REC_STATUS_DISCONNECT   = 3,  /* Caller has disconnected; no hypothesis */
  REC_STATUS_ERROR        = 4,  /* An error aborted recognition           */
  REC_STATUS_CONTINUE     = 5,  /* In progress, not completed yet         */
  REC_STATUS_DTMFSUCCESS  = 6,  /* Recognition returned an answer, no match*/
} VXIrecStatus;

/**
 * Keys identifying properties in VXIMap used to return recognition
 * result information for each n-best entry, see
 * VXIrecRecognitionResult below for details 
 */
#define REC_KEYS     L"vxi.rec.keys"        /* VXIVector of VXIStrings   */
#define REC_VALUES   L"vxi.rec.values"      /* VXIVector of VXIStrings   */
#define REC_CONF     L"vxi.rec.confidence"  /* VXIVector of VXIFloats    */
#define REC_RAW      L"vxi.rec.raw"         /* VXIVector of VXIStrings   */
#define REC_GRAMMAR  L"vxi.rec.grammar"     /* VXIPtr to a VXIrecGrammar */

/** Default timeout - 5000 (5 seconds) */
#define VXI_TIMEOUT_DEFAULT             5000
#define VXI_RECOGNIZE_MAX_INPUT_LENGTH  100

/**
 * Recognition results structure as returned by Recognize( )
 */
typedef struct VXIrecRecognitionResult {
  /** Status code for the recognition as defined above */
  VXIrecStatus     status;
  /** Input modes as defined above */
  VXIrecInputMode  mode;

  /*
   * Waveform of the caller's utterance. Returned a VXIContent which
   * stores the length and the mime type in one unit.
   */
  VXIContent*      waveform;

  /*
   * Detailed recognition results, a VXIVector where each element
   * contains an n-best result where the first element (index 0) has
   * the highest confidence and each successive element has a lower
   * confidence. The number of n-best entries can be determined by
   * calling VXIVectorLength( ). For unsuccessful recognition the
   * results pointer is NULL.
   *
   * Each result is returned as a VXIMap with the following key
   * names as defined above for providing speech or dtmf results:
   *
   * vxi.rec.keys        VXIVector containing the keys for the results,
   *                 each key is a VXIString
   * vxi.rec.values      VXIVector containing the value for each key,
   *                 each result value is a VXIString where the
   *                 Nth value in this vector is for the Nth key
   *                 in the vxi.rec.keys vector
   * vxi.rec.confidence  VXIVector containing the score for each key,
   *                 each confidence score is a VXIFloat with a value
   *                 between 0.0 (no confidence) and 1.0 (extremely
   *                 confident).  The Nth score in this vector is for 
   *                 the Nth key in the vxi.rec.keys vector.
   * vxi.rec.raw         VXIString with the raw recognition result value
   *                 (there is only one raw recognition result for each
   *                 n-best entry)
   * vxi.rec.grammar     VXIPtr with the VXIrecGrammar handle that matched
   *                 (there is only one grammar for each n-best entry)
   */
  VXIVector*      results;

  /*
   * In the near future, the results vector will be replaced by a VXIContent
   * containing the semantic interpretation of the recognition.  This might
   * use the Natural Language Semantic Markup Language (NLSML), an
   * ECMAScript-like format, or some standard not yet proposed.  The mimetype
   * of the VXIContent may allow multiple formats to be supported.
   *
   * VXIContent*     xmlresult;
   */

  /*
   * Call to release the resources stored with this result structure
   */
  void            (*Destroy)(struct VXIrecRecognitionResult **result);
} VXIrecRecognitionResult;


/**
 * Record results structure as returned by Record( )
 */
typedef struct VXIrecRecordResult {
  /** Status code for the recognition as defined above */
  VXIrecStatus status;

  /** The bytes of the recording */
  VXIContent * waveform;

  /** Duration of the recording in milliseconds */
  VXIunsigned duration;

  /** Duration of the recording in bytes */
  VXIunsigned size;

  /** Timestamp of the recording */
  VXIchar* timestamp;

  /** DTMF character which terminated the recording, or 0 if none. */
  VXIbyte termchar;

  /*
  ** Call to release the resources stored with this result structure
  */
  void            (*Destroy)(struct VXIrecRecordResult **result);
} VXIrecRecordResult;


/**
 * Result codes for interface methods
 * 
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXIrecResult {
  /* Fatal error, terminate call     */
  VXIrec_RESULT_FATAL_ERROR        = -100, 
  /* I/O error                       */
  VXIrec_RESULT_IO_ERROR           =   -8,
  /* Out of memory                   */
  VXIrec_RESULT_OUT_OF_MEMORY      =   -7, 
  /* System error, out of service    */
  VXIrec_RESULT_SYSTEM_ERROR       =   -6, 
  /* Errors from platform services   */
  VXIrec_RESULT_PLATFORM_ERROR     =   -5, 
  /* Return buffer too small         */
  VXIrec_RESULT_BUFFER_TOO_SMALL   =   -4, 
  /* Property name is not valid      */
  VXIrec_RESULT_INVALID_PROP_NAME  =   -3, 
  /* Property value is not valid     */
  VXIrec_RESULT_INVALID_PROP_VALUE =   -2, 
  /* Invalid function argument       */
  VXIrec_RESULT_INVALID_ARGUMENT   =   -1, 
  /* Success                         */
  VXIrec_RESULT_SUCCESS            =    0,
  /* Normal failure, nothing logged  */
  VXIrec_RESULT_FAILURE            =    1,
  /* Non-fatal non-specific error    */
  VXIrec_RESULT_NON_FATAL_ERROR    =    2, 
  /* URL fetch timeout               */
  VXIrec_RESULT_FETCH_TIMEOUT      =   50,
  /* URL fetch error                 */
  VXIrec_RESULT_FETCH_ERROR        =   51,
  /* Unsupported grammar type        */
  VXIrec_RESULT_BAD_GRAMMAR        =   52,
  /* Grammar syntax error            */
  VXIrec_RESULT_SYNTAX_ERROR       =   53,
  /* Unsupported MIME type           */
  VXIrec_RESULT_BAD_MIME_TYPE      =   54,
  /* Generic HW source error         */
  VXIrec_RESULT_HW_ERROR           =   55,
  /* Call disconnected               */
  VXIrec_RESULT_DISCONNECT        =   56,
  /* Operation is not supported      */
  VXIrec_RESULT_UNSUPPORTED        =  100
} VXIrecResult;


/*
** ==================================================
** VXIrecInterface Interface definition
** ==================================================
*/
/** @name VXIrecInterface
 ** @memo VXIrec interface for speech recognition
 **
 */
typedef struct VXIrecInterface {
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
   *          where the interface name is prefixed by the implementator's
   *          Internet address in reverse order, such as com.xyz.rec for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   */
  const VXIchar* (*GetImplementationName)(void);
  
  /**
   * @name BeginSession
   * @memo Reset for a new speaker
   *
   * @doc
   * This must be called for each new caller, allowing for
   * caller-specific recognition adaptation to occur.
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for the new session
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*BeginSession)(struct VXIrecInterface  *pThis,  
			       VXIMap                 *args);

  /**
   * @name EndSession
   * @memo Performs cleanup at the end of a speaker's session
   * @doc 
   * This must be called at the termination of a call, allowing for
   * call specific termination to occur. For some implementations, this can be a no-op. For
   * others runtime resources may be released or other adaptation may be completed.
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for ending the session
   *
   * @result VXIrec_RESULT_SUCCESS on success
   */
  VXIrecResult (*EndSession)(struct VXIrecInterface  *pThis, 
			     VXIMap                 *args);

  /**
   * @name LoadGrammarURI
   * @memo Load a grammar from a URI, typically non-blocking.
   *
   * @doc
   * Initiates grammar creation from a URI (Universal Resource
   * Identifier).  The resulting grammar must be activated before use.
   * This allows for multiple grammars to be loaded when the page is
   * read.  Grammar creation may proceed either synchronously or
   * asynchronously, but must be available once ActivateGrammar( )
   * returns.
   *
   * The supported grammar formats are determined by the implementor,
   * but certain VoiceXML built-in types (e.g., grammar
   * src=builtin:grammar/digits?length=5) must be supported.
   *
   * @param properties  [IN]  Set of properties as defined above
   * @param type        [IN]  MIME type of the grammar, one of the
   *                           REC_MIME_[...] defines above or an
   *                           implementation defined grammar type
   * @param uri         [IN]  URI of the grammar definition
   * @param uriArgs     [IN]  Caching, timeout, base URI and other properties
   *                           associated with URI retrieval.
   * @param gram        [OUT] Handle to the new grammar 
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*LoadGrammarURI)(struct VXIrecInterface  *pThis,
                                 const VXIMap            *properties,
                                 const VXIchar           *type, 
                                 const VXIchar           *uri,
                                 const VXIMap           *uriArgs,
                                 VXIrecGrammar          **gram);

  /**
   * @name LoadGrammarString
   * @memo Load an inline grammar, typically non-blocking
   *
   * @doc
   * Initiates grammar creation from a string inlined in the VoiceXML
   * page.  As with URIs, the resulting grammar must be activated
   * before use and creation may be either synchronous or
   * asynchronous.  See LoadGrammarURI( ). <p>
   *
   * The supported grammar formats are determined by the implementor,
   * but support for a minimal JSGF fragment of the following form is
   * required: <p>
   *
   *    "vocab_item {value} | vocab_item {value} | vocab_item" <p>
   *
   * This format is used by the VoiceXML interpreter for <fields> and
   * in some other cases. 
   *
   * @param properties  [IN]  Set of properties as defined above
   * @param type        [IN]  MIME type of the grammar, one of the
   *                           REC_MIME_[...] defines above or an
   *                           implementation defined grammar type
   * @param gramDef     [IN]  String containing the grammar definition
   * @param gram        [OUT] Handle to the new grammar 
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*LoadGrammarString)(struct VXIrecInterface  *pThis,
				    const VXIMap            *properties,
                                    const VXIchar           *type,
                                    const VXIchar           *gramDef,
                                    VXIrecGrammar          **gram);

  /**
   * @name ActivateGrammar
   * @memo Activate a loaded VXIrecGrammar for subsequent recognition calls
   *
   * @doc
   * Multiple grammars may be activated before invoking Recognize( ).
   * Once this returns, the grammar must be fully loaded and ready for
   * recognition. See also LoadGrammarURI( ).
   *
   * @param gram        [IN]  Grammar to activate
   *
   * @result VXIrec_RESULT_SUCCESS on success
   */
  VXIrecResult (*ActivateGrammar)(struct VXIrecInterface  *pThis,
				  const VXIMap            *properties,
                                  VXIrecGrammar           *gram);

  /**
   * @name DeactivateGrammar
   * @memo Deactivate a loaded VXIrecGrammar for subsequent recognition calls
   * @doc Deactivate a grammar on this interface which should be used by one line.  Subsequent recognitions
   * will not use this grammar in the recognition.
   *
   * @param gram        [IN]  Grammar to deactivate
   *
   * @result VXIrec_RESULT_SUCCESS on success
   */
  VXIrecResult (*DeactivateGrammar)(struct VXIrecInterface  *pThis,
				    VXIrecGrammar           *gram);

  /**
   * @name FreeGrammar
   * @memo Free a loaded grammar.
   * 
   * @doc
   * Signals that the grammar is no longer needed.  The implementation
   * can recover resources as it sees fit.
   *
   * @param gram       [IN/OUT] Grammar to free, the pointer is set to
   *                             NULL
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*FreeGrammar)(struct VXIrecInterface  *pThis,
                              VXIrecGrammar          **gram);

  /**
   * @name Recognize
   * @memo Recognize against the currently active grammars
   * @doc
   * Blocks until recognition completes (possibly unsuccessfully),
   * DTMF input completes, a timeout occurs, or an error
   * occurs. Recognition results are returned in a structure. All
   * activated grammars are deactivated after the recognition.
   *
   * @param properties   [IN]  Set of properties as defined above
   * @param recogResult  [OUT] Newly allocated result structure 
   *                            containing the result of the
   *                            recognize operation, see the structure
   *                            definition above
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*Recognize)(struct VXIrecInterface    *pThis,
			    const VXIMap              *properties,
			    VXIrecRecognitionResult  **recogResult);


  /**
   * @name Record
   * @memo Record an utterance
   * @doc
   * Blocks until termination input occurs, a timeout occurs, or an
   * error occurs. Record results are returned in a structure.
   *
   * @param properties   [IN]  Set of properties as defined above
   * @param recordResult [OUT] Newly allocated result structure 
   *                            containing the result of the
   *                            record operation, see the structure
   *                            definition above
   *
   * @result VXIrec_RESULT_SUCCESS on success 
   */
  VXIrecResult (*Record)(struct VXIrecInterface  *pThis,
			 const VXIMap            *properties,
			 VXIrecRecordResult     **recordResult);

} VXIrecInterface;

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */
