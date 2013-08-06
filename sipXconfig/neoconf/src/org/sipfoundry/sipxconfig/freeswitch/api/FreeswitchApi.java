/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.freeswitch.api;

/**
 * XML/RPC API provided by Freeswitch
 */
public interface FreeswitchApi {
    void reloadxml();
    String conference(String command);
    String g729_status();
    String g729_available();
    String callcenter_config(String command);
}
