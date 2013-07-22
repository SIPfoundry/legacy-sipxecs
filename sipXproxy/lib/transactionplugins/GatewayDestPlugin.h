#ifndef GATEWAY_DEST_PLUGIN_H_INCLUDED
#define GATEWAY_DEST_PLUGIN_H_INCLUDED

#include <net/SipBidirectionalProcessorPlugin.h>
#include <string>
#include "sipdb/GatewayDestDB.h"
#include "sipdb/EntityDB.h"

extern "C" SipBidirectionalProcessorPlugin* getTransactionPlugin(const UtlString& pluginName);

/**
 * This plugin takes care of providing the correct gateway destination for
 * transferring scenarios which use multiple gateways.
 *
 * 1. First, the plugin detects calls which have a gateway as destination and
 * save the gateway's lineid in top Via header to be retrieved in the
 * call response.
 *
 * 2. If a call which had a gateway as destination is successful, the plugin
 * add a new record in GatewayDestDB with information about the call and the
 * gateway id.
 *
 * 3. If the call originator is involved in any call transfer or pickup
 * scenario the plugin makes sure that any INVITE request with Replaces
 * would follow the same gateway
 *
 * @nosubgrouping
 */
class GatewayDestPlugin : public SipBidirectionalProcessorPlugin
{
protected:
  GatewayDestPlugin(
      const UtlString& instanceName, ///< the configured name for this plugin instance
      int priority = 910
  );

public:
  /// destructor
  virtual ~GatewayDestPlugin();

  virtual void initialize();


  /// Read (or re-read) whatever configuration the plugin requires.
  virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                      * parameters for this instance of this plugin. */
               );

  ///
  /// All incoming Sip Messages will be sent to this virtual function.
  /// Plugins that need to manipulate incoming Sip messages must do so here.
  virtual void handleIncoming(SipMessage& message, const char* address, int port);
  /**< This method does two processings:
  * 1. process any 200 OK responses to INVITEs that had a gateway as destination.
  * Processing means verifying if via has sipxecs-lineid field.
  * If sipxecs-lineid is present in via it means call destination is a gateway,
  * and mongo will be update with a record
  * 2. Add location info header with gateway destination in any INVITE requests
  * with Replaces header
  */

  ///
  /// All outgoing Sip Messages will be sent to this virtual function.
  /// Plugins that need to manipulate outgoing Sip messages must do so here.
  virtual void handleOutgoing(SipMessage& message, const char* address, int port);
  /**< This method process any INVITE that has a gateway as destination.
   * Processing means copying the sipxecs-lineid field from the request uri to top Via
   * sipxecs-lineid present in request uri means call destination is a gateway, by adding it to Via
   * we'll get it back in 200 OK and a record will be added to mongo if call is successful.
   */


protected:
  friend SipBidirectionalProcessorPlugin* getTransactionPlugin(const UtlString& pluginName);


  /// Retrieve the location associated with an user from EntityDB
  bool getUserLocation (
     const UtlString& identity, ///< identity of the user
     UtlString& location        ///< location of the user or empty string
     ) const;

private:
  ///Searches the via header for any sipxecs-lineid param and retrieves it
  bool topViaHasLineId(const SipMessage& response, UtlString& lineId);
  /**<
   * @return true if it found a lineid param in via. False otherwise
   */

  /// Adds a record in GatewayDestDB for gateway identified by lineid
  void addDBRecord(const SipMessage& message, const UtlString& lineId);
  /**<
   * Callid, to-tag, from-tag will be extracted from message
   */

  /// Adds a new X-SipX-Location-Info header with info from given record in message
  void addLocationInfo(SipMessage& message, const GatewayDestRecord& record);

  /// Extract callid, to-tag, from-tag from the message
  void extractRecordData(const SipMessage& message, UtlString* callid, UtlString* toTag, UtlString* fromTag) const;
  /**<
   * @note If the given pointers are null the corresponding fields will not be extracted
   */

  void addLineIdToTopVia(SipMessage& message, const UtlString& lineId);

  GatewayDestDB* _gatewayDestDB;
  EntityDB* _entityDB;
  static const unsigned int MongoRecordExpirationTimeout; /// Expiration timeout in seconds for a GatewayDest record added in mongo
};

#endif // GATEWAY_DEST_PLUGIN_H_INCLUDED
