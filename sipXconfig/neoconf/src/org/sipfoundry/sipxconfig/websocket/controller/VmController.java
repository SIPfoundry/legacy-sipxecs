/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.websocket.controller;

import javax.annotation.PostConstruct;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.hz.HzConstants;
import org.sipfoundry.commons.hz.HzVmEvent;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Controller;

import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.ITopic;
import com.hazelcast.core.Message;
import com.hazelcast.core.MessageListener;

@Controller
public class VmController implements MessageListener<HzVmEvent> {
    private static final Log LOG = LogFactory.getLog(VmController.class);

    @Autowired
    private SimpMessagingTemplate m_template;

    private HazelcastInstance m_hzInstance;

    @PostConstruct
    public void init() {
        ITopic<HzVmEvent> topic = m_hzInstance.getTopic(HzConstants.VM_TOPIC);
        topic.addMessageListener(this);
    }

    @Override
    public void onMessage(Message<HzVmEvent> message) {
        HzVmEvent event = message.getMessageObject();
        try {
            m_template.convertAndSendToUser(event.getUserIdTo(), "/topic/receive-vm", event);
        } catch (Exception ex) {
            LOG.error("Cannot route ws packet ", ex);
        }
    }

    @Autowired
    @Required
    public void setHzInstance(HazelcastInstance hzInstance) {
        m_hzInstance = hzInstance;
    }
}
