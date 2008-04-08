/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

/**
 * State of the bridge.
 * 
 * @author M. Ranganathan.
 * 
 */
public enum BridgeState {
    INITIAL, RUNNING, PAUSED, TERMINATED;

    @Override
    public String toString() {
        if (this == INITIAL) {
            return "INITIAL";
        } else if (this == RUNNING) {
            return "RUNNING";
        } else if (this == PAUSED) {
            return "PAUSED";
        } else if (this == TERMINATED) {
            return "TERMINATED";
        } else {
            return null;
        }
    }

}
