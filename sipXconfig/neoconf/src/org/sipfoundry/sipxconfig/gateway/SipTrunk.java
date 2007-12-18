/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.setting.Setting;

public class SipTrunk extends Gateway {
    public static final String BEAN_ID = "gwSipTrunk";

    private static final String ROUTE_SETTING = "trunk/route";

    public SipTrunk() {
    }

    public SipTrunk(GatewayModel model) {
        super(model);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("siptrunk.xml", "commserver");
    }

    @Override
    public String getRoute() {
        return getSettingValue(ROUTE_SETTING);
    }

    public void setRoute(String route) {
        setSettingValue(ROUTE_SETTING, route);
    }
}
