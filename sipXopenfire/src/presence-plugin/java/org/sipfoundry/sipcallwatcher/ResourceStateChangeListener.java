/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

public interface ResourceStateChangeListener {
    /*
     * Handler for this component to interact with other components.
     */
    public void handleResourceStateChange(ResourceStateEvent resourceStateEvent) ;

}
