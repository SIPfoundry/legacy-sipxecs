package org.sipfoundry.sipxrelay;

public interface SymmitronResetHandler {
    
    
    /**
     * Reset all resources connected to the given client handle
     * (typically send BYE to all b2bua with this server handle).
     * @param serverHandle
     */
    public void reset(String serverHandle) ;

}
