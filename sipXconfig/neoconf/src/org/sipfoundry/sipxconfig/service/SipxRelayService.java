/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

public class SipxRelayService extends SipxService {
    public static final String BEAN_ID = "sipxRelayService";

    public String getXmlRpcPort() {
        return getSettingValue("relay-config/xml-rpc-port");
    }
}
