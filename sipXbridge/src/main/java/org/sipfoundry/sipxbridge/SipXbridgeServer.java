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

    /*
     * Map that pairs the SymSession id with the SymSession
     */
    private HashMap<String, Sym> sessionMap = new HashMap<String, Sym>();

    /*
     * Map that pairs the rtpSesion owner with the rtp session
     */
    private HashMap<String, HashSet<Sym>> sessionResourceMap = new HashMap<String, HashSet<Sym>>();

    /*
     * A map that pairs the RtpBridge id with the rtp Bridge.
     */
    private HashMap<String, Bridge> bridgeMap = new HashMap<String, Bridge>();

    /*
     * A map that pairs the owner with the RtpBridge
     */
    private HashMap<String, HashSet<Bridge>> bridgeResourceMap = new HashMap<String, HashSet<Bridge>>();

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

    private void addSymSessionResource(String controllerId, Sym rtpSession) {
        this.sessionMap.put(rtpSession.getId(), rtpSession);
        HashSet<Sym> rtpSessions = this.sessionResourceMap.get(controllerId);
        if (rtpSessions != null) {
            rtpSessions.add(rtpSession);
        } else {
            rtpSessions = new HashSet<Sym>();
            rtpSessions.add(rtpSession);
            this.sessionResourceMap.put(controllerId, rtpSessions);
        }
    }

    private void addRtpBridgeResource(String controllerId, Bridge rtpBridge) {
        this.bridgeMap.put(rtpBridge.getId(), rtpBridge);
        HashSet<Bridge> bridges = this.bridgeResourceMap.get(controllerId);
        if (bridges != null) {
            bridges.add(rtpBridge);
        } else {
            bridges = new HashSet<Bridge>();
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
            HashSet<Bridge> rtpBridges = this.bridgeResourceMap
                    .get(previousInstance);
            for (Bridge rtpBridge : rtpBridges) {
                rtpBridge.stop();
            }
            this.bridgeResourceMap.remove(previousInstance);
            this.bridgeResourceMap.put(controllerHandle, new HashSet<Bridge>());
            HashSet<Sym> rtpSessions = this.sessionResourceMap
                    .get(previousInstance);
            for (Sym rtpSession : rtpSessions) {
                rtpSession.close();
            }
            this.sessionResourceMap.remove(previousInstance);
            this.sessionResourceMap.put(controllerHandle, new HashSet<Sym>());

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

    public Map<String, Object> createSyms(String controllerHandle, int count,
            int parity) {
        try {
            this.checkForControllerReboot(controllerHandle);

            Map<String, Object> retval = createSuccessMap();
            HashSet<Object> hset = new HashSet<Object>();
            for (int i = 0; i < count; i++) {
                Sym rtpSession = new Sym();
                SymEndpoint rtpEndpoint = new SymEndpoint(false);
                rtpSession.setMyEndpoint(rtpEndpoint);
                hset.add(rtpSession);
                this.addSymSessionResource(controllerHandle, rtpSession);
            }
            retval.put(SYM_SESSION, hset.toArray());

            return retval;
        } catch (Exception ex) {
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());

        }

    }

    public Map<String, Object> getSym(String controllerHandle, String sessionId) {
        try {
            this.checkForControllerReboot(controllerHandle);

            if (this.sessionMap.get(sessionId) != null) {
                Map<String, Object> retval = createSuccessMap();
                Sym rtpSession = this.sessionMap.get(sessionId);
                retval.put(SYM_SESSION, rtpSession.toMap());
                return retval;

            } else {
                return createErrorMap(SESSION_NOT_FOUND, "");

            }
        } catch (Exception ex) {
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }
    }

    public Map<String, Object> pairSyms(String controllerHandle,
            String rtpSessionId1, String rtpSessionId2, boolean parityFlag) {
        try {
            this.checkForControllerReboot(controllerHandle);

            Sym rtpSession1 = this.sessionMap.get(rtpSessionId1);
            Sym rtpSession2 = this.sessionMap.get(rtpSessionId2);
            if (rtpSession1 == null || rtpSession2 == null) {
                return createErrorMap(SESSION_NOT_FOUND,
                        "Invalid rtpSession Id");
            }

            Bridge rtpBridge = new Bridge(parityFlag);
            rtpBridge.addSymSession(rtpSession1);
            rtpBridge.addSymSession(rtpSession2);

            this.bridgeMap.put(rtpBridge.getId(), rtpBridge);

            Map<String, Object> retval = createSuccessMap();

            retval.put(BRIDGE_ID, rtpBridge.getId());
            return retval;
        } catch (Exception ex) {
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }

    }

    public Map<String, Object> setDestination(String controllerHandle,
            String rtpSessionId, Map<String, Object> rtpEndpointMap,
            int keepAliveTime, boolean useLastSentForKeepalive,
            byte[] keepAlivePacketData, boolean autoDiscoverFlag) {
        this.checkForControllerReboot(controllerHandle);

        Sym rtpSession = this.sessionMap.get(rtpSessionId);
        if (rtpSession == null) {
            return createErrorMap(SESSION_NOT_FOUND, "");
        }
        try {
            Map<String, Object> retval = createSuccessMap();
            String ipAddress = (String) rtpEndpointMap.get("ipAddress");
            int port = (Integer) rtpEndpointMap.get("port");
            SymEndpoint rtpEndpoint = new SymEndpoint(true);
            rtpEndpoint.setPort(port);
            rtpEndpoint.setIpAddress(ipAddress);
            rtpEndpoint.setMaxSilence(keepAliveTime);
            rtpEndpoint.setUseLastSentForKeepAlive(useLastSentForKeepalive);
            rtpEndpoint.setKeepalivePayload(keepAlivePacketData);
            rtpEndpoint.setRemoteAddressAutoDiscovery(autoDiscoverFlag);
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

            Bridge rtpBridge = this.bridgeMap.get(bridgeId);
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

            Bridge rtpBridge = this.bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return createErrorMap(SESSION_NOT_FOUND,
                        "Could not find Bridge for ID " + bridgeId);
            }
            rtpBridge.stop();
            for (Sym rtpSession : rtpBridge.getSessionTable()) {
                String key = rtpSession.getId();
                this.sessionMap.remove(key);
            }
            return createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }
    }

    public Map<String, Object> holdSym(String controllerHandle, String sessionId) {
        try {
            this.checkForControllerReboot(controllerHandle);
            Sym rtpSession = this.sessionMap.get(sessionId);
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

    public Map<String, Object> removeSym(String controllerHandle,
            String bridgeId, String rtpSessionId) {
        try {
            this.checkForControllerReboot(controllerHandle);
            Bridge rtpBridge = this.bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Bridge was not found " + bridgeId);
            }

            Sym rtpSession = this.sessionMap.get(rtpSessionId);

            if (rtpSession == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Session was not found " + rtpSessionId);
            }

            rtpBridge.removeSymSession(rtpSession);
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }
    }

    public Map<String, Object> setRemoteIpAddress(String controllerHandle,
            String rtpSessionId, String ipAddress, int port) {
        this.checkForControllerReboot(controllerHandle);
        Sym rtpSession = this.sessionMap.get(rtpSessionId);
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

    public Map<String, Object> addSym(String controllerHandle, String bridgeId,
            String rtpSessionId) {
        try {
            this.checkForControllerReboot(controllerHandle);
            Bridge rtpBridge = this.bridgeMap.get(bridgeId);
            if (rtpBridge == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Bridge was not found " + bridgeId);
            }

            Sym rtpSession = this.sessionMap.get(rtpSessionId);

            if (rtpSession == null) {
                return this.createErrorMap(SESSION_NOT_FOUND,
                        "Specified RTP Session was not found " + rtpSessionId);
            }

            rtpBridge.addSymSession(rtpSession);
            return this.createSuccessMap();
        } catch (Exception ex) {
            logger.error("Processing Error", ex);
            return createErrorMap(PROCESSING_ERROR, ex.getMessage());
        }
    }

    public Map<String, Object> createBridge(String controllerHandle,
            boolean maintainParity) {

        this.checkForControllerReboot(controllerHandle);

        Bridge rtpBridge = new Bridge(maintainParity);

        Map<String, Object> retval = this.createSuccessMap();
        retval.put(BRIDGE_ID, rtpBridge.getId());
        return retval;

    }

    public Map<String, Object> pauseBridge(String controllerHandle,
            String bridgeId) {
        this.checkForControllerReboot(controllerHandle);
        Bridge rtpBridge = this.bridgeMap.get(bridgeId);
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
        Bridge rtpBridge = this.bridgeMap.get(bridgeId);
        if (rtpBridge == null) {
            return this.createErrorMap(SESSION_NOT_FOUND,
                    "Bridge corresponding to " + bridgeId + " not found");
        }
        rtpBridge.resume();
        return this.createSuccessMap();
    }

    public Map<String, Object> resumeSym(String controllerHandle,
            String sessionId) {
        // TODO Auto-generated method stub
        this.checkForControllerReboot(controllerHandle);
        Sym rtpSession = this.sessionMap.get(sessionId);
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

    public Map<String, Object> getSymStatistics(String controllerHandle,
            String rtpSessionId) {

        return null;
    }

    public Map<String, Object> getBridgeStatistics(String controllerHandle,
            String bridgeId) {

        return null;
    }

    public Map<String, Object> signOut(String controllerHandle) {
        HashSet<Bridge> rtpBridges = this.bridgeResourceMap
                .get(controllerHandle);
        for (Bridge rtpBridge : rtpBridges) {
            rtpBridge.stop();
        }
        this.bridgeResourceMap.remove(controllerHandle);

        HashSet<Sym> rtpSessions = this.sessionResourceMap
                .get(controllerHandle);
        for (Sym rtpSession : rtpSessions) {
            rtpSession.close();
        }
        this.sessionResourceMap.remove(controllerHandle);

        return this.createSuccessMap();
    }

    public Map<String, Object> destroySym(String controllerHandle, String symId) {
        this.checkForControllerReboot(controllerHandle);
        if (this.sessionResourceMap.containsKey(symId)) {
            Sym sym = this.sessionMap.get(symId);
            this.sessionResourceMap.remove(symId);
            sym.close();
            Map<String, Object> retval = this.createSuccessMap();
            return retval;
        } else {
            return this.createErrorMap(SESSION_NOT_FOUND,
                    "Sym with the given Id was not found");
        }
    }

}
