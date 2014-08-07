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
package org.sipfoundry.sipximbot;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.hz.HzImEvent;
import org.sipfoundry.commons.userdb.User;

import com.hazelcast.core.Message;
import com.hazelcast.core.MessageListener;

public class ImMessageListener implements MessageListener<HzImEvent> {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");

    @Override
    public void onMessage(Message<HzImEvent> message) {
        HzImEvent event = message.getMessageObject();
        if (event.getType() == HzImEvent.Type.ADD_MYBUDDY_TO_ROSTER) {
            String userId = event.getUserId();
            User user = FullUsers.INSTANCE.isValidUser(userId);
            LOG.debug("Add mybuddy to ROSTER for: " + userId);
            IMBot.AddToRoster(user);
        }
    }

}
