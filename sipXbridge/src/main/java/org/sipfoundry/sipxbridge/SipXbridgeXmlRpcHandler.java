package org.sipfoundry.sipxbridge;

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
public class SipXbridgeXmlRpcHandler {
    
    private static Logger logger = Logger.getLogger(SipXbridgeXmlRpcHandler.class);
    
    /**
     * The RPC handler for sipxbridge.
     */
    public SipXbridgeXmlRpcHandler() {

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
            logger.error("Exception starting bridge " ,ex);

        }
        return Gateway.getState().toString();
    }
    
    
    /**
     * Stop the bridge
     */
    public String stop() {
        try {
            Gateway.stop(); 
        } catch ( Exception ex) {
            logger.error("Exception in stopping bridge",ex);
        }
        return Gateway.getState().toString();
    }

    /**
     * Get the Port Range that is handled by the Bridge.
     * 
     * @return the port range supported by the bridge.
     */
    public PortRange getPortRange() {
        PortRange portRange = new PortRange();
        portRange.setLowBound(Gateway.getRtpPortRangeLowerBound());
        portRange.setHighBound(Gateway.getRtpPortRangeUpperBound());
        return portRange;

    }

}
