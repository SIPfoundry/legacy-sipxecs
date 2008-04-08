/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.Map;

/**
 * The XML RPC interface defining the Symmitron interfaces.
 * 
 * <p>
 * Calling conventions: <br/> Note that for interaction with C++ clients, we
 * will use the standard data type conventions. Following the normal mechanism
 * of XML-RPC. The data will be transmitted as bodies of HTTP requests and
 * responses and formatted in XML. See the following web page for data type
 * conversion from <a href="http://ws.apache.org/xmlrpc/types.html"> Java to XML
 * RPC </a>
 * 
 * <p>
 * Each request and response contains an instance-handle. When a controller
 * signs it, it specifies its non-unique name (such as "sipXproxy"), and a
 * unique instance handle (such as host name and process id). If the Symmitron
 * detects that this is a new instance of that controller (as the handle will be
 * different than before), then any resources currently controlled by the
 * previous instance will be reset and restored back to a known benign state.
 * This check is done on every interaction with the symmitron. This allows the
 * controllers to determine that the symmitron has rebooted and drop sessions
 * and similarly for the symmitron to detect that controllers rebooted and drop
 * sessions.
 * 
 * <p>
 * We use the following abstractions in the API below:
 * 
 * <ul>
 * <li><it>SymEndpoint</it> This is an IP address and port. It represents
 * either a transmitter or a receiver.
 * <li><it>Sym</it> This is a pair of <it>SymEndpoint</it> A
 * Sym has a transmitter and a receiver. A Sym transmits packets
 * to the remote endpoint through its transmitter and receives packets through
 * the receiver.
 * <li><it>Bridge</it> a set of <it>Sym</it> A packet received on a
 * Sym belonging to a given Bridge is potentially transmitted via each of
 * the other Sym that belong to the Bridge. A given Sym can belong
 * to only one Bridge.
 * </ul>
 * 
 * 
 * 
 * <p>
 * We use the java bean naming conventions to generate the key value pairs for
 * java objects. A Sym is represented by the following keys :
 * 
 * <ul>
 * <li><it>id</it> -- the session Identifier.
 * <li><it>receiver</it> -- a hash map pointing to the receiver SymEndpoint
 * <li><it>transmitter</it> -- a hash map pointing to the transmitter
 * SymEndpoint.
 * </ul>
 * 
 * <p>
 * An SymEndpoint is represented by the following Hashmpap:
 * <ul>
 * <li><it>id</it> -- an unique to this endpoint.
 * <li><it>port</it> -- the Integer port where the SymEndpoint is receiving
 * messages or remote endpoint where the sym Endpoint is transmitting messages.
 * <li><it>ipAddress</it> -- the String IP Address where the sym Endpoint is
 * receiving or transmitting.
 * </ul>
 * 
 * <p>
 * We follow the convention of returning a status code with every response. The
 * status codes are simply Strings <b>ERROR</b> and <b>OK</b>. When returning
 * a Map, the status code is referenced by a String <it>status-code</it>.
 * Detailed failure cause is returned by referencing the key <it>error-info</it>.
 * In addition all Maps returned from the Symmitron will contain a special entry
 * called <it>instance-handle</it> that identifies the instance handle
 * corresponding to the current generation of Symmitron. We refer to this map as
 * the <it>standard map</it> in the API docs below.
 * 
 * <b> The following method Javadocs will only document the information in
 * addition to these standard keys. </b>
 * 
 * @author mranga
 * 
 */
public interface Symmitron {

    /*
     * The following reserved keywords are used to refer to values that are
     * returned in the Map structures that the Symmitron returns. The following
     * are returned with every return map.
     */

    /**
     * The status code OK or ERROR.
     */
    public static final String STATUS_CODE = "status-code";

    /**
     * A standard error code -- see definitions below.
     */
    public static final String ERROR_CODE = "error-code";

    /**
     * Detailed error information.
     */
    public static final String ERROR_INFO = "error-info";

    /**
     * Instance handle of Symmitron
     */
    public static final String INSTANCE_HANDLE = "instance-handle";

    /*
     * The following are specific to individual method calls.
     */

    /**
     * Id of sym bridge.
     */
    public static final String BRIDGE_ID = "bridge-id";

    /**
     * references a map that defines an sym
     */

    public static final String SYM_SESSION = "sym";

    /**
     * references collection or sym Session statistics
     */
    public static final String SYM_SESSION_STATS = "sym-stats";

    /**
     * The current time of day
     */
    public static final String CURRENT_TIME_OF_DAY = "current-time-of-day";

    /**
     * The sym Bridge State.
     */
    public static final String BRIDGE_STATE = "bridge-state";

    /**
     * The sym Session State.
     */
    public static final String SESSION_STATE = "session-state";

    /**
     * The number of packets received.
     */
    public static final String PACKETS_RECEIVED = "packets-received";

    /**
     * The number of packets sent.
     */
    public static final String PACKETS_SENT = "packets-sent";

    /**
     * The number of packets processed.
     */
    public static final String PACKETS_PROCESSED = "packets-processed";

    /**
     * Successful return.
     */
    public static final String OK = "ok";

    /**
     * Error return.
     */
    public static final String ERROR = "error";

    /*
     * Error Codes
     */
    public static final int HANDLE_NOT_FOUND = 1;

    public static final int PROCESSING_ERROR = 2;

    public static final int SESSION_NOT_FOUND = 3;

    public static final int ILLEGAL_ARGUMENT = 4;

    public static final int ILLEGAL_STATE = 5;
    
    /**
     * Starting port for Sym allocation.
     * 
     */
    public static final int EVEN = 1;
    public static final int ODD = 2;
  
    
    

    /**
     * Sign in to symmmitron. This allows remote hosts to sign into symmitron.
     * Returns a controllerHandle that is used in subsequent interactions with
     * the symmitron. The controllerHandle is used as a generation Identifier.
     * 
     * 
     * 
     * @param controllerHandle -
     *            instance handle of the controller.
     * 
     * 
     * @return -- a standard map. Note that the standard map contains the
     *         instance handle of the Symmitron that is used by the client to
     *         detect Symmitron reboots.
     * 
     */
    public Map<String, Object> signIn(String controllerHandle);
    
    
    /**
     * Sign out of the symmitron. This allows remote hosts to delete all resources.
     * 
     * @param controllerHandle --
     *          instance handle of the controller.
     *          
     * @return -- a standard map
     */
    public Map<String,Object> signOut(String controllerHandle);

    /**
     * Allocate a set of syms. This returns with the receivers end
     * running. This method does not specify an Id.
     * 
     * @param controllerHandle -
     *            the controller handle that is making this call.
     * 
     * @param count - number of syms to be allocated.
     * 
     * @param parity - 1 Even or 2 Odd the allocated set is a contiguous set of ports.
     * 
     * 
     * 
     * @return a map containing a key that references a map containing the
     *         allocated sym  if any in addition to the standard entries.
     * 
     */
    public Map<String, Object> createSyms(String controllerHandle, int count, int parity);
    
    /**
     * Destroy a sym. This deallocates any resources ( sockets, ports ) that have been reserved
     * for that Sym
     */
    public Map<String,Object> destroySym(String controllerHandle, String symId);
    
   
    /**
     * Get an sym Session given its ID if it exists.
     * 
     * @param controllerHandle -
     *            the controller handle that is making this call.
     *            
     * @param symId - 
     *          the symId that we want to get
     * 
     * @return a map containing a key "sym" that references a map
     *         containing the Sym. If such a session cannot be found,
     *         then the entry is not present in the returned map.
     * 
     */
    public Map<String, Object> getSym(String controllerHandle, String symId);

    /**
     * Hold an sym Session.
     * 
     * @param controllerHandle -
     *            the controller handle making this call.
     * @param symId --
     *            the session Id for the sym.
     * 
     * 
     * @return a standard map.
     */
    public Map<String, Object> holdSym(String controllerHandle,
            String symId);

    /**
     * Resume an sym Session.
     * 
     * @param controllerHandle --
     *            the controller handle.
     * @param symId --
     *            the session ID for the sym.
     * 
     * @return a standard map
     */
    public Map<String, Object> resumeSym(String controllerHandle,
            String symId);

    /**
     * 
     * Set the remote session endpoint.
     * 
     * @param controllerHandle -
     *            the controller handle that is making this call.
     * 
     * @param symId --
     *            the Sym Identifier for the sym Session for which we
     *            need to add a remote endpoint.
     * 
     * 
     * @param symEndpoint --
     *            the remote endpoint to add specified as Name-value pairs.
     * 
     * @param keepAliveTime --
     *            the keep alive time (0 means no keep alive packets).
     * 
     * @param useLastSentForKeepalive --
     *            if true, the last transmitted packet is used for keepalive.
     *            Keepalive will not start until the first transmission.
     * 
     * @param keepAlivePacketData --
     *            the keep alive packet data. Use an empty byte array if sending
     *            an empty UDP Packet for keepalive.
     *            
     * @param autoLearnDestination -- true implies auto discover remote port - send port is based on
     *          remote port of last seen packet ( useful for dealing with NAT reboots ).
     * 
     * 
     * @return - a standard map
     * 
     */
    public Map<String, Object> setDestination(String controllerHandle,
            String symId, Map<String, Object> symEndpoint,
            int keepAliveTime, boolean useLastSentForKeepalive,
            byte[] keepAlivePacketData, boolean autoLearnDestination );

    

    /**
     * Remove a sym from a bridge.
     * 
     * @param controllerHandle --
     *            the controller handle.
     * 
     * @param bridgeId --
     *            the sym bridge id.
     * @param symId --
     *            the session Id of the sym to remove.
     * 
     * @return a standard map.
     */
    public Map<String, Object> removeSym(String cotrollerHandle,
            String bridgeId, String symId);

    /**
     * Add an sym to a bridge.
     * 
     * @param controllerHandle --
     *            the controller handle.
     * @param bridgeId --
     *            the sym Bridge ID.
     * @param symId --
     *            the sym id.
     * 
     * @return a standard map
     */
    public Map<String, Object> addSym(String controllerHandle,
            String bridgeId, String symId);

    

    /**
     * Create an empty sym bridge.
     * 
     * @param controllerHandle --
     *            the controller handle.
     * @param maintainPortParity -- maintain port parity. If this is set to true, then
     *           data received on even ports is forwarded to even port receivers and data
     *           received on odd ports is forwarded to odd port receivers.
     * 
     * @return a map containing the allocated bridge ID.
     */
    public Map<String, Object> createBridge(String controllerHandle, boolean maintainPortParity);

    /**
     * Start shuffling data on the specified bridge.
     * 
     * @param controllerHandle -
     *            the controller handle that is making this call.
     * 
     * @param bridgeId --
     *            the bridge Id.
     * 
     * @return A standard map.
     * 
     */
    public Map<String, Object> startBridge(String controllerHandle,
            String bridgeId);

    /**
     * Stop a bridge.
     * 
     * @param controllerHandle --
     *            the controller handle making this call.
     * 
     * @param bridgeId --
     *            the bridge id.
     * 
     * @return A standard map
     */
    public Map<String, Object> stopBridge(String controllerHandle,
            String bridgeId);

    /**
     * Pause the bridge. When you pause the bridge, all data shuffling will
     * stop.
     * 
     * @param bridgeId --
     *            the bridge Id.
     * @param controllerHandle --
     *            the controller handle.
     * 
     * @return A standard map.
     */
    public Map<String, Object> pauseBridge(String controllerHandle,
            String bridgeId);

    /**
     * Resume bridge operation. When you resume the bridge, data shuffling will
     * resume.
     * 
     * @param bridgeId --
     *            the bridge Id.
     * @param controllerHandle --
     *            the controller Handle.
     * 
     * @return A standard map.
     */
    public Map<String, Object> resumeBridge(String controllerHandle,
            String bridgeId);

    /**
     * Get Sym statistics.
     * 
     * @param controllerHandle --
     *            the controller handle making this call.
     * 
     * @param symId --
     *            the sym id.
     * 
     * @return A map containing statistics for the sym . On successful
     *         lookup, a map containing the following keys will be returned.
     *         <ul>
     *         <li><it>current-time-of-day</it> - the current time of day (
     *         according to symitrons clock )
     *         <li><it>sym</it> - pointer to a hash map containing the
     *         sym ( note that this contains the current receiver port.
     *         <li><it>creation-time</it> - time this session was created.
     *         <li><it>idle-timer-starts</it> - number of times ( from start
     *         of session ) that the idle timer kicked in.
     *         <li><it>packets-sent</it> - number of packets sent.
     *         <li><it>packets-received</it> - number of packets received.
     *         </ul>
     */
    public Map<String, Object> getSymStatistics(String controllerHandle,
            String symId);

    /**
     * Get the bridge statistics.
     * 
     * @param controllerHandle --
     *            the controller handle making this call.
     * 
     * @param bridgeId --
     *            the bridge id.
     * 
     * 
     * @return A map containing the statistics for the bridge. A map containing
     *         the following statistics is returned.
     *         <ul>
     *         <li><it>current-time-of-day</it> -- current time of day.
     *         <li><it>creation-time</it> -- time this bridge was created.
     *         <li><it>bridge-state</it> -- the bridge state.
     *         <li><it>packets-processed</it> -- the number of packets
     *         processed.
     *         <li><it>sym-stats</it> -- a collection of statistics
     *         for the individual sym Sessions that are part of this Bridge.
     *         </ul>
     */
    public Map<String, Object> getBridgeStatistics(String controllerHandle,
            String bridgeId);

}
