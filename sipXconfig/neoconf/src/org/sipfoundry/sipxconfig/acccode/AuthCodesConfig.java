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
import java.util.Collection;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.springframework.beans.factory.annotation.Required;

public class AuthCodesConfig implements ConfigProvider {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/authcodes-00-00";
    private static final String AUTHCODE = "authcode";
    private static final String CODE = "code";
    // please note: US locale always...
    private AuthCodeManager m_authCodeManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(AuthCodes.FEATURE)) {
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                    AuthCodes.FEATURE);
            for (Location location : locations) {
                File file = new File(manager.getLocationDataDirectory(location), "authcodes.xml.cfdat");
                FileWriter wtr = new FileWriter(file);
                XmlFile xml = new XmlFile(wtr);
                xml.write(getDocument());
            }
        }
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
}
