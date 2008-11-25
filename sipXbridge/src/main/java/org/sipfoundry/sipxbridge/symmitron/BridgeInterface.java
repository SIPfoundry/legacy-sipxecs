package org.sipfoundry.sipxbridge.symmitron;

import java.util.Set;


public interface BridgeInterface {

    /**
     * Get my Id.
     * 
     * @return myId
     */
    public abstract String getId();

    /**
     * Starts the data shuffler running
     * 
     * 
     */

    public abstract void start() throws IllegalStateException;

    /**
     * Stops the data shuffler from running and kills all syms.
     * 
     */
    public abstract void stop();

    /**
     * Pauses the bridge.
     * 
     * 
     */
    public abstract void pause();

    /**
     * Resumes the operation of the bridge.
     * 
     * 
     */

    public abstract void resume();

    /**
     * Gets the current state.
     * 
     * @return
     */
    public abstract BridgeState getState();

    /**
     * Add a sym to this bridge.
     * 
     * @param sym
     */
    public abstract void addSym(SymInterface sym);

    /**
     * Remove a Sym from this bridge.
     * 
     * @param sym
     */
    public abstract void removeSym(SymInterface sym);


}
