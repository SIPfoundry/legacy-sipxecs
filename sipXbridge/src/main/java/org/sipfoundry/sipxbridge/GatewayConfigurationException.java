/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

/**
 * Gateway configuration exception.
 * 
 * @author M. Ranganathan.
 * 
 */
public class GatewayConfigurationException extends RuntimeException {
    public GatewayConfigurationException(String reason) {
        super(reason);
    }

    public GatewayConfigurationException(String reason, Throwable cause) {
        super(reason, cause);
    }

}
