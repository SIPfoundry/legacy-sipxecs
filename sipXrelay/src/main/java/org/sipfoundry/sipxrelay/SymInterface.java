/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxrelay;



/**
 * A sym session is a pair of transmitter and receiver. Here is the lifecycle of
 * a SymSession:
 * 
 * <pre>
 *  ---&gt; READY ----------PAUSED---&gt; TERMINATED
 * 	create	&circ;	pause        | close
 *          |				 |
 * 			+----------------+
 * 				resume
 * </pre>
 * 
 * The state of the session governs the transmit state of the session. In the
 * <b>PAUSED</b> state no transmissions for this bridge are allowed. The state
 * of a session does not govern heartbeats sent out through that session.
 * 
 * @author M. Ranganathan
 * 
 */
public interface SymInterface {

    /**
     * Get the unique session ID.
     * 
     * @return -- the unique session ID.
     * 
     */
    public String getId();

    /**
     * Get the transmitter Sym.
     * 
     * @return -- the transmitter.
     */
    public SymEndpointInterface getTransmitter();

    /**
     * Get the receiver Sym.
     * 
     * @return -- the receiver Sym half.
     * 
     */
    public SymEndpointInterface getReceiver();

    

}
