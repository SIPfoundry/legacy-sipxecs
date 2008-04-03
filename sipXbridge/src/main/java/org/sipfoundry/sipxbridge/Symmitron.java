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
 * This check is done on every interaction with the symitron. This allows the
 * controllers to determine that the symitron has rebooted and drop sessions and
 * similarly for the symitron to detect that controllers rebooted and drop
 * sessions.
 * 
 * <p>
 * We follow the convention of returning a status code with every response. The
 * status codes are simply Strings <b>ERROR</b> and <b>OK</b>. When returning a Map, the
 * status code is referenced by a String <it>status-code</it>. Detailed failure cause
 * is returned by referencing the key <it>error-info</it>. In addition all Maps
 * returned from the Symmitron will contain a special entry called
 * <it>instance-handle</it> that identifies the instance handle corresponding to the
 * current generation of Symmitron.
 * We refer to this map as the <it>standard map</it> in the API docs below. 
 * 
 * <b> The following method Javadocs will only document the information in
 * addition to these standard keys. </b>
 * 
 * <p>
 * We use the java bean naming conventions to generate the key value pairs for
 * java objects. A Sym (AKA RtpSession) is represented by the following keys :
 * 
 * <ul>
 * <li><it>id</it> -- the session Identifier.
 * <li><it>receiver</it> -- a hash map pointing to the recever RtpEndpoint
 * <li><it>transmitter</it> -- a hash map pointing to the transmitter
 * RtpEndpoint.
 * </ul>
 * 
 * <p>
 * An RtpEndpoint is represented by the following Hashmpap:
 * <ul>
 * <li><it>id</it> -- an unique to this endpoint.
 * <li><it>port</it> -- the Integer port where the RtpEndpoint is receiving
 * messages or remote endpoint where the rtp Endpoint is transmitting messages.
 * <li><it>ipAddress</it> -- the String IP Address where the Rtp Endpoint is
 * receiving or transmitting.
 * </ul>
 * 
 * 
 * @author mranga
 * 
 */
public interface Symmitron {

	/*
	 * The following reserved keywords are used to refer to values that are
	 * returned in the Map structures that the Symmitron returns.
	 * The following are returned with every return map.
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
	 * Id of rtp bridge.
	 */
	public static final String BRIDGE_ID = "bridge-id";
	
	/**
	 * references a map that defines an RTP session
	 */

	public static final String RTP_SESSION = "rtp-session";
	
	/**
	 * references collection or RTP Session statistics
	 */
	public static final String RTP_SESSION_STATS = "rtp-session-stats";

	
	/**
	 * The current time of day
	 */
	public static final String CURRENT_TIME_OF_DAY = "current-time-of-day";
	
	/**
	 * The RTP Bridge State. 
	 */
	public static final String BRIDGE_STATE = "bridge-state";
	
	/**
	 * The RTP Session State.
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
	 * Sign in to symmmitron. This allows remote hosts to sign into symmitron.
	 * Returns a cookie that is used in subsequent interactions with the
	 * symmitron. The cookie is used as a generation Identifier.
	 * 
	 * 
	 * 
	 * @param controllerHandle - instance handle of the controller.
	 * 
	 * 
	 * @return -- a standard map. Note that the standard map contains the instance handle of the
	 * 				Symmitron that is used by the client to detect Symmitron reboots.
	 * 
	 */
	public Map<String, Object> signIn(String controllerHandle);

	/**
	 * Allocate a single port binding. This returns with the receiver end
	 * running. This method does not specify an Id.
	 * 
	 * @param controllerHandle - the controller handle that is making this call.
	 * 
	 * 
	 * @return a map containing a key  that references a map
	 *         containing the allocated Rtp Session if any in addition to the
	 *         standard entries.
	 * 
	 */
	public Map<String, Object> createRtpSession(String controllerHandle);

	/**
	 * Get an Rtp Session given its ID if it exists.
	 * 
	 * @param controllerHandle - the controller handle that is making this call.
	 * 
	 * @return a map containing a key "rtp-session" that references a map
	 *         containing the RtpSession. If such a session cannot be found,
	 *         then the entry is not present in the returned map.
	 * 
	 */
	public Map<String, Object> getRtpSession(String controllerHandle, String id);
	
	
	/**
	 * Hold an Rtp Session.
	 * 
	 * @param controllerHandle - the controller handle making this call.
	 * @param rtpSessionId -- the session Id for the rtp session.
	 * 
	 * 
	 * @return a standard map.
	 */
	public Map<String,Object> holdRtpSession(String controllerHandle, String rtpSessionId);
	
	/**
	 * Resume an RTP Session.
	 * 
	 * @param controllerHandle -- the controller handle.
	 * @param rtpSessionId -- the session ID for the rtp session.
	 * 
	 * @return a standard map
	 */
	public Map<String,Object> resumeRtpSession(String controllerHandle, String rtpSessionId);
	
	/**
	 * 
	 * Set the remote session endpoint.
	 *
	 * @param controllerHandle - the controller handle that is making this call.
	 * 
	 * @param rtpSessionId --
	 *            the RtpSession Identifier for the rtp Session for which we
	 *            need to add a remote endpoint.
	 * 
	 * 
	 * @param rtpEndpoint --
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
	 * 
	 * @return - a standard map
	 * 
	 */
	public Map<String, Object> setRemoteRtpEndpoint(String cookie,
			String rtpSessionId, Map<String, Object> rtpEndpoint,
			int keepAliveTime, boolean useLastSentForKeepalive,
			byte[] keepAlivePacketData);
	
	

	/**
	 * Pair or bridge sessions. Note that if the Rtp Sessions were part of another bridge at the
	 * time this method is called, they will be removed from that bridge and added to the new
	 * bridge. This method is useful for consultative call transfers.
	 * 
	 * @param controllerHandle - the controller handle that is making this call.
	 * 
	 * @param rtpSessionId1 -
	 *            the first Rtp Session.
	 * 
	 * @param rtpSessionId2 -
	 *            the second Rtp Session.
	 * 
	 * @return A map containing an rtpBridgeId for the session pair.
	 * 
	 * 
	 */
	public Map<String, Object> pairRtpSessions(String controllerHandle,
			String rtpSessionId1, String rtpSessionId2);
	
	
	/**
	 * Remove an Rtp Session from a bridge.
	 * 
	 * @param controllerHandle -- the controller handle.
	 * 
	 * @param bridgeId -- the RTP bridge id.
	 * @param rtpSessionId -- the session Id of the rtp Session to remove.
	 * 
	 * @return a standard map. 
	 */
	public Map<String,Object> removeRtpSession(String cotrollerHandle, String bridgeId, String rtpSessionId);
	
	/**
	 * Add an RTP session to a bridge.
	 * 
	 * @param controllerHandle -- the controller handle.
	 * @param bridgeId -- the RTP Bridge ID.
	 * @param rtpSessionId -- the rtp session id.
	 * 
	 * @return a standard map
	 */
	public Map<String,Object> addRtpSession(String controllerHandle, String bridgeId, String rtpSessionId);
	

	/**
	 * Change the Remote IP address and port ( where the media gets sent ) for the transmitter of an
	 * RTP session. This method is useful for call transfers.
	 * 
	 * @param controllerHandle -- the controller handle.
	 * @param rtpSessionId -- the rtp session Id.
	 * 
	 * @return - a standard map. 
	 */
	public Map<String,Object> setRemoteIpAddress(String controllerHandle, 
			String rtpSessionId, String ipAddress, int port);
	/**
	 * Create an empty Rtp bridge.
	 * 
	 * @param controllerHandle -- the controller handle.
	 * 
	 * @return  a map containing the allocated bridge ID.
	 */
	public Map<String,Object> createRtpBridge(String controllerHandle);
		
	/**
	 * Start shuffling data on the specified bridge.
	 * 
	 * @param controllerHandle - the controller handle that is making this call.
	 * 
	 * @param bridgeId --
	 *            the bridge Id.
	 * 
	 * @return A standard map.
	 * 
	 */
	public Map<String, Object> startBridge(String controllerHandle, String bridgeId);

	/**
	 * Stop a bridge.
	 * 
	 * @param controllerHandle -- the controller handle making this call.
	 * 
	 * @param bridgeId --
	 *            the bridge id.
	 * 
	 * @return A standard map
	 */
	public Map<String, Object> stopBridge(String controllerHandle, String bridgeId);
	
	/**
	 * Pause the bridge. When you pause the bridge, all data shuffling will stop.
	 * 
	 * @param bridgeId -- the bridge Id.
	 * @param controllerHandle -- the controller handle.
	 * 
	 * @return A standard map.
	 */
	public Map<String,Object> pauseBridge(String controllerHandle, String bridgeId);
	
	/**
	 * Resume bridge operation. When you resume the bridge, data shuffling will resume.
	 * 
	 * @param bridgeId -- the bridge Id.
	 * @param controllerHandle -- the controller Handle.
	 * 
	 * @return A standard map.
	 */
	public Map<String,Object> resumeBridge(String controllerHandle, String bridgeId);
	
	/**
	 * Get Session statistics. 
	 * 
	 * @param controllerHandle -- the controller handle making this call.
	 * 
	 * @param sessionId -- the session id.
	 * 
	 * @return A map containing statistics for the Rtp Session. On successful lookup, a map containing the
	 * following keys will be returned.
	 * <ul>
	 * <li><it>current-time-of-day</it> - the current time of day ( according to symitrons clock )
	 * <li><it>rtp-session</it> - pointer to a hash map containing the rtp session ( note that this contains
	 *							the current receiver port.
	 * <li><it>creation-time</it> - time this session was created.
	 * <li><it>idle-timer-starts</it> - number of times ( from start of session ) that the idle timer kicked in.
	 * <li><it>packets-sent</it> - number of packets sent.
	 * <li><it>packets-received</it> - number of packets received.
	 * </ul>
	 */
	public Map<String,Object> getSessionStatistics(String controllerHandle, String rtpSessionId);
	
	/**
	 * Get the bridge statistics.
	 * 
	 * @param controllerHandle -- the controller handle making this call.
	 * 
	 * @param bridgeId -- the bridge id.
	 * 
	 * 
	 * @return A map containing the statistics for the bridge. A map containing the following statistics is returned.
	 * <ul>
	 * <li><it>current-time-of-day</it> -- current time of day.
	 * <li><it>creation-time</it> -- time this bridge was created.
	 * <li><it>bridge-state</it> -- the bridge state.
	 * <li><it>packets-processed</it> -- the number of packets processed.
	 * <li><it>rtp-session-stats</it> -- a collection of statistics for the individual Rtp Sessions that are part of this Bridge.
	 * </ul>
	 */
	public Map<String, Object> getBridgeStatistics(String controllerHandle, String bridgeId);
	
	

}
