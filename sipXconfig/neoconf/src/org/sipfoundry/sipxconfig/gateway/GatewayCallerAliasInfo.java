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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;

public class GatewayCallerAliasInfo implements Cloneable {
    private String m_defaultCallerAlias;

    private boolean m_anonymous;

    private boolean m_ignoreUserInfo;

    private boolean m_transformUserExtension;

    private String m_addPrefix;

    private int m_keepDigits;

    private String m_callerId;

    private String m_displayName;

    private String m_urlParameters;

    private boolean m_enableCallerId;

    /**
     * Transforms user extension into from header
     *
     * @param user for which we are transforming extension
     * @return transformed extension which should be used, or null if there is nothing to
     *         transform
     */
    public String getTransformedNumber(User user) {
        if (!m_transformUserExtension) {
            return null;
        }
        String extension = user.getExtension(true);
        if (extension == null) {
            // nothing to transform
            return null;
        }
        if (m_keepDigits > 0) {
            extension = StringUtils.substring(extension, -m_keepDigits);
        }
        if (m_addPrefix != null) {
            extension = m_addPrefix + extension;
        }
        return extension;
    }

    public String getDefaultCallerAlias() {
        return m_defaultCallerAlias;
    }

    public void setDefaultCallerAlias(String defaultCallerAlias) {
        m_defaultCallerAlias = defaultCallerAlias;
    }

    public boolean isAnonymous() {
        return m_anonymous;
    }

    public void setAnonymous(boolean anonymous) {
        m_anonymous = anonymous;
    }

    public boolean isIgnoreUserInfo() {
        return m_ignoreUserInfo;
    }

    public void setIgnoreUserInfo(boolean overwriteUserInfo) {
        m_ignoreUserInfo = overwriteUserInfo;
    }

    public String getAddPrefix() {
        return m_addPrefix;
    }

    public void setAddPrefix(String addPrefix) {
        m_addPrefix = addPrefix;
    }

    public int getKeepDigits() {
        return m_keepDigits;
    }

    public void setKeepDigits(int keepDigits) {
        m_keepDigits = keepDigits;
    }

    public boolean isTransformUserExtension() {
        return m_transformUserExtension;
    }

    public void setTransformUserExtension(boolean transformUserExtension) {
        m_transformUserExtension = transformUserExtension;
    }

    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    public String getCallerId() {
        return m_callerId;
    }

    public void setCallerId(String callerId) {
        m_callerId = callerId;
    }

    public String getDisplayName() {
        return m_displayName;
    }

    public void setDisplayName(String displayName) {
        m_displayName = displayName;
    }

    public String getUrlParameters() {
        return m_urlParameters;
    }

    public void setUrlParameters(String urlParameters) {
        m_urlParameters = urlParameters;
    }

    public boolean isEnableCallerId() {
        return m_enableCallerId;
    }

    public void setEnableCallerId(boolean enableCallerId) {
        m_enableCallerId = enableCallerId;
    }
}
