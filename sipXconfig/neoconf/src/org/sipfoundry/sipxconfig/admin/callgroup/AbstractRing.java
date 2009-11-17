/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public abstract class AbstractRing extends BeanWithId {
    public static final String TYPE_PROP = "type";

    private static final int DEFAULT_EXPIRATION = 30;
    private static final String FORMAT = "<sip:%s%s%s?expires=%s>;%s";
    private static final String IGNORE_VOICEMAIL_FIELD_PARAM = "sipx-noroute=Voicemail";
    private static final String DISABLE_USERFORWARD_FIELD_PARAM = "sipx-userforward=false";
    private static final String PARAM_DELIMITER = ";";

    private int m_expiration = DEFAULT_EXPIRATION;
    private Type m_type = Type.DELAYED;
    private boolean m_enabled = true;

    public int getExpiration() {
        return m_expiration;
    }

    public void setExpiration(int expiration) {
        m_expiration = expiration;
    }

    public Type getType() {
        return m_type;
    }

    public void setType(Type type) {
        m_type = type;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public static class Type extends Enum {
        public static final Type DELAYED = new Type("If no response");
        public static final Type IMMEDIATE = new Type("At the same time");

        public Type(String name) {
            super(name);
        }

        public static Type getEnum(String type) {
            return (Type) getEnum(Type.class, type);
        }
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(Type.class);
        }
    }

    /**
     * Retrieves the user part of the contact used to calculate contact
     *
     * @return String or object implementing toString method
     */
    protected abstract Object getUserPart();

    static boolean isAor(String userPart) {
        return userPart.indexOf('@') >= 0;
    }

    /**
     * Calculates contact for line or alias. See FORMAT field.
     *
     * @param domain contact domain
     * @param q contact q value
     * @return String representing the contact
     */
    public final String calculateContact(String domain, ForkQueueValue q, boolean appendIgnoreVoicemail,
            boolean userforward, String prefix) {

        StringBuilder userPart = new StringBuilder(StringUtils.defaultString(prefix));
        userPart.append(getUserPart().toString());

        // XCF-963 Allow forwarding to user supplied AORs
        String domainPart = StringUtils.EMPTY;
        if (!isAor(userPart.toString())) {
            domainPart = '@' + domain;
        }

        StringBuilder fieldParams = new StringBuilder();
        if (!userforward) {
            fieldParams.append(PARAM_DELIMITER);
            fieldParams.append(DISABLE_USERFORWARD_FIELD_PARAM);
        }
        if (appendIgnoreVoicemail) {
            fieldParams.append(PARAM_DELIMITER);
            fieldParams.append(IGNORE_VOICEMAIL_FIELD_PARAM);
        }

        StringBuilder urlParams = new StringBuilder(q.getValue(m_type));
        addUrlParams(urlParams);

        return String.format(FORMAT, userPart, domainPart, fieldParams.toString(), m_expiration, urlParams
                .toString());
    }

    protected void addUrlParams(@SuppressWarnings("unused") StringBuilder params) {
    }
}
