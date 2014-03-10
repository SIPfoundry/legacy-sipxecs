/**
 *
 *
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
package org.sipfoundry.attendant;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.SipxIvrApp;
import org.sipfoundry.voicemail.mailbox.MailboxManager;

public class LiveAttendantManagement extends SipxIvrApp {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private String m_did;
    private String m_enablePrefix;
    private String m_disablePrefix;
    private MailboxManager m_mailboxManager;

    @Override
    public void run() {
        AaLiveManagementController controller = (AaLiveManagementController) getEslRequestController();
        LOG.debug("LiveAttendantManagement:run for" + controller.getDialedNumber());
        String dialedNumber = controller.getDialedNumber();
        if (StringUtils.equals(dialedNumber, m_did)) {
            controller.play("aalive_please_enter", "0123456789#*");
            dialedNumber = controller.promptForCode();
        }
        boolean enable = false;
        if (dialedNumber.startsWith(m_enablePrefix)) {
            dialedNumber = StringUtils.strip(dialedNumber, m_enablePrefix);
            enable = true;
        } else if (dialedNumber.startsWith(m_disablePrefix)) {
            dialedNumber = StringUtils.strip(dialedNumber, m_disablePrefix);
            enable = false;
        }
        LOG.debug("LiveAttendantManagement:run dialed number" + dialedNumber + " action " + enable);
        boolean result = m_mailboxManager.manageLiveAttendant(dialedNumber, enable);
        if (result) {
            controller.play("aalive_valid", "0123456789#*");
        } else {
            controller.play("aalive_error_hang_up", "0123456789#*");
        }
    }

    public void setDid(String did) {
        m_did = did;
    }

    public void setEnablePrefix(String prefix) {
        m_enablePrefix = prefix;
    }

    public void setDisablePrefix(String prefix) {
        m_disablePrefix = prefix;
    }

    public void setMailboxManager(MailboxManager manager) {
        m_mailboxManager = manager;
    }

}
