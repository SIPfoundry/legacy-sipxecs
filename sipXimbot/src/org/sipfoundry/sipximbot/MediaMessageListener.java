/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipximbot;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.hz.HzMediaEvent;
import org.sipfoundry.commons.userdb.User;

import com.hazelcast.core.Message;
import com.hazelcast.core.MessageListener;

public class MediaMessageListener implements MessageListener<HzMediaEvent> {
    private static final long serialVersionUID = 1L;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");

    @Override
    public void onMessage(Message<HzMediaEvent> message) {
        HzMediaEvent event = message.getMessageObject();
        User user = FullUsers.INSTANCE.isValidUser(event.getUserIdTo());
        if (user != null) {
            String description = event.getDescription();
            LOG.debug("Send IM message: " + description);
            IMBot.sendIM(user, description);
        }
    }
}
