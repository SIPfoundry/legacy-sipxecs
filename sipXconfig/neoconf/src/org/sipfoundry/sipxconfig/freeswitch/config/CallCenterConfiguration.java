/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.ArrayList;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueueContext;
import org.sipfoundry.sipxconfig.callqueue.CallQueue;
import org.sipfoundry.sipxconfig.callqueue.CallQueueAgent;
import org.sipfoundry.sipxconfig.callqueue.CallQueueTier;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.springframework.beans.factory.annotation.Required;

public class CallCenterConfiguration extends AbstractFreeswitchConfiguration {

    private CallQueueContext m_callQueueContext;
    private Collection<CallQueue> m_callQueues;
    private Collection<CallQueueAgent> m_callQueueAgents;
    private Collection<CallQueueTier> m_callQueueTiers = new ArrayList<CallQueueTier>();

    @Required
    public void setCallQueueContext(CallQueueContext callQueueContext) {
        m_callQueueContext = callQueueContext;
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        m_callQueues = m_callQueueContext.getCallQueues();
        m_callQueueAgents = m_callQueueContext.getCallQueueAgents();
        for (CallQueueAgent callqueueagent : m_callQueueAgents) {
            m_callQueueTiers.addAll(callqueueagent.getTiers().getTiers());
        }
        write(writer);
    }

    void write(Writer writer) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("queues", m_callQueues);
        context.put("agents", m_callQueueAgents);
        context.put("tiers", m_callQueueTiers);
        write(writer, context);
    }

    @Override
    protected String getTemplate() {
        return "freeswitch/callcenter.conf.xml.vm";
    }

    @Override
    protected String getFileName() {
        return "autoload_configs/callcenter.conf.xml";
    }

}
