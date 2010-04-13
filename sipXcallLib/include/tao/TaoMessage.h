//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _TaoMessage_h_
#define _TaoMessage_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "net/HttpMessage.h"
#include "tao/TaoDefs.h"
#include "tao/TaoDefs.h"
#include "tao/TaoListenerEventMessage.h"

// Field names
#define TAO_MESSAGE_CONTENT_TYPE "TAO-MESSAGE"
#define TAO_MESSAGE_ID_FIELD "TAO-MESSAGE-ID"
#define TAO_MESSAGE_TYPE_FIELD "TAO-MESSAGE-TYPE"
#define TAO_MESSAGE_SUBTYPE_FIELD "TAO-MESSAGE-SUBTYPE"
#define TAO_MESSAGE_CMD_FIELD "TAO-MESSAGE-CMD"
#define TAO_MESSAGE_HANDLE_FIELD "TAO-MESSAGE-HANDLE"
#define TAO_MESSAGE_SOCKET_FIELD "TAO-MESSAGE-SOCKET"
#define TAO_MESSAGE_ARGCNT_FIELD "TAO-MESSAGE-ARGCNT"
#define TAO_MESSAGE_ARGLIST_FIELD "TAO-MESSAGE-ARGLIST"
#define TAO_MESSAGE_QUEUEHANDLE_FIELD "TAO-MESSAGE-QUEUEHANDLE"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

//#define TAOMSG_DEBUG 1
//#define USE_HTTPMSG 1

// FORWARD DECLARATIONS

//:This class is for carrying information in Tao subsystems, typically as carrier
// of information between server and client transports.  It may be subclassed for
// different message types.
class TaoMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // MsgType categories defined for use by the system
        enum TaoMsgTypes
        {
          UNSPECIFIED = 0,

          REQUEST_ADDRESS = 1,
          REQUEST_CALL,
          REQUEST_CONNECTION,
          REQUEST_PROVIDER,
          REQUEST_TERMCONNECTION,
          REQUEST_TERMINAL,
          REQUEST_PHONEBUTTTON,
          REQUEST_PHONECOMPONENT,

          RESPONSE_ADDRESS = 9,
          RESPONSE_CALL,
          RESPONSE_CONNECTION,
          RESPONSE_PROVIDER,
          RESPONSE_TERMCONNECTION,
          RESPONSE_TERMINAL,
          RESPONSE_PHONEBUTTTON,
          RESPONSE_PHONECOMPONENT,

          EVENT = 17,

          TAO_CREATE_CALL = 18,
          TAO_CALL_CONNECT,
          TAO_CALL_DROP
        };

   // Function categories defined for use by the system
        enum TaoMsgCmds
        {
/* ----------------------------- PROVIDER --------------------------------- */
          ADD_PROVIDER_LISTENER                 = 0x00,

          CREATE_CALL                                           = 0x01,
          CREATE_CALL_RESULT                            = 0x02,
          GET_ADDRESS                                           = 0x03,
          GET_ADDRESSES                                 = 0x04,
          GET_CALLS                                                     = 0x05,
          GET_CONNECTION                                        = 0x06,
          GET_PROVIDER                                          = 0x07,
          GET_PROVIDER_LISTENERS                = 0x08,
          GET_STATE                                                     = 0x09,

          GET_TERM_CONNECTION                   = 0x0a,
          GET_TERMINAL                                          = 0x0b,
          GET_TERMINALS                                 = 0x0c,

          NUM_ADDRESSES                                 = 0x0d,
          NUM_CALLS                                                     = 0x0e,
          NUM_PROVIDER_LISTENERS                = 0x0f,
          NUM_TERMINALS                                 = 0x10,
          REMOVE_PROVIDER_LISTENER              = 0x11,
          SHUTDOWN                                                      = 0x12,
/* ----------------------------- ADDRESS --------------------------------- */
          ADD_ADDRESS_LISTENER                  = 0x20,
          ADD_CALL_LISTENER                             = 0x21,
          CANCEL_FORWARDING                             = 0x22,
          CANCEL_ALL_FORWARDING                 = 0x23,
          GET_ADDRESS_LISTENERS                 = 0x24,
          GET_CALL_LISTENERS                            = 0x25,
          GET_CONNECTIONS                                       = 0x26,
          GET_DONOT_DISTURB                             = 0x27,
          GET_FORWARDING                                        = 0x28,
          GET_MESSAGE_WAITING                   = 0x29,
          GET_NAME                                                      = 0x2a,
          GET_OFFERED_TIMEOUT                   = 0x2b,

          NUM_ADDRESS_LISTENERS                 = 0x2c,
          NUM_CALL_LISTENERS                            = 0x2d,
          NUM_CONNECTIONS                                       = 0x2e,
          NUM_FORWARDS                                          = 0x2f,
          REMOVE_ADDRESS_LISTENER               = 0x30,
          REMOVE_CALL_LISTENER                  = 0x31,
          SET_DONOT_DISTURB                             = 0x32,
          SET_FORWARDING                                        = 0x33,
          SET_MESSAGE_WAITING                   = 0x34,
          SET_OFFERED_TIMEOUT                   = 0x36,

/* ----------------------------- CALL ------------------------------------- */
          ADD_PARTY                                                     = 0x40,
          CONFERENCE                                            = 0x41,
          CONNECT                                                       = 0x42,
          CONNECT_RESULT                                        = 0x43,
          CONSULT                                                       = 0x44,
          DROP                                                          = 0x45,
          DROP_RESULT                                           = 0x46,
          GET_CALLED_ADDRESS                            = 0x47,
          GET_CALLING_ADDRESS                   = 0x48,
          GET_CALLING_TERMINAL                  = 0x49,

          GET_CONF_CONTROLLER                   = 0x4a,
          GET_LAST_REDIRECTED_ADDRESS   = 0x4b,
          GET_TRANSFER_CONTROLLER               = 0x4c,
          NUM_CALLLISTENERS                             = 0x4d,
          SET_CONF_CONTROLLER                   = 0x4e,
          SET_TRANSFER_CONTROLLER               = 0x4f,
          TRANSFER_CON                                     = 0x50,
          TRANSFER_SEL                                     = 0x51,
     CALL_HOLD                   = 0x52,
     CALL_UNHOLD                 = 0x53,
     GET_CODEC_CPU_LIMIT         = 0x54,
     SET_CODEC_CPU_LIMIT         = 0x55,
     GET_CODEC_CPU_COST          = 0x56,
     CODEC_RENEGOTIATE           = 0x57,

/* ----------------------------- CONNECTION ------------------------------- */
          ACCEPT                                                      = 0x60,
          DISCONNECT                                       = 0x61,
          GET_CALL                                                 = 0x62,
          GET_TERM_CONNECTIONS                  = 0x63,
          NUM_TERM_CONNECTIONS                  = 0x64,
          PARK                                                     = 0x65,
          REDIRECT                                                 = 0x66,
          REJECT                                                      = 0x67,
          GET_FROM_FIELD                                   = 0x68,
          GET_TO_FIELD                                     = 0x69,
          GET_SESSION_INFO                              = 0x6a,
/* ----------------------------- TERMCONNECTION --------------------------- */
          ANSWER                                                                = 0x70,
          HOLD                                                          = 0x71,
          UNHOLD                                                                = 0x72,
          PLAY_FILE_NAME                                        = 0x73,
          PLAY_FILE_URL                                 = 0x74,
          START_TONE                                            = 0x75,
          STOP_TONE                                                     = 0x76,
          STOP_PLAY                                                     = 0x77,
          IS_LOCAL                                                      = 0x78,
     CREATE_PLAYER               = 0x79,
     DESTROY_PLAYER              = 0x7A,
     CREATE_PLAYLIST_PLAYER      = 0x7B,
     DESTROY_PLAYLIST_PLAYER     = 0x7C,
/* ----------------------------- TERMINAL --------------------------------- */
          ADD_TERM_LISTENER                             = 0x80,
          GET_COMPONENT                                 = 0x81,
          GET_COMPONENTS                                        = 0x82,
          GET_COMPONENTGROUPS                   = 0x83,
          GET_CONFIG                                            = 0x84,
          GET_TERM_LISTENERS                            = 0x85,

          NUM_COMPONENTS                                        = 0x86,
          NUM_TERM_LISTENERS                            = 0x87,
          PICKUP                                                                = 0x88,
          REMOVE_TERM_LISTENER                  = 0x89,
          TERMINAL_RESULT                                       = 0x8a,
     SET_INBOUND_CODEC_CPU_LIMIT = 0x8b,
/* ----------------------------- PHONEBUTTON --------------------------------- */
          BUTTON_PRESS                                          = 0x90,
          BUTTON_DOWN                                           = 0x91,
          BUTTON_UP                                                     = 0x92,
          BUTTON_GET_INFO                                       = 0x93,
          BUTTON_SET_INFO                                       = 0x94,
          BUTTON_GET_PHONELAMP                  = 0x95,
/* ----------------------------- PHONEHOOKSWITCH --------------------------------- */
          HOOKSWITCH_SET_STATE                  = 0xa0,
          HOOKSWITCH_GET_STATE                  = 0xa1,
          HOOKSWITCH_GET_CALL                   = 0xa2,
/* ----------------------------- PHONELAMP --------------------------------- */
        LAMP_GET_MODE                                           = 0xb0,
        LAMP_GET_SUPPORTED_MODES                = 0xb1,
        LAMP_GET_BUTTON                                 = 0xb2,
        LAMP_SET_MODE                                           = 0xb3,
/* ----------------------------- PHONEDISPALY --------------------------------- */
        DISPLAY_GET_DISPLAY                             = 0xb4,
        DISPLAY_GET_ROWS                                        = 0xb5,
        DISPLAY_GET_COLS                                        = 0xb6,
        DISPLAY_GET_CONTRAST                            = 0xb7,
        DISPLAY_SET_DISPLAY                             = 0xb8,
        DISPLAY_SET_CONTRAST                            = 0xb9,
/* ----------------------------- PHONEMIC --------------------------------- */
        MIC_GET_GAIN                                            = 0xc0,
        MIC_SET_GAIN                                            = 0xc1,
/* ----------------------------- PHONERINGER --------------------------------- */
        RINGER_SET_INFO                                 = 0xc2,
        RINGER_SET_PATTERN                              = 0xc3,
        RINGER_SET_VOLUME                                       = 0xc4,
        RINGER_GET_INFO                                 = 0xc5,
        RINGER_GET_PATTERN                              = 0xc6,
        RINGER_GET_VOLUME                                       = 0xc7,
        RINGER_GET_MAX_PATTERN_INDEX    = 0xc8,
        RINGER_GET_NUMBER_OF_RINGS              = 0xc9,
        RINGER_IS_ON                                            = 0xca,
/* ----------------------------- PHONESPEAKER --------------------------------- */
        SPEAKER_SET_VOLUME                              = 0xcb,
        SPEAKER_GET_VOLUME                              = 0xcc,
        SPEAKER_GET_NOMINAL_VOLUME              = 0xcd,
/* ----------------------------- PHONEGROUP --------------------------------- */
        PHONEGROUP_ACTIVATE                             = 0xd0,
        PHONEGROUP_DEACTIVATE                   = 0xd1,
        PHONEGROUP_GET_COMPONENTS               = 0xd2,
        PHONEGROUP_GET_DESCRIPTION              = 0xd3,
        PHONEGROUP_GET_TYPE                             = 0xd4,
        PHONEGROUP_IS_ACTIVATED                 = 0xd5,

        EXTSPEAKER_SET_VOLUME                   = 0xd6,
        EXTSPEAKER_GET_VOLUME                   = 0xd7,
        EXTSPEAKER_GET_NOMINAL_VOLUME   = 0xd8,

        COMPONENT_RESULT                                        = 0xd9
        };

#ifdef TAOMSG_DEBUG
        static unsigned int mMsgCnt;
#endif
/* ============================ CREATORS ================================== */

        TaoMessage();

        TaoMessage(const UtlString& msgString);
         //:Constructor

        TaoMessage(UtlString& msgString, UtlString delimiter);
         //:Constructor

        TaoMessage(const unsigned char msgType,
                                const unsigned char cmd,
                                const int msgId,
                                TaoObjHandle handle,
                                TaoObjHandle socket,
                                const int argCnt,
                                const UtlString& argList);
         //:Constructor

        TaoMessage(TaoListenerEventMessage& rEventMessage, TaoObjHandle hSocket);

        TaoMessage(const TaoMessage& rTaoMessage);
         //:Copy constructor

        TaoMessage(OsMsg& msgString);
         //:Constructor

        virtual OsMsg* createCopy(void) const;
         //:Create a copy of this msg object (which may be of a derived type)

        virtual
          ~TaoMessage();
         //:Destructor

/* ============================ MANIPULATORS ============================== */

        TaoMessage& operator=(const TaoMessage& rhs);
         //:Assignment operator

        void setCmd(TaoMsgCmds cmd);

        void setSocket(TaoObjHandle handle);

        void setMsgID(unsigned int id);

        void setArgCnt(int cnt);

        void setArgList(UtlString& argList);
        void setArgList(const char* szArgList);

        void setObjHandle(TaoObjHandle handle);

        void setMsgQueueHandle(TaoObjHandle handle);

/* ============================ ACCESSORS ================================= */

        TaoObjHandle getMsgQueueHandle(void) const { return mMessageQueueHandle; };
         //:Return pointer to message queue owner.

        TaoObjHandle getSocket(void) const { return mSocketHandle; };
         //:Return pointer to OsConnectionSocket.

        unsigned int getMsgID(void) const { return mMsgID; };
         //:Return the used size of the message in bytes

        TaoObjHandle getTaoObjHandle(void) const { return mTaoObjHandle; };
         //:Return the mTaoObjHandle object handle

        unsigned int getArgCnt(void) const { return mArgCnt; };
         //:Return the used size of the message in bytes

        const char* getArgList(void) const { return mArgList.data(); };
         //:Return the used size of the message in bytes

        unsigned char getCmd() { return mCmd; };

        void getBytes(UtlString* bytes, ssize_t* length);
        //: Get the bytes for the complete message, using the mHttpMsg's getBytes method.
        // Suitable for streaming or sending over a socket
        //! param: bytes - gets allocated and must be freed
        //! param: length - the length of bytes

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        void createHTTPMsg(TaoMessage& rTaoMessage);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ FUNCTIONS ================================= */
        void serialize();
        // construct mBody with other class members

        void deSerialize();
        // construct class members with mBody

/* ============================ VARIABLES ================================= */

        TaoObjHandle    mTaoObjHandle;  // the PTAPI object associated with the msg
        TaoObjHandle    mSocketHandle;  // socket on the transport
        TaoObjHandle    mMessageQueueHandle;    // message queue owner i fnon zero

        unsigned int mMsgID;                    // the identifier of this msg
        unsigned int mArgCnt;      // the number of arguments in the msg
        UtlString        mArgList;              // argument list
        UtlBoolean    mbDirty;      // Is the message dirty (needs to be reserialized)

        unsigned char mCmd;
#ifdef USE_HTTPMSG
        HttpMessage      mHttpMsg;              // HTTPMessage form of the TaoMessage
#endif
   ssize_t          mBodyLength;
   UtlString mBody;


};

#endif // _TaoMessage_h_
