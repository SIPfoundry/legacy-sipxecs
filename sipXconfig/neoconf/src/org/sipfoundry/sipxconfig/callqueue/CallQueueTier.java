/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.io.Serializable;

public class CallQueueTier implements Serializable {
    private static final long serialVersionUID = 1L;

    private Integer m_callQueueAgentId;
    private Integer m_callQueueId;
    private Integer m_position = 0;
    private Integer m_level = 0;

    public void setCallQueueAgentId(Integer callQueueAgentId) {
        m_callQueueAgentId = callQueueAgentId;
    }

    public Integer getCallQueueAgentId() {
        return m_callQueueAgentId;
    }

    public void setCallQueueId(Integer callQueueId) {
        m_callQueueId = callQueueId;
    }

    public Integer getCallQueueId() {
        return m_callQueueId;
    }

    public void setPosition(Integer position) {
        m_position = position;
    }

    public Integer getPosition() {
        return m_position;
    }

    public void setLevel(Integer level) {
        m_level = level;
    }

    public Integer getLevel() {
        return m_level;
    }
}
