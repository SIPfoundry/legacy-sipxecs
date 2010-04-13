/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxrelay;



/**
 * The bridge implementation class.
 * 
 */
public class BridgeImpl implements BridgeInterface {
    
    private SymmitronClient symmitronClient;
    private String id;
    private BridgeState state;

    protected BridgeImpl ( SymmitronClient symmitronClient ) throws SymmitronException {
        this.symmitronClient  = symmitronClient;
        this.id = this.symmitronClient.createNewBridge();
        this.state = BridgeState.INITIAL;
        
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#addSym(org.sipfoundry.sipxbridge.symmitron.Sym)
     */
     public void addSym(SymInterface sym) {
       symmitronClient.addSym(id, sym);
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#getId()
     */
     public String getId() {
        return this.id;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#getState()
     */
    public BridgeState getState() {
        return state;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#pause()
     */
    public void pause() {
       symmitronClient.pauseBridge(this.id);
       this.state = BridgeState.PAUSED;

    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#removeSym(org.sipfoundry.sipxbridge.symmitron.Sym)
     */
    public void removeSym(SymInterface sym) {
        symmitronClient.removeSym(this.id,sym);
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#resume()
     */
     public void resume() {
       this.symmitronClient.resumeBridge(this.id);
       this.state = BridgeState.RUNNING;

    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#start()
     */
    public void start() throws IllegalStateException {
      this.symmitronClient.startBridge(this.id);
      this.state = BridgeState.RUNNING;

    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxbridge.symmitron.BridgeInterface#stop()
     */
    public void stop() {
        this.symmitronClient.destroyBridge(this.id);
        this.state = BridgeState.TERMINATED; 

    }

}
