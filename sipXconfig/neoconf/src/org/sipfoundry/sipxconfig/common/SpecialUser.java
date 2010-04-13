/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.RandomStringUtils;

public class SpecialUser extends BeanWithId {

    public enum SpecialUserType {
        PARK_SERVER("~~id~park"), MEDIA_SERVER("~~id~media"), ACD_SERVER("~~id~acd"),
        CONFIG_SERVER("~~id~config"), RLS_SERVER("~~id~sipXrls"), REGISTRAR_SERVER("~~id~registrar"),
        APPEAR_AGENT("~~id~sipXsaa"), XMPP_SERVER("~~id~xmpprlsclient"),
        PHONE_PROVISION("~~id~sipXprovision");

        private String m_userName;

        private SpecialUserType(String userName) {
            m_userName = userName;
        }

        public String getUserName() {
            return m_userName;
        }
    }

    private SpecialUserType m_type;
    private String m_sipPassword;

    public SpecialUser() {
        // required by Hibernater
    }

    /**
     * Use when creating a first instance of special user. It generatates random credentials for
     * special users.
     */
    public SpecialUser(SpecialUserType type) {
        m_type = type;
        m_sipPassword = RandomStringUtils.randomAlphanumeric(10);
    }

    public void setType(String type) {
        m_type = SpecialUserType.valueOf(type);
    }

    public void setType(SpecialUserType type) {
        m_type = type;
    }

    public String getType() {
        return m_type.toString();
    }

    public String getUserName() {
        return m_type.getUserName();
    }

    public void setSipPassword(String sipPassword) {
        m_sipPassword = sipPassword;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }
}
