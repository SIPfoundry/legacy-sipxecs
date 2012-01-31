/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import static org.sipfoundry.commons.mongo.MongoConstants.AUTH_CODE;
import static org.sipfoundry.commons.mongo.MongoConstants.PASSTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;

public class AuthCode extends BeanWithUserPermissions {
    private String m_code;
    private String m_description;

    public String getCode() {
        return m_code;
    }

    public void setCode(String code) {
        m_code = code;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(AUTH_CODE, m_code);
        props.put(UID, getInternalUser().getUserName());
        props.put(PASSTOKEN, getInternalUser().getSipPassword());
        return props;
    }
}
