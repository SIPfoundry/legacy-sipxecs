/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

@SuppressWarnings("serial")
public class ConfigException extends RuntimeException {
    public ConfigException(String msg) {
        super(msg);
    }
}
