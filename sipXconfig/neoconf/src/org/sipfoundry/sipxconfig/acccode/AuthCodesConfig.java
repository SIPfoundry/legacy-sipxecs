/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.acccode;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.springframework.beans.factory.annotation.Required;

public class AuthCodesConfig implements ConfigProvider {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/authcodes-00-00";
    private static final String AUTHCODE = "authcode";
    private static final String CODE = "code";
    private AuthCodeManager m_authCodeManager;
    private AuthCodes m_authCodes;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AuthCodes.FEATURE)) {
            return;
        }

        Collection<Location> locations = manager.getFeatureManager()
                .getLocationsForEnabledFeature(AuthCodes.FEATURE);
        if (locations.isEmpty()) {
            return;
        }

        Address fs = manager.getAddressManager().getSingleAddress(FreeswitchFeature.EVENT_ADDRESS);
        Domain domain = manager.getDomainManager().getDomain();
        for (Location location : locations) {
            AuthCodeSettings settings = m_authCodes.getSettings();
            File dir = manager.getLocationDataDirectory(location);
            Writer xml = new FileWriter(new File(dir, "authcodes.xml"));
            try {
                XmlFile config = new XmlFile(xml);
                config.write(getDocument());
            } finally {
                IOUtils.closeQuietly(xml);
            }

            Writer flat = new FileWriter(new File(dir, "sipxacccode.properties.cfdat"));
            try {
                writeConfig(flat, settings, domain, fs.getPort());
            } finally {
                IOUtils.closeQuietly(xml);
            }
        }
    }

    void writeConfig(Writer wtr, AuthCodeSettings settings, Domain domain, int freeswithPort) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.write(settings.getSettings().getSetting("acccode-config"));
        config.write("acccode.sipxchangeDomainName", domain.getName());
        config.write("acccode.realm", domain.getSipRealm());
        config.write("freeswitch.eventSocketPort", freeswithPort);
    }

    public Document getDocument() {
        Document document = XmlFile.FACTORY.createDocument();
        // See authcodes.xsd for "authcodes" element schema
        QName authcodesName = XmlFile.FACTORY.createQName("authcodes", NAMESPACE);
        Element authcodesElement = document.addElement(authcodesName);

        Collection<AuthCode> authCodes = m_authCodeManager.getAuthCodes();
        // See authcodes.xsd for "authcode", "authname" and "authpassword" element schema
        for (AuthCode authCode : authCodes) {
            Element codeElement = authcodesElement.addElement(AUTHCODE);
            codeElement.addAttribute(CODE, authCode.getCode());
            InternalUser internalUser = authCode.getInternalUser();
            codeElement.addElement("authname").setText(internalUser.getUserName());
            codeElement.addElement("authpassword").setText(internalUser.getSipPassword());
        }

        return document;
    }

    @Required
    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    public AuthCodeManager getAuthCodeManager() {
        return m_authCodeManager;
    }

    public void setAuthCodes(AuthCodes authCodes) {
        m_authCodes = authCodes;
    }
}
