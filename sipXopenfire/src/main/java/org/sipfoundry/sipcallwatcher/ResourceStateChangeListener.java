package org.sipfoundry.sipcallwatcher;

public interface ResourceStateChangeListener {
    /*
     * Handler for this component to interact with other components.
     */
    public void handleResourceStateChange(ResourceStateEvent resourceStateEvent) ;

}
