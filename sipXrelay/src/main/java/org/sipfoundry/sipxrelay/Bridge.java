/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.nio.channels.DatagramChannel;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.log4j.Logger;

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
final class Bridge implements BridgeInterface {
    private static Logger logger = Logger.getLogger(Bridge.class.getPackage().getName());

    ConcurrentSet sessions = new ConcurrentSet(this);

    private boolean started;

   
    private BridgeState state = BridgeState.INITIAL;

    private String id;

    long pakcetsReceived = 0;

    private long creationTime = System.currentTimeMillis();

    private AtomicLong lastPacketTime = new AtomicLong(0);

    private Parity parity;

    long packetsSent = 0;

    
    // ///////////////////////////////////////////////////////////////////////////////////
    // Private methods.
    // ///////////////////////////////////////////////////////////////////////////////////
    void setState(BridgeState newState) {
        BridgeState oldState = this.state;
        this.state = newState;
        if (oldState != newState) {
            DataShuffler.initializeSelectors();
        }

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

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#getId()
     */
    public String getId() {

        return this.id;
    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#start()
     */

    public void start() throws IllegalStateException {
        if (started)
            return;
        else
            started = true;

        this.setState(BridgeState.RUNNING);

    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#stop()
     */
    public void stop() {
        if (logger.isDebugEnabled()) {
            logger.debug("Closing SymBridge : " + this.toString());

        }

        for (Sym sym : this.sessions) {
            sym.close();
        }

        this.setState(BridgeState.TERMINATED);

    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#pause()
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

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#resume()
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

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#getState()
     */
    public BridgeState getState() {
        return this.state;
    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#addSym(org.sipfoundry.sipxbridge.symmitron.Sym)
     */
    public void addSym(SymInterface sym) {
        this.sessions.add((Sym) sym);
        if (sym.getTransmitter() != null) {
            if (this.parity == null) {
                if (sym.getTransmitter().getPort() % 2 == 0) {
                    this.parity = Parity.EVEN;
                } else {
                    this.parity = Parity.ODD;
                }
            } else {
                if (this.parity == Parity.EVEN && sym.getTransmitter().getPort() % 2 != 0) {
                    logger.warn("Unexpected mixing of odd and even parity " + this.parity);
                } else if (sym.getTransmitter().getPort() % 2 != 1) {
                    logger.warn("Unexpected mixing of odd and even parity" + this.parity);
                }
            }
        }

        ((Sym) sym).setBridge(this);
        if (logger.isDebugEnabled()) {
            logger.debug("addSymSession : " + sym);
            logger.debug("addSymSession : " + this);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#removeSym(org.sipfoundry.sipxbridge.symmitron.Sym)
     */
    public void removeSym(SymInterface symSession) {
        try {
            Sym sym = (Sym) symSession;
            logger.debug("RtpBridge: removing RtpSession " + sym.getId());
            this.sessions.remove(sym);

            if (logger.isDebugEnabled()) {
                logger.debug("removeSymSession : " + sym);
                logger.debug("removeSymSession : " + this);
            }
        } catch (RuntimeException ex) {
            logger.error("unexpected exception", ex);
            throw ex;
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#getSyms()
     */
    public Set<Sym> getSyms() {
        return this.sessions;
    }

    public long getLastPacketTime() {
        return this.lastPacketTime.get();
    }

    public long getCreationTime() {
        return this.creationTime;
    }

    /*
     * (non-Javadoc)
     *
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#toString()
     */
    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        retval.append("Bridge = [ ");
        retval.append("id = " + this.getId() );
        for (Sym rtpSession : this.sessions) {
            retval.append("{");
            retval.append(rtpSession);
            retval.append("}");
        }
        retval.append(" ]");
        return retval.toString();
    }

    /**
     * @param lastPacketTime the lastPacketTime to set
     */
    void setLastPacketTime(long lastPacketTime) {
        this.lastPacketTime.set(lastPacketTime);
    }



    Sym getReceiverSym(DatagramChannel datagramChannel) {
        Sym retval = null;
        for (Sym sym : this.sessions ) {
            if ( sym.getReceiver().getDatagramChannel() == datagramChannel ) {
                retval = sym;
                break;
            }
        }
        return retval;
    }

}
