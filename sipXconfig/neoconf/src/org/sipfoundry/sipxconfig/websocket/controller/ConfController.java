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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.hz.HzConfEvent;
import org.sipfoundry.commons.hz.HzConstants;
import org.sipfoundry.sipxconfig.hz.HzProvider;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Controller;

import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.ITopic;
import com.hazelcast.core.Message;
import com.hazelcast.core.MessageListener;

@Controller
public class ConfController implements MessageListener<HzConfEvent>, HzProvider {
    private static final Log LOG = LogFactory.getLog(ConfController.class);

    @Autowired
    private SimpMessagingTemplate m_template;

    private HazelcastInstance m_hzInstance;

    @Override
    public void onMessage(Message<HzConfEvent> message) {
        HzConfEvent event = message.getMessageObject();
        try {
            m_template.convertAndSendToUser(event.getUserIdTo(), "/topic/conference", event);
        } catch (Exception ex) {
            LOG.error("Cannot route ws packet ", ex);
        }
    }

    @Override
    public void configuretHzInstance(HazelcastInstance hzInstance) {
        m_hzInstance = hzInstance;
        ITopic<HzConfEvent> topic = m_hzInstance.getTopic(HzConstants.CONF_TOPIC);
        topic.addMessageListener(this);
    }
}
