package org.sipfoundry.sipxbridge;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Map;
import java.util.Random;
import java.util.UUID;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.XmlRpcHandler;
import org.apache.xmlrpc.XmlRpcRequest;

/**
 * The SIPXbridge XML RPC handler.
 * 
 * @author M. Ranganathan
 * 
 */
public class SipXbridgeServer implements Symmitron {

	private static Logger logger = Logger.getLogger(SipXbridgeServer.class);

	private HashMap<String, String> handleMap = new HashMap<String, String>();

	/*
	 * Map that pairs the RtpSession id with the RtpSession
	 */
	private HashMap<String, RtpSession> sessionMap = new HashMap<String, RtpSession>();

	/*
	 * Map that pairs the rtpSesion owner with the rtp session
	 */
	private HashMap<String, HashSet<RtpSession>> sessionResourceMap = new HashMap<String, HashSet<RtpSession>>();

	/*
	 * A map that pairs the RtpBridge id with the rtp Bridge.
	 */
	private HashMap<String, RtpBridge> bridgeMap = new HashMap<String, RtpBridge>();

	/*
	 * A map that pairs the owner with the RtpBridge
	 */
	private HashMap<String, HashSet<RtpBridge>> bridgeResourceMap = new HashMap<String, HashSet<RtpBridge>>();

	/*
	 * A map of component name to instance handle.
	 */
	private HashMap<String, String> instanceTable;

	/*
	 * My Instance handle
	 */
	private String myHandle = "sipxbridge:" + new Random().nextLong();

	private Map<String, Object> createErrorMap(int errorCode, String reason) {
		Map<String, Object> retval = new HashMap<String, Object>();
		retval.put(STATUS_CODE, ERROR);
		retval.put(ERROR_CODE, errorCode);
		retval.put(ERROR_INFO, reason);

		retval.put(INSTANCE_HANDLE, myHandle);

		return retval;
	}

	private Map<String, Object> createSuccessMap() {
		Map<String, Object> retval = new HashMap<String, Object>();
		retval.put(STATUS_CODE, OK);
		retval.put(INSTANCE_HANDLE, myHandle);
		return retval;
	}

	private void addRtpSessionResource(String controllerId,
			RtpSession rtpSession) {
		this.sessionMap.put(rtpSession.getId(), rtpSession);
		HashSet<RtpSession> rtpSessions = this.sessionResourceMap
				.get(controllerId);
		if (rtpSessions != null) {
			rtpSessions.add(rtpSession);
		} else {
			rtpSessions = new HashSet<RtpSession>();
			rtpSessions.add(rtpSession);
			this.sessionResourceMap.put(controllerId, rtpSessions);
		}
	}

	private void addRtpBridgeResource(String controllerId, RtpBridge rtpBridge) {
		this.bridgeMap.put(rtpBridge.getId(), rtpBridge);
		HashSet<RtpBridge> bridges = this.bridgeResourceMap.get(controllerId);
		if (bridges != null) {
			bridges.add(rtpBridge);
		} else {
			bridges = new HashSet<RtpBridge>();
			bridges.add(rtpBridge);
			this.bridgeResourceMap.put(controllerId, bridges);
		}
	}

	/**
	 * The RPC handler for sipxbridge.
	 */
	public SipXbridgeServer() {

	}

	/**
	 * Current bridge State.
	 * 
	 * @return the current bridge state.
	 */
	public String getStatus() {
		return Gateway.getState().toString();
	}

	/**
	 * Start the bridge.
	 * 
	 */
	public String start() {
		try {
			Gateway.start();
		} catch (Exception ex) {
			logger.error("Exception starting bridge ", ex);

		}
		return Gateway.getState().toString();
	}

	/**
	 * Stop the bridge
	 */
	public String stop() {
		try {
			Gateway.stop();
		} catch (Exception ex) {
			logger.error("Exception in stopping bridge", ex);
		}
		return Gateway.getState().toString();
	}

	/**
	 * Get the Port Range that is handled by the Bridge.
	 * 
	 * @return the port range supported by the bridge.
	 */
	public Map getRtpPortRange() {

		PortRange portRange = new PortRange();
		portRange.setLowerBound(Gateway.getRtpPortRangeLowerBound());
		portRange.setHigherBound(Gateway.getRtpPortRangeUpperBound());
		return portRange.toMap();

	}

	/**
	 * Check for client reboot.
	 * 
	 * @param controllerHandle -
	 *            a client handle in the form componentName:instanceId
	 */
	public void checkForControllerReboot(String controllerHandle) {
		String[] handleParts = controllerHandle.split(":");
		if (handleParts.length != 2) {
			Map<String, Object> retval = this.createErrorMap(ILLEGAL_ARGUMENT,
					"handle must have the format componentName:instance");
		}
		String componentName = handleParts[0];

		String previousInstance = this.instanceTable.get(componentName);
		if (previousInstance == null) {
			this.instanceTable.put(componentName, controllerHandle);
		} else if (!previousInstance.equals(controllerHandle)) {
			HashSet<RtpBridge> rtpBridges = this.bridgeResourceMap
					.get(previousInstance);
			for (RtpBridge rtpBridge : rtpBridges) {
				rtpBridge.stop();
			}
			this.bridgeResourceMap.remove(previousInstance);
			this.bridgeResourceMap.put(controllerHandle,
					new HashSet<RtpBridge>());
			HashSet<RtpSession> rtpSessions = this.sessionResourceMap
					.get(previousInstance);
			for (RtpSession rtpSession : rtpSessions) {
				rtpSession.close();
			}
			this.sessionResourceMap.remove(previousInstance);
			this.sessionResourceMap.put(controllerHandle,
					new HashSet<RtpSession>());

			this.instanceTable.put(componentName, controllerHandle);
		}
	}

	/**
	 * Sign in a controller.
	 */
	public Map<String, Object> signIn(String remoteHandle) {
		try {
			checkForControllerReboot(remoteHandle);
			return createSuccessMap();
		} catch (Exception ex) {
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}

	}

	
	public Map<String, Object> createRtpSession(String controllerHandle) {
		try {
			this.checkForControllerReboot(controllerHandle);

			Map<String, Object> retval = createSuccessMap();
			RtpSession rtpSession = new RtpSession();
			RtpEndpoint rtpEndpoint = new RtpEndpoint(false);
			rtpSession.setMyEndpoint(rtpEndpoint);
			retval.put(RTP_SESSION, rtpSession.toMap());

			this.addRtpSessionResource(controllerHandle, rtpSession);
			return retval;
		} catch (Exception ex) {
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());

		}

	}

	
	public Map<String, Object> getRtpSession(String controllerHandle,
			String sessionId) {
		try {
			this.checkForControllerReboot(controllerHandle);

			if (this.sessionMap.get(sessionId) != null) {
				Map<String, Object> retval = createSuccessMap();
				RtpSession rtpSession = this.sessionMap.get(sessionId);
				retval.put(RTP_SESSION, rtpSession.toMap());
				return retval;

			} else {
				return createErrorMap(SESSION_NOT_FOUND, "");

			}
		} catch (Exception ex) {
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}
	}


	public Map<String, Object> pairRtpSessions(String controllerHandle,
			String rtpSessionId1, String rtpSessionId2) {
		try {
			this.checkForControllerReboot(controllerHandle);

			RtpSession rtpSession1 = this.sessionMap.get(rtpSessionId1);
			RtpSession rtpSession2 = this.sessionMap.get(rtpSessionId2);
			if (rtpSession1 == null || rtpSession2 == null) {
				return createErrorMap(SESSION_NOT_FOUND,
						"Invalid rtpSession Id");
			}

			RtpBridge rtpBridge = new RtpBridge();
			rtpBridge.addRtpSession(rtpSession1);
			rtpBridge.addRtpSession(rtpSession2);

			this.bridgeMap.put(rtpBridge.getId(), rtpBridge);

			Map<String, Object> retval = createSuccessMap();

			retval.put(BRIDGE_ID, rtpBridge.getId());
			return retval;
		} catch (Exception ex) {
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}

	}

	
	public Map<String, Object> setRemoteRtpEndpoint(String controllerHandle,
			String rtpSessionId, Map<String, Object> rtpEndpointMap,
			int keepAliveTime, boolean useLastSentForKeepalive,
			byte[] keepAlivePacketData) {
		this.checkForControllerReboot(controllerHandle);

		RtpSession rtpSession = this.sessionMap.get(rtpSessionId);
		if (rtpSession == null) {
			return createErrorMap(SESSION_NOT_FOUND, "");
		}
		try {
			Map<String, Object> retval = createSuccessMap();
			String ipAddress = (String) rtpEndpointMap.get("ipAddress");
			int port = (Integer) rtpEndpointMap.get("port");
			RtpEndpoint rtpEndpoint = new RtpEndpoint(true);
			rtpEndpoint.setPort(port);
			rtpEndpoint.setIpAddress(ipAddress);
			rtpEndpoint.setMaxSilence(keepAliveTime);
			rtpEndpoint.setUseLastSentForKeepAlive(useLastSentForKeepalive);
			rtpEndpoint.setKeepalivePayload(keepAlivePacketData);
			return retval;

		} catch (Exception ex) {
			logger.error("Processing Error", ex);

			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}

	}

	
	public Map<String, Object> startBridge(String controllerHandle,
			String bridgeId) {
		try {
			this.checkForControllerReboot(controllerHandle);

			RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
			if (rtpBridge == null) {
				return createErrorMap(SESSION_NOT_FOUND, "");
			}
			rtpBridge.start();
			return createSuccessMap();
		} catch (Exception ex) {
			logger.error("Processing Error", ex);
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}
	}


	public Map<String, Object> stopBridge(String controllerHandle,
			String bridgeId) {
		try {
			this.checkForControllerReboot(controllerHandle);

			RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
			if (rtpBridge == null) {
				return createErrorMap(SESSION_NOT_FOUND,
						"Could not find Bridge for ID " + bridgeId);
			}
			rtpBridge.stop();
			for (RtpSession rtpSession : rtpBridge.getRtpSessionTable()) {
				String key = rtpSession.getId();
				this.sessionMap.remove(key);
			}
			return createSuccessMap();
		} catch (Exception ex) {
			logger.error("Processing Error", ex);
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}
	}

	
	public Map<String, Object> holdRtpSession(String controllerHandle,
			String sessionId) {
		try {
			this.checkForControllerReboot(controllerHandle);
			RtpSession rtpSession = this.sessionMap.get(sessionId);
			if (rtpSession == null) {
				return this.createErrorMap(SESSION_NOT_FOUND,
						"Specified RTP Session was not found " + sessionId);
			}
			if (rtpSession.getTransmitter() == null) {
				return this.createErrorMap(ILLEGAL_STATE,
						"transmitter is not assigned for rtp session "
								+ sessionId);
			}
			rtpSession.getTransmitter().setOnHold(true);
			return this.createSuccessMap();
		} catch (Exception ex) {
			logger.error("Processing Error", ex);
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}
	}

	
	public Map<String, Object> removeRtpSession(String controllerHandle,
			String bridgeId, String rtpSessionId) {
			try {
				this.checkForControllerReboot(controllerHandle);
				RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
				if (rtpBridge == null) {
					return this.createErrorMap(SESSION_NOT_FOUND,
							"Specified RTP Bridge was not found " + bridgeId);
				}

				RtpSession rtpSession = this.sessionMap.get(rtpSessionId);

				if (rtpSession == null) {
					return this.createErrorMap(SESSION_NOT_FOUND,
							"Specified RTP Session was not found " + rtpSessionId);
				}

				rtpBridge.removeRtpSession(rtpSession);
				return this.createSuccessMap();
			} catch (Exception ex) {
				logger.error("Processing Error", ex);
				return createErrorMap(PROCESSING_ERROR, ex.getMessage());
			}
	}

	
	public Map<String, Object> setRemoteIpAddress(String controllerHandle,
			String rtpSessionId, String ipAddress, int port) {
		this.checkForControllerReboot(controllerHandle);
		RtpSession rtpSession = this.sessionMap.get(rtpSessionId);
		if (rtpSession == null) {
			return this.createErrorMap(SESSION_NOT_FOUND,
					"Specified RTP Session was not found " + rtpSessionId);
		}
		if (rtpSession.getTransmitter() == null) {
			return this.createErrorMap(ILLEGAL_STATE,
					"Specified Rtp Session does not have a transmitter assigned to it "
							+ rtpSessionId);
		}
		rtpSession.getTransmitter().setIpAddress(ipAddress);
		rtpSession.getTransmitter().setPort(port);
		return this.createSuccessMap();
	}

	
	public Map<String, Object> addRtpSession(String controllerHandle,
			String bridgeId, String rtpSessionId) {
		try {
			this.checkForControllerReboot(controllerHandle);
			RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
			if (rtpBridge == null) {
				return this.createErrorMap(SESSION_NOT_FOUND,
						"Specified RTP Bridge was not found " + bridgeId);
			}

			RtpSession rtpSession = this.sessionMap.get(rtpSessionId);

			if (rtpSession == null) {
				return this.createErrorMap(SESSION_NOT_FOUND,
						"Specified RTP Session was not found " + rtpSessionId);
			}

			rtpBridge.addRtpSession( rtpSession);
			return this.createSuccessMap();
		} catch (Exception ex) {
			logger.error("Processing Error", ex);
			return createErrorMap(PROCESSING_ERROR, ex.getMessage());
		}
	}

	
	public Map<String, Object> createRtpBridge(String controllerHandle) {

		this.checkForControllerReboot(controllerHandle);

		RtpBridge rtpBridge = new RtpBridge();

		Map<String, Object> retval = this.createSuccessMap();
		retval.put(BRIDGE_ID, rtpBridge.getId());
		return retval;

	}

	
	public Map<String, Object> pauseBridge(String controllerHandle,
			String bridgeId) {
		this.checkForControllerReboot(controllerHandle);
		RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
		if (rtpBridge == null) {
			return this.createErrorMap(SESSION_NOT_FOUND,
					"Bridge corresponding to " + bridgeId + " not found");
		}
		rtpBridge.pause();
		return this.createSuccessMap();
	}

	
	public Map<String, Object> resumeBridge(String controllerHandle,
			String bridgeId) {
		this.checkForControllerReboot(controllerHandle);
		RtpBridge rtpBridge = this.bridgeMap.get(bridgeId);
		if (rtpBridge == null) {
			return this.createErrorMap(SESSION_NOT_FOUND,
					"Bridge corresponding to " + bridgeId + " not found");
		}
		rtpBridge.resume();
		return this.createSuccessMap();
	}

	
	public Map<String, Object> resumeRtpSession(String controllerHandle,
			String sessionId) {
		// TODO Auto-generated method stub
		this.checkForControllerReboot(controllerHandle);
		RtpSession rtpSession = this.sessionMap.get(sessionId);
		if (rtpSession == null) {
			return this.createErrorMap(SESSION_NOT_FOUND,
					"Specified RTP Session was not found " + sessionId);
		}
		if (rtpSession.getTransmitter() == null) {
			return this.createErrorMap(ILLEGAL_STATE,
					"transmitter is not assigned for rtp session " + sessionId);
		}
		rtpSession.getTransmitter().setOnHold(false);
		return this.createSuccessMap();
	}

	
	public Map<String, Object> getSessionStatistics(String controllerHandle,
			String rtpSessionId) {

		return null;
	}

	
	public Map<String, Object> getBridgeStatistics(String controllerHandle,
			String bridgeId) {

		return null;
	}

}
