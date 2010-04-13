/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

public class DefaultJarMessagesSourceContext implements JarMessagesSourceContext {

    private String m_localizationPackageRoot;

    public void setLocalizationPackageRoot(String localiztionPackageRoot) {
        m_localizationPackageRoot = localiztionPackageRoot;
    }

    public String getLocalizationPackageRoot() {
        return m_localizationPackageRoot;
    }
}
