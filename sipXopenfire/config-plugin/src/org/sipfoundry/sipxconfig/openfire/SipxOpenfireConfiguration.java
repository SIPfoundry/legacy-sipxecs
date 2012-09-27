/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.factory.annotation.Required;

public class SipxOpenfireConfiguration {
    private CoreContext m_coreContext;
    private AddressManager m_addressManager;
    private Openfire m_openfire;
    private DomainManager m_domainManager;
    private VelocityEngine m_velocityEngine;
    private boolean m_enableCallWatcher = true;
    private boolean m_enableParsing = true;

    public void write(Writer writer, Location location) throws IOException {
        VelocityContext context = new VelocityContext();
        OpenfireSettings settings = m_openfire.getSettings();
        Address proxyAddress = m_addressManager.getSingleAddress(ProxyManager.TCP_ADDRESS);
        Address restAddress = m_addressManager.getSingleAddress(RestServer.HTTP_API);
        Address restPublicAddress = m_addressManager.getSingleAddress(RestServer.PUBLIC_HTTP_API);

        context.put("settings", settings);
        String username = XMPP_SERVER.getUserName();
        User user = m_coreContext.getSpecialUser(XMPP_SERVER);
        context.put("username", username);
        context.put("location", location);
        context.put("password", user.getSipPassword());
        context.put("enableCallWatcher", m_enableCallWatcher);
        context.put("enableParsing", m_enableParsing);
        context.put("resource-list", SpeedDial.getResourceListId(username, true));
        context.put("domainName", m_domainManager.getDomainName());
        if (proxyAddress != null) {
            context.put("proxyPort", proxyAddress.getPort());
        }
        context.put("restAddress", restAddress);
        context.put("restPublicAddress", restPublicAddress);
        try {
            m_velocityEngine.mergeTemplate("openfire/sipxopenfire.vm", context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setOpenfire(Openfire openfire) {
        m_openfire = openfire;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setEnableCallWatcher(boolean enableCallWatcher) {
        m_enableCallWatcher = enableCallWatcher;
    }

    public void setEnableParsing(boolean enableParsing) {
        m_enableParsing = enableParsing;
    }        
}
