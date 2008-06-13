/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.util.Random;
import java.util.Set;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.BridgeState;

/**
 * Manages the pairing of Rtp pipes. One side belongs to the Lan the other side terminates the
 * ITSP. There is a data shuffler that reads from one end and writes to the other side of the
 * bridge. This method can potentially benifit from a kernel level copy such as ip-tables.
 * 
 * 
 * 
 * 
 * @author M. Ranganathan <mranga@pingtel.com>
 * 
 */
public class Bridge {
    private static Logger logger = Logger.getLogger(Bridge.class);

    ConcurrentSet sessions = new ConcurrentSet(this);

    private boolean started;

    private static Thread dataShufflerThread;

    private static DataShuffler dataShuffler;

    private BridgeState state = BridgeState.INITIAL;

   

    private String id;

    long processingCount = 0;

    private long creationTime = System.currentTimeMillis();

    long lastPacketTime;

    private Parity parity;

    static {
        dataShuffler = new DataShuffler();
        dataShufflerThread = new Thread(dataShuffler);
        dataShufflerThread.start();
    }

    // ///////////////////////////////////////////////////////////////////////////////////
    // Private methods.
    // ///////////////////////////////////////////////////////////////////////////////////
    void setState(BridgeState newState) {
        this.state = newState;

    }

    // /////////////////////////////////////////////////////////////////////////////////
    // Constructors.
    // /////////////////////////////////////////////////////////////////////////////////

    /**
     * Default constructor.
     */
    public Bridge() {
        id = "bridge:" + Long.toString(Math.abs(new Random().nextLong()));
    }

   

    /**
     * Constructor when we are given a media map
     */
    public Bridge(Set<Sym> rtpSession) {
        this();
        this.sessions.addAll(rtpSession);
    }

    // ///////////////////////////////////////////////////////////////////////////////////////////
    // Public Methods.
    // ///////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Get my Id.
     * 
     * @return myId
     */
    public String getId() {

        return this.id;
    }

    /**
     * Starts the data shuffler running
     * 
     * 
     */

    public void start() throws IllegalStateException {
        if (started)
            return;
        else
            started = true;

        this.setState(BridgeState.RUNNING);

    }

    /**
     * Stops the data shuffler from running and kills all syms.
     * 
     */
    public void stop() {
        if (logger.isDebugEnabled()) {
            logger.debug("Closing SymBridge : " + this.toString());
        }
        for (Sym rtpSession : this.sessions) {
            rtpSession.close();
        }

        this.setState(BridgeState.TERMINATED);

    }

    /**
     * Pauses the bridge.
     * 
     * 
     */
    public void pause() {

        if (this.getState() == BridgeState.PAUSED)
            return;
        else if (this.getState() == BridgeState.RUNNING) {
            this.setState(BridgeState.PAUSED);

        } else {
            throw new IllegalStateException("Cannot pause bridge in " + this.getState());
        }

    }

    /**
     * Resumes the operation of the bridge.
     * 
     * 
     */

    public void resume() {
        if (this.getState() == BridgeState.RUNNING)
            return;
        else if (this.getState() == BridgeState.PAUSED) {
            this.setState(BridgeState.RUNNING);

        } else {
            throw new IllegalStateException(" Cannot resume bridge in " + this.getState());
        }
    }

    /**
     * Gets the current state.
     * 
     * @return
     */
    public BridgeState getState() {
        return this.state;
    }

    /**
     * Add a sym to this bridge.
     * 
     * @param sym
     */
    public void addSym(Sym sym) {
        this.sessions.add(sym);
        if (sym.getTransmitter() != null) {
            if (this.parity == null) {
                if (sym.getTransmitter().getPort() % 2 == 0) {
                    this.parity = Parity.EVEN;
                } else {
                    this.parity = Parity.ODD;
                }
            } else {
                if ( this.parity == Parity.EVEN && sym.getTransmitter().getPort() %2  != 0 ) {
                    logger.warn("Unexpected mixing of odd and even parity", new Exception());     
                } else if ( sym.getTransmitter().getPort() %2 != 1 ) {
                    logger.warn("Unexpected mixing of odd and even parity", new Exception());
                }
            }
        }

        sym.setBridge(this);
        if (logger.isDebugEnabled()) {
            logger.debug("addSymSession : " + sym);
            logger.debug("addSymSession : " + this);
        }
    }

    /**
     * Remove a Sym from this bridge.
     * 
     * @param rtpSession
     */
    public void removeSym(Sym rtpSession) {
        logger.debug("RtpBridge: removing RtpSession " + rtpSession.getId());
        this.sessions.remove(rtpSession);

        if (logger.isDebugEnabled()) {
            logger.debug("removeSymSession : " + rtpSession);
            logger.debug("removeSymSession : " + this);
        }
    }

    public Set<Sym> getSyms() {
        return this.sessions;
    }

    public long getLastPacketTime() {
        return this.lastPacketTime;
    }

    public long getCreationTime() {
        return this.creationTime;
    }

    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        retval.append("Bridge = [ \n");
        retval.append("id = " + this.getId() + "\n");
        for (Sym rtpSession : this.sessions) {
            retval.append("{\n");
            retval.append(rtpSession);
            retval.append("}\n");
        }
        retval.append(" ] \n");
        return retval.toString();
    }

}
