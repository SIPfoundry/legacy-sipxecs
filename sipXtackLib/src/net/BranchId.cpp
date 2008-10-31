// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <errno.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlSortedListIterator.h"
#include "utl/UtlRegex.h"

#include "net/NetMd5Codec.h"

#include "net/BranchId.h"
#include "net/SipMessage.h"

//#define TEST_LOG

/* ****************************************************************
 * The syntax for a sipXecs branch id is:
 *
 * branchid           ::= cookie "-" uniquepart [ "~" loopdetectkey ]
 * cookie             ::= 3261cookie "-sipXecs"
 * 3261cookie         ::= "z9hG4bK"
 * uniquepart         ::= smCounter signed-branch
 * signed-branch      ::= MD5(smIdSecret smCounter call-id)
 * loopdetectkey      ::= MD5(loop-detect-inputs)
 * loop-detect-inputs ::= (concatenated request URIs for all forks of this
 *                         message, in addr-spec form, sorted lexically)
 *
 * Examples:
 *
 * - With no loop detect key (an unforked request):
 *   z9hG4bK-sipXecs-0000f9b367e05e4caf080a850f722bd5d10d
 *
 * - With a loop detect key (a forked request):
 *   z9hG4bK-sipXecs-0000f9b367e05e4caf080a850f722bd5d10d~cffa2937819b53a8b7cee52b0fcba1f7
 *
 * **************************************************************** */

// CONSTANTS
#define RFC3261_MAGIC_COOKIE_VALUE "z9hG4bK"
const char* BranchId::RFC3261_MAGIC_COOKIE = RFC3261_MAGIC_COOKIE_VALUE;

#ifndef SIPXECS_COOKIE_EXTENSION
#  define SIPXECS_COOKIE_EXTENSION "-sipXecs-"
#endif
const char*        SIPXECS_MAGIC_COOKIE        = RFC3261_MAGIC_COOKIE_VALUE SIPXECS_COOKIE_EXTENSION;
const size_t       SIPXECS_MAGIC_COOKIE_LENGTH = strlen(SIPXECS_MAGIC_COOKIE);

#define SIPXECS_LOOP_KEY_SEPARATOR "~"

/// The (secret) unique value used to sign the uniquepart hash.
UtlString BranchId::smIdSecret;

/// A monotonically increasing value that ensures some input to unique part is always different.
unsigned int BranchId::smCounter;
/// the number of hex digits needed to encode smCounter
#define SEQUENCE_COUNTER_LENGTH 4 

const RegEx SipXBranchRecognizer(
   "^" RFC3261_MAGIC_COOKIE_VALUE SIPXECS_COOKIE_EXTENSION // the fixed cookie values
   "([0-9a-fA-F]{4})"                                   // the sequence counter
   "(" MD5_REGEX ")"                                    // the signed-branch
   "(?:" "\\" SIPXECS_LOOP_KEY_SEPARATOR                   // loop key separator
   "(" MD5_REGEX ")"                                    // the loopdetectkey
   ")?$"
                                 );
   
/// constructor for a client transaction in a User Agent Client
BranchId::BranchId(const SipMessage& message)
{
   unsigned int uniqueCounter = smCounter;
   smCounter++;
   smCounter &= 0xFFFF;
   generateUniquePart(message, uniqueCounter, *this);
   mParentStringValid = true;   
}

/// constructor for a server transaction in a proxy
BranchId::BranchId(const UtlString& existingBranchIdValue)
{
   // store full value in parent UtlString
   append(existingBranchIdValue);
   mParentStringValid = true;   
}

/// constructor for building a client transaction in a proxy
BranchId::BranchId(BranchId&         parentId, ///< the branchid of the server transaction
                   const SipMessage& message   ///< the new message
                   )
{
   unsigned int uniqueCounter = smCounter;
   smCounter++;
   smCounter &= 0xFFFF;
   generateUniquePart(message, uniqueCounter, *this);

   parentId.generateFullValue();

   mLoopDetectionKey = parentId.mLoopDetectionKey;
   if (!mLoopDetectionKey.isNull())
   {
      append(SIPXECS_LOOP_KEY_SEPARATOR);
      append(mLoopDetectionKey);
   }
   
   mParentStringValid = true;
}


/// accessor for the full string form of the value.
const char* BranchId::data()
{
   generateFullValue();
   return this->UtlString::data();
}

/// Equality Operator
bool BranchId::equals(const BranchId& otherBranchId)
{
   generateFullValue();
#  ifdef TEST_LOG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "BranchId::equals(BranchId '%s')\n"
                 "                          '%s'",
                 const_cast<BranchId*>(&otherBranchId)->data(), data());
#  endif
   return 0 == compareTo(otherBranchId);
}

/// Equality Operator
bool BranchId::equals(const UtlString& otherBranchId)
{
   generateFullValue();
#  ifdef TEST_LOG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "BranchId::equals(UtlString '%s')\n"
                 "                           '%s'",
                 otherBranchId.data(), data());
#  endif
   return 0 == compareTo(otherBranchId);
}

/// Was this branch id produced by an RFC 3261 stack?
bool BranchId::isRFC3261(const UtlString& otherBranchId)
{
   return 0 == otherBranchId.index(RFC3261_MAGIC_COOKIE);
}

/// Was the topmost branch id produced by this proxy (or a redundant peer)?
bool BranchId::topViaIsMyBranch(const SipMessage& response)
{
   bool isMyBranch = false;

   UtlString via;
   if (response.getViaFieldSubField(&via, 0))
   {
      UtlString branch;
      if (SipMessage::getViaTag(via.data(), "branch", branch))
      {
         unsigned int counter = 0;
         UtlString    uniqueKey;
         UtlString    loopDetectKey;
         if (parse(branch, counter, uniqueKey, loopDetectKey)) 
         {
            // found something that looks like a sipXecs branch id

            // generate the unique part I would have generated for that counter value
            UtlString generatedKey;
            generateUniquePart(response, counter, generatedKey);

            // if they match, key is mine
            isMyBranch = (0 == generatedKey.compareTo(uniqueKey)); 
         }
         else
         {
            // branch parameter was not built using our syntax
         }
      }
      else
      {
         // no branch parameter - must not be 3261-compliant, so it's not me 
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "BranchId::topViaIsMyBranch - no top branch found");
   }
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "BranchId::topViaIsMyBranch = %s",
                 isMyBranch ? "Is My Branch" : "Is Not My Branch");
   return isMyBranch;
}

/// Record a fork into the loop detection key for this branch.
void BranchId::addFork(const Url& contact)
{
   // ensure that the value is regenerated with this key 
   mParentStringValid = false;

   // extract the requestUri for this fork
   UtlString* contactUri = new UtlString;
   contact.getUri(*contactUri);

   // put the fork requestUri into an ordered list for later hashing
   // to create the loop detection key.
   if (!mForks.contains(contactUri))
   {
      mForks.insert(contactUri);
   }
   else
   {
      delete contactUri; // discard duplicate
   }
}

/// Parse a sipXecs branch id into its component parts.
bool BranchId::parse(const UtlString& branchValue,   ///< input
                     unsigned int&    uniqueCounter, ///< output sequence value
                     UtlString&       uniqueValue,   ///< output
                     UtlString&       loopDetectKey  ///< output
                     )
{
   RegEx recognizer(SipXBranchRecognizer);
   
   bool isAsipXbranch = recognizer.Search(branchValue);
   if (isAsipXbranch)
   {
      UtlString hexCounter;
      recognizer.MatchString(&hexCounter, 1);
      char* endptr;
      // we know this conversion works because the recognizer matched
      uniqueCounter = strtoul(hexCounter.data(), &endptr, 16);

      recognizer.MatchString(&uniqueValue, 2);

      if (!recognizer.MatchString(&loopDetectKey, 3))
      {
         loopDetectKey.remove(0);
      }
   }
   else
   {
      // no match, so this does not look like one of our branch id values
      uniqueCounter = 0 ;
      uniqueValue.remove(0);
      loopDetectKey.remove(0);
   }

   return isAsipXbranch;
}

void BranchId::generateFullValue()
{
   /*
    * If needed, add the loop detect key to the unique value in the parent string
    * This MUST be called before any use of the parent value as a UtlString.
    */
   if (!mParentStringValid)
   {
      mLoopDetectionKey.remove(0);
      
      UtlString* contact;
      UtlSortedListIterator contacts(mForks);

      NetMd5Codec loopDetectKey;
   
      while((contact = dynamic_cast<UtlString*>(contacts())))
      {
         loopDetectKey.hash(*contact);
      }
      loopDetectKey.appendHashValue(mLoopDetectionKey);

      // get the unique part of the existing value
      unsigned int existingCounter;
      UtlString    existingUniquePart;
      UtlString    oldLoopKey;
      if (parse(*this, existingCounter, existingUniquePart, oldLoopKey))
      {
      
         // rebuild the parent string with the existing unique part and the new key
         remove(0);
         append(SIPXECS_MAGIC_COOKIE);
         char hexCounter[5];
         sprintf(hexCounter, "%04x", existingCounter & 0xFFFF);
         append(hexCounter);
         append(existingUniquePart);
         if (!mLoopDetectionKey.isNull())
         {
            append(SIPXECS_LOOP_KEY_SEPARATOR);
            append(mLoopDetectionKey);
         }
      
         mParentStringValid = true;
      }
   }
}


/// Does this message contain a loop detection key equivalent to this branch?
unsigned int BranchId::loopDetected(const SipMessage& message)
{
   unsigned int loopDetectedWithHop = 0;

   generateFullValue(); // make sure that mLoopDetectionKey has been computed
   if (!mLoopDetectionKey.isNull())
   {
      int viaNumber;
      UtlString via;
      for (viaNumber = 0;
           !loopDetectedWithHop && message.getViaFieldSubField(&via, viaNumber);
           viaNumber++
           )
      {
         UtlString branch;
         if (SipMessage::getViaTag(via.data(), "branch", branch))
         {
            unsigned int counter = 0;
            UtlString    uniqueKey;
            UtlString    loopDetectKey;
            if (parse(branch, counter, uniqueKey, loopDetectKey)) 
            {
               // found something that looks like a sipXecs branch id

               // first compare the loop detection key (faster than recomputing the unique key)
               if (0 == mLoopDetectionKey.compareTo(loopDetectKey,UtlString::matchCase))
               {
                  // generate the unique part I would have generated for that counter value
                  UtlString generatedKey;
                  generateUniquePart(message, counter, generatedKey);

                  // extract just the md5 signature from the unique part
                  UtlString generatedSignature;
                  generatedSignature.append(generatedKey,
                                            SIPXECS_MAGIC_COOKIE_LENGTH+4, UTLSTRING_TO_END);
                  
                  // if they match, key is mine
                  if (0 == generatedSignature.compareTo(uniqueKey))
                  {
                     loopDetectedWithHop = viaNumber+1;
                  }
               }
               else
               {
                  // loop detection key did not match
               }
            }
            else
            {
               // branch parameter was not built using our syntax
            }
         }
         else
         {
            // no branch parameter - must not be 3261-compliant, so it's not me 
         }
      }
   }
   else
   {
      // this request is not forking, so there is no mLoopDetectionKey to compare
   }

   return loopDetectedWithHop;
}

/// Initialize the secret value used to sign values.
void BranchId::setSecret(const UtlString& secret /**< used as input to sign the branch-id value.
                                                  * This should be chosen such that it:
                                                  * is hard for an attacker to guess (includes at
                                                  * least 32 bits of cryptographicaly random data)
                                                  * ideally, is the same in replicated proxies
                                                  */
                         )
{
   if (!smIdSecret.isNull()
       && smIdSecret.compareTo(secret) != 0
       )
   {
      OsSysLog::add(FAC_SIP, PRI_NOTICE,
                    "BranchId::setSecret reset identifier key;"
                    " previously generated branch ids will not be recognized as local."
                    );
      smIdSecret.remove(0);
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "BranchId::setSecret set identifier key."
                 );
   smIdSecret.append(secret);
}


void BranchId::generateUniquePart(const SipMessage& message,
                                  unsigned int uniqueCounter,
                                  UtlString& uniqueValue
                                  )
{
   uniqueValue.remove(0);
   
   uniqueValue.append(SIPXECS_MAGIC_COOKIE); // make it easy to see we did this one

   // build up the unique part of the branch id by hashing
   //  - a value unique to this call
   //  - a value unique to this host
   //  - a monotonically increasing counter
   NetMd5Codec branchSignature;
   branchSignature.hash(smIdSecret);

   char hexCounter[5];
   sprintf(hexCounter, "%04x", uniqueCounter & 0xFFFF);
   uniqueValue.append(hexCounter);
   
   branchSignature.hash(&uniqueCounter, sizeof(uniqueCounter));
   
   UtlString callValue; // make this branch unique to this call
   message.getCallIdField(&callValue);
   branchSignature.hash(callValue);
   
   branchSignature.appendHashValue(uniqueValue);
}

/// destructor
BranchId::~BranchId()
{
   mForks.destroyAll();
};
