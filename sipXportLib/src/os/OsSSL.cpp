//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>

// APPLICATION INCLUDES
#include "os/OsSSL.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TEST_DEBUG
static UtlString defaultPublicCertificateFile = SIPX_CONFDIR "/ssl/ssl.crt";
static UtlString defaultPrivateKeyFile        = SIPX_CONFDIR "/ssl/ssl.key";
static UtlString defaultAuthorityPath         = SIPX_CONFDIR "/ssl/authorities";
static UtlString defaultCAFile                = SIPX_CONFDIR "/ssl/ca.crt";
static bool isCertificateAuthorityEnabled        = false;

bool OsSSL::sInitialized = false;
OsMutex* OsSSL::spOpenSSL_locks[];


/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor

OsSSL::OsSSL(const char* authorityPath,
             const char* publicCertificateFile,
             const char* privateKeyPath,
             const char* certificateAuthority
             )
{
   if (!sInitialized)
   {
      // Initialize random number generator before using SSL

      // TODO: this is a bad way to do this - it may need to be fixed.
      //
      //       We should be using better randomness if possible, but at the very
      //       least we should be saving the current rand state in a file so that
      //       is not reset each time.  I think that on a modern Linux, this has
      //       no effect because OpenSSL will use /dev/urandom internally anyway?
      //
      // This needs to be examined.

      /* make a random number and set the top and bottom bits */
      int seed[32];
      for (unsigned int i = 0; i < sizeof(seed)/sizeof(int);i++)
      {
         seed[i] = rand();
      }

      RAND_seed(seed,sizeof(seed));
      SSLeay_add_ssl_algorithms();

      // It is suggested by the OpenSSL group that embedded systems
      // only enable loading of error strings when debugging.
      // Perhaps this should be conditional?
      SSL_library_init();
      SSL_load_error_strings();

      OpenSSL_thread_setup();

      sInitialized = true;
   }

   mCTX = SSL_CTX_new(SSLv23_method());
  

   if (mCTX)
   {
     UtlString caFile;
     if (certificateAuthority != NULL && isCertificateAuthorityEnabled)
       caFile = certificateAuthority;
     if (caFile.isNull() && isCertificateAuthorityEnabled)
       caFile = defaultCAFile;
     
     if (!caFile.isNull() )
     {
           OsSysLog::add(FAC_KERNEL, PRI_INFO ,"OsSSL::_ Enforcing Bundled Certificate: %s", caFile.data());
     }

      if (SSL_CTX_load_verify_locations(mCTX,
                                        caFile.isNull() ? NULL : caFile.data(), // we do not support using a bundled CA file
                                        authorityPath ? authorityPath : defaultAuthorityPath.data())
          > 0)
      {

         if (SSL_CTX_use_certificate_file(mCTX,
                                          publicCertificateFile
                                          ? publicCertificateFile
                                          : defaultPublicCertificateFile.data(),
                                          SSL_FILETYPE_PEM)
             > 0)
         {
            if (SSL_CTX_use_PrivateKey_file(mCTX,
                                            privateKeyPath
                                            ? privateKeyPath
                                            : defaultPrivateKeyFile.data(),
                                            SSL_FILETYPE_PEM)
                > 0)
            {
               if (SSL_CTX_check_private_key(mCTX))
               {
                  OsSysLog::add(FAC_KERNEL, PRI_INFO
                                ,"OsSSL::_ %p CTX %p loaded key pair:\n"
                                "   public  '%s'\n"
                                "   private '%s'"
                                ,this, mCTX,
                                publicCertificateFile
                                ? publicCertificateFile
                                : defaultPublicCertificateFile.data(),
                                privateKeyPath
                                ? privateKeyPath
                                : defaultPrivateKeyFile.data()
                                );

                  // TODO: log our own certificate data

                  // Establish verification rules
                  SSL_CTX_set_verify(mCTX,
                                     SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
                                     verifyCallback
                                     );

                  // disable server connection caching
                  // TODO: Investigate turning this on...
                  SSL_CTX_set_session_cache_mode(mCTX, SSL_SESS_CACHE_OFF);
               }
               else
               {
                  OsSysLog::add(FAC_KERNEL, PRI_ERR,
                                "OsSSL::_ Private key '%s' does not match certificate '%s'",
                                privateKeyPath
                                ? privateKeyPath
                                : defaultPrivateKeyFile.data(),
                                publicCertificateFile
                                ? publicCertificateFile
                                : defaultPublicCertificateFile.data()
                                );
               }
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_ERR,
                             "OsSSL::_ Private key '%s' could not be initialized.",
                             privateKeyPath
                             ? privateKeyPath
                             : defaultPrivateKeyFile.data()
                             );
            }
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                          "OsSSL::_ Public key '%s' could not be initialized.",
                          publicCertificateFile
                          ? publicCertificateFile
                          : defaultPublicCertificateFile.data()
                          );
         }

      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR,
                       "OsSSL::_ SSL_CTX_load_verify_locations failed\n"
                       "    authorityDir:  '%s'",
                       authorityPath ? authorityPath : defaultAuthorityPath.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::_ SSL_CTX_new failed");
   }
}


// Destructor

OsSSL::~OsSSL()
{
   // Since error queue data structures are allocated automatically for new threads,
   // they must be freed when threads are terminated in order to avoid memory leaks.
   ERR_remove_state(0);

   if (mCTX)
   {
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::~ SSL_CTX free %p", mCTX);
      SSL_CTX_free(mCTX);
      mCTX = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

void OsSSL::setDefaultPublicCertificateFile(const char * certFile)
{
  defaultPublicCertificateFile = certFile;
}

void OsSSL::setDefaultPrivateKeyFile(const char* privateKeyFile)
{
  defaultPrivateKeyFile = privateKeyFile;
}

void OsSSL::setDefaultAuthorityPath(const char*authorityPath)
{
  defaultAuthorityPath = authorityPath;
}

void OsSSL::setDefaultCertificateAuthority(const char* caFile)
{
  defaultCAFile = caFile;
}

void OsSSL::enableCertificateAuthority(bool enable)
{
  isCertificateAuthorityEnabled = enable;
}
void OsSSL::OpenSSL_thread_setup()
{
   if (sInitialized)
   {
      return;
   }

   for (int i=0 ; i<CRYPTO_NUM_LOCKS ; i++)
   {
      spOpenSSL_locks[i] = new OsMutex(OsMutex::Q_FIFO);
   }

   // set locking callback to make SSL thread-safe
   CRYPTO_set_locking_callback((void (*)(int,int,const char*, int))OpenSSL_locking_function);

   // set ID callback for linux, where getpid() returns the same for multiple threads
   CRYPTO_set_id_callback(OpenSSL_id_function);
}

void OsSSL::OpenSSL_thread_cleanup()
{
   CRYPTO_set_locking_callback(NULL);
   for (int i=0 ; i<CRYPTO_NUM_LOCKS ; i++)
   {
      delete spOpenSSL_locks[i];
      spOpenSSL_locks[i] = NULL;
   }
}

/// callback for OpenSSL CRYPTO_set_id_callback
unsigned long OsSSL::OpenSSL_id_function(void)
{
   // This implementation assumes that pthread_self() can be cast to long,
   // which is not necessarily true, though it is on Linux.
   // The man page for CRYPTO_set_id_callback says, of platforms on which
   // pthread_self() is not an integer:
   // "This is a bit unusual, and this manual has no cookbook solution for that case."
   return ((unsigned long) pthread_self());
}

/// callback for OpenSSL CRYPTO_set_locking_callback
void OsSSL::OpenSSL_locking_function(int mode, int n, const char *file, int line)
{
   if (mode & CRYPTO_LOCK)
   {
      spOpenSSL_locks[n]->acquire();
   }
   else if (mode & CRYPTO_UNLOCK)
   {
      spOpenSSL_locks[n]->release();
   }
}

/* ============================ ACCESSORS ================================= */

/// Get an SSL server connection handle
SSL* OsSSL::getServerConnection()
{
   SSL* server = SSL_new(mCTX);
   if (server)
   {
#     if SSL_DEBUG
      UtlString ciphers;
      const char* cipher;
      for (int i = 0; cipher = SSL_get_cipher_list(server, i); i++)
      {
         ciphers.append("\n    ");
         ciphers.append(cipher);
      }
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::getServerConnection returning %p%s",
                    server, ciphers.isNull() ? " NO CIPHERS" : ciphers.data());
      // SSL_set_accept_state(server);
      // SSL_set_options(server, SSL_OP_NO_SSLv2);
#     endif
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::getServerConnection SSL_new failed.");
   }

   return server;
}

/// Get an SSL client connection handle
SSL* OsSSL::getClientConnection()
{
   SSL* client = SSL_new(mCTX);
   if (client)
   {
      OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsSSL::getClientConnection returning %p", client);
      // SSL_set_connect_state(client);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::getClientConnection SSL_new failed.");
   }

   return client;
}

/// Release an SSL connection handle
void OsSSL::releaseConnection(SSL*& connection)
{
   if (connection)
   {
      SSL_free(connection);
      ERR_remove_state(0);
      connection = NULL;
   }
}

void OsSSL::logConnectParams(const OsSysLogFacility facility, ///< callers facility
                             const OsSysLogPriority priority, ///< log priority
                             const char* callerMsg,  ///< Identifies circumstances of connection
                             SSL*  connection  ///< SSL connection to be described
                             )
{
   if (connection)
   {
      char* subjectStr = NULL;
      char* issuerStr = NULL;

      UtlString altNames;

      // Extract the subject and issuer information about the peer
      // and the certificate validation result.  Neither of these
      // are meaningful without the other.
      //    (note various dynamically allocated items - freed below)
      int   validity  = SSL_get_verify_result(connection);
      X509* peer_cert = SSL_get_peer_certificate(connection);
      if (peer_cert)
      {
         subjectStr = X509_NAME_oneline(X509_get_subject_name(peer_cert),0,0);
         issuerStr = X509_NAME_oneline(X509_get_issuer_name(peer_cert),0,0);

         // Look for the subjectAltName URI or DNS attributes
         GENERAL_NAMES* names;
         names = (GENERAL_NAMES*)X509_get_ext_d2i(peer_cert, NID_subject_alt_name, NULL, NULL);
         for(int i = 0; i < sk_GENERAL_NAME_num(names); i++)
         {
            GENERAL_NAME* name = sk_GENERAL_NAME_value(names, i);

            switch (name->type)
            {
            case GEN_DNS:
            {
               ASN1_IA5STRING* uri = name->d.uniformResourceIdentifier;
               if (!altNames.isNull())
               {
                  altNames.append(",");
               }
               altNames.append((const char*)(uri->data),uri->length);
            }
            break;

            case GEN_URI:
            {
               ASN1_IA5STRING* uri = name->d.uniformResourceIdentifier;
               if (!altNames.isNull())
               {
                  altNames.append(",");
               }
               altNames.append((const char*)(uri->data),uri->length);
            }
            break;

            default:
               // don't care about any other values
               break;
            }
         }
         sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
      }

      // Get the name of the encryption applied to the connection
      const char* cipher = SSL_get_cipher(connection);

      OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                    "%s SSL Connection:\n"
                    "   status:  %s\n"
                    "   peer:    '%s'\n"
                    "   alt names: %s\n"
                    "   cipher:  '%s'\n"
                    "   issuer:  '%s'",
                    callerMsg,
                    validity == X509_V_OK ? "Verified" : "NOT VERIFIED",
                    subjectStr ? subjectStr : "",
                    altNames.isNull() ? "" : altNames.data(),
                    cipher     ? cipher     : "",
                    issuerStr  ? issuerStr  : ""
                    );

      // Release the various dynamic things
      if (subjectStr)
      {
         OPENSSL_free(subjectStr);
      }
      if (issuerStr)
      {
         OPENSSL_free(issuerStr);
      }
      if (peer_cert)
      {
         X509_free(peer_cert);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsSSL::logConnectParams called by %s with NULL connection",
                    callerMsg
                    );
   }
}

/* Get the validated names for the connection peer.
 *
 * Usually, the names in the altNames will be easier to parse and use than commonName
 * Returns
 * - true if the connection peer is validated by a trusted authority
 * - false if not, in which case no names are returned.
 */
bool OsSSL::peerIdentity( SSL*       connection ///< SSL connection to be described
                         ,UtlSList*  altNames   /**< UtlStrings for verfied subjectAltNames
                                                 *   are added to this - caller must free them.
                                                 */
                         ,UtlString* commonName ///< the Subject name is returned here
                         )
{
   bool peerCertTrusted = false;

#  ifdef TEST_DEBUG
   UtlString debugMsg;
#  endif

   if (altNames)
   {
      altNames->destroyAll();
   }
   if (commonName)
   {
      commonName->remove(0);
   }

   if (connection)
   {
      // Extract the subject and issuer information about the peer
      // and the certificate validation result.  Neither of these
      // are meaningful without the other.
      //    (note various dynamically allocated items - freed below)
      X509* peer_cert = SSL_get_peer_certificate(connection);
      if (peer_cert)
      {
         if (X509_V_OK == SSL_get_verify_result(connection))
         {
            peerCertTrusted = true;

            char* subjectStr = X509_NAME_oneline(X509_get_subject_name(peer_cert),NULL,0);

            // @TODO this should also enforce any extendedKeyUsage limitations

#           ifdef TEST_DEBUG
            debugMsg.append("OsSSL::peerIdentity verified");
#           endif
            if (subjectStr)
            {
               // this should always be true, I think...
               if (commonName)
               {
                  commonName->append(subjectStr);
               }

#              ifdef TEST_DEBUG
               debugMsg.append(" '");
               debugMsg.append(subjectStr);
               debugMsg.append("'");
#              endif
               OPENSSL_free(subjectStr);
            }

            if (altNames)
            {
               // Look for the subjectAltName attributes
               GENERAL_NAMES* names;
               names = (GENERAL_NAMES*)X509_get_ext_d2i(peer_cert, NID_subject_alt_name, NULL, NULL);

               for(int i = 0; i < sk_GENERAL_NAME_num(names); i++)
               {
                  GENERAL_NAME*   name = sk_GENERAL_NAME_value(names, i);
                  ASN1_IA5STRING* nameValue;
                  UtlString*      normalizedName;

                  switch (name->type)
                  {
                  case GEN_DNS:
                  case GEN_URI:
                     nameValue = name->d.uniformResourceIdentifier;
                     normalizedName
                        = new UtlString((const char*)(nameValue->data),nameValue->length);
                     // @TODO: We should parse this value before adjusting the case,
                     //        but that requires doing it at a higher level in the stack
                     //        where we can parse a URL, and we don't yet have selection
                     //        based on type anyway.
                     normalizedName->toLower();
#                    ifdef TEST_DEBUG
                     debugMsg.append(" '");
                     debugMsg.append(*normalizedName);
                     debugMsg.append("'");
#                    endif
                     altNames->append(normalizedName);
                     break;

                  default:
                     // don't care about any other values
                     break;
                  }
               }
               sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
            }
#           ifdef TEST_DEBUG
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "%s", debugMsg.data());
#           endif
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsSSL::peerIdentity peer not validated");
         }

         X509_free(peer_cert);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_WARNING, "OsSSL::peerIdentity no peer certificate");
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT, "OsSSL::peerIdentity called with NULL connection");
   }

   return peerCertTrusted;
}


void OsSSL::logError(const OsSysLogFacility facility,
                     const OsSysLogPriority priority,
                     const char* callerMsg,
                     int errCode)
{
   char sslErrorString[256];
   ERR_error_string_n(errCode, sslErrorString, sizeof(sslErrorString));
   OsSysLog::add(facility, priority,
                 "%s:\n   SSL error: %d '%s'",
                 callerMsg, errCode, sslErrorString
                 );
   int err;              
   while ((err = ERR_get_error()) != 0) 
   {
      ERR_error_string_n(err, sslErrorString, sizeof(sslErrorString));
      OsSysLog::add(facility, priority,
                 "%s:\n   SSL error: %d '%s'",
                 callerMsg, err, sslErrorString
                 );
   }
}

/********************************************************************************/


OsSSL* OsSharedSSL::get()
{
   // critical region to ensure that only one shared ssl context is created
   OsLock lock(*spSslLock);

   if (!spSharedSSL)
   {
      spSharedSSL = new OsSSL();
   }
   return spSharedSSL;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsBSem* OsSharedSSL::spSslLock   = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
OsSSL*  OsSharedSSL::spSharedSSL = NULL;

/* //////////////////////////// PROTECTED ///////////////////////////////// */



/* ============================ FUNCTIONS ================================= */

int OsSSL::verifyCallback(int valid,            // validity so far from openssl
                          X509_STORE_CTX* store // certificate information db
                          )
{
   X509* cert = X509_STORE_CTX_get_current_cert(store);

   if (valid)
   {
      // apply any additional logic we want
   }
   else
   {
      // log the details of why openssl thinks this is not valid
      char issuer[256];
      char subject[256];

      X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer));
      X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsSSL::verifyCallback invalid certificate at depth %d\n"
                    "       error='%s'\n"
                    "       issuer='%s'\n"
                    "       subject='%s'",
                    X509_STORE_CTX_get_error_depth(store),
                    X509_verify_cert_error_string(X509_STORE_CTX_get_error(store)),
                    issuer, subject);
   }

   return valid;
}
