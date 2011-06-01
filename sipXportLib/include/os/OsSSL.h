//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsSSL_h_
#define _OsSSL_h__

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsMutex.h"
#include "os/OsLogger.h"
#include "openssl/ssl.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;
class UtlSList;

/// Wrapper for the OpenSSL SSL_CTX context structure.
/// This class is responsible for all global policy initialization and
/// enforcement.
class OsSSL
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   /// Construct an SSL Context from which connections are created.
   OsSSL(const char* authorityPath = NULL,   /**< Path to a directory containing trusted
                                              *   certificates files;
                                              * If NULL, compiled-in default is used */
         const char* publicCertificatePath = NULL, /**< Path to certificate file;
                                                    * If NULL, compiled-in default is used */
         const char* privateKeyPath = NULL,  /**< Path to private key file;
                                             * If NULL, compiled-in default is used.
                                             * @note: If publicCertificatePath is NULL, this
                                             *        must also be NULL.
                                             */
         const char* certificateAuthority = NULL  /**< Path to the certificate authority*/
         );

   ~OsSSL();

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /// Set the default public certificate file
   static void setDefaultPublicCertificateFile(const char * certFile);

   /// Set the default private key file
   static void setDefaultPrivateKeyFile(const char* privateKeyFile);

   /// Set the default CA file
   static void setDefaultCertificateAuthority(const char* caFile);

   /// Set the default authority path
   static void setDefaultAuthorityPath(const char*authorityPath);

   /// Enable CA processing.  The default is false;
   static void enableCertificateAuthority(bool enable = true);

   /// Get an SSL server connection handle
   SSL* getServerConnection();

   /// Get an SSL client connection handle
   SSL* getClientConnection();

   /// Release an SSL session handle
   void releaseConnection(SSL*& connection);

   /// Get the validated names for the connection peer.
   static bool peerIdentity( SSL*       connection ///< SSL context from connection to be described
                            ,UtlSList*  altNames   /**< UtlStrings for verfied subjectAltNames
                                                    *   are added to this - caller must free them.
                                                    */
                            ,UtlString* commonName /**< the Subject name is returned here.
                                                    *   this should not be used if there are any
                                                    *   altName values.
                                                    */
                            );
   /**<
    * Usually, the names in the altNames will be easier to parse and use than commonName
    * Either or both of altNames or commonName may be NULL, in which case no names are returned;
    * the return value still indicates the trust relationship with the peer certificate.
    *
    * @NOTE All name values taken from the cert have been changed to all lower case; this is
    *       not completely correct (it should be sensitive to the type), but will do for now.
    *
    * @TODO This should allow selection based on the type of the subjectAltName, and should.
    *       adjust case sensitivity by type.
    *
    * @returns
    * - true if the connection peer is validated by a trusted authority
    * - false if not, in which case no names are returned.
    */

   /// Log SSL connection information
   static void logConnectParams(const OsSysLogFacility facility, ///< callers facility
                                const OsSysLogPriority priority, ///< log priority
                                const char* callerMsg,  ///< Identifies circumstances of connection
                                SSL*  connection  ///< SSL connection to be described
                                );


   /// Log an error resulting from an SSL call, with the SSL error text expanded
   static void logError(const OsSysLogFacility facility, ///< callers facility
                        const OsSysLogPriority priority, ///< how bad was it?
                        const char* callerMsg,  ///< Identifies caller and what failed
                        int errCode             ///< error returned from ssl routine
                        );

   /// Set OpenSSL callbacks for locking and thread id
   void OpenSSL_thread_setup();

   /// Cleanup OpenSSL callbacks
   void OpenSSL_thread_cleanup();

   /// callback for OpenSSL CRYPTO_set_id_callback
   static unsigned long OpenSSL_id_function(void);

   /// callback for OpenSSL CRYPTO_set_locking_callback
   static void OpenSSL_locking_function(int mode, int n, const char *file, int line);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   static bool sInitialized;

   SSL_CTX* mCTX;

   /// Certificate chain validation hook called by openssl
   static int verifyCallback(int valid,            ///< validity so far from openssl
                             X509_STORE_CTX* store ///< certificate information db
                             );
   /**<
    * @returns validity as determined by local policy
    * @note See 'man SSL_CTX_set_verify'
    */

   static OsMutex* spOpenSSL_locks[CRYPTO_NUM_LOCKS];

   /// Disable copy constructor
   OsSSL(const OsSSL& rOsSSL);

   /// Disable assignment operator
   OsSSL& operator=(const OsSSL& rhs);
};

/// A singleton wrapper for OsSSL
class OsSharedSSL
{
  public:

   static OsSSL* get();

  private:

   static OsBSem* spSslLock;
   static OsSSL*  spSharedSSL;

   /// singleton constructor
   OsSharedSSL();

   ~OsSharedSSL();

   /// Disable copy constructor
   OsSharedSSL(const OsSharedSSL& r);

   /// Disable assignment operator
   OsSharedSSL& operator=(const OsSharedSSL& rhs);
};



/* ============================ INLINE METHODS ============================ */

#endif  // _OsSSL_h_
