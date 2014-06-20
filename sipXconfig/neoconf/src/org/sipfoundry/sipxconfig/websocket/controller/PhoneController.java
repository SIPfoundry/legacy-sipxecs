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
import org.sipfoundry.sipxconfig.api.model.PhoneBean;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Controller;

@Controller
public class PhoneController implements DaoEventListener {
    private static final Log LOG = LogFactory.getLog(PhoneController.class);

    @Autowired
    private SimpMessagingTemplate m_template;

    @Override
    public void onDelete(Object entity) {

    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Phone) {
            Phone phone = (Phone) entity;
            try {
                m_template.convertAndSend("/topic/store-phone", PhoneBean.convertPhone(phone));
            } catch (Exception ex) {
                LOG.error("Cannot push packet ", ex);
            }
        }

    }
}
