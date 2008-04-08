package org.sipfoundry.sipxbridge;

/**
 * A sym endpoint is a host-port pair. It can be assigned the role of either a
 * reciever or a sender.
 * 
 * @author mranga
 * 
 */
public interface SymEndpointInterface {

    /**
     * Get the local or remote port where data is received or the remote port
     * where data is sent.
     * 
     * @return -- the port where data is received or sent
     * 
     */
    public int getPort();

    /**
     * Get the IP address ( local or remote ) where data is received or sent.
     * 
     * @return -- the local or remote Ip address.
     */
    public String getIpAddress();

    /**
     * The unique id for this endpoint.
     * 
     * @return -- an identifier for this endpoint.
     */
    public String getId();

}
