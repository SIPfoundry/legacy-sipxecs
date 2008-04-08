package org.sipfoundry.sipxbridge;

public enum GatewayState {

    INITIALIZING, INITIALIZED, STOPPED;

    @Override
    public String toString() {
        if (this == INITIALIZING) {
            return "Initializing";
        } else if (this == INITIALIZED) {
            return "Initialized";
        } else if (this == STOPPED) {
            return "Stopped";
        } else {
            return null;
        }
    }

}
