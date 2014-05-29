/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.voicemail;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipxivr.SipxIvrApp;

public class VoiceMail extends SipxIvrApp {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private VmAction m_deposit;
    private VmAction m_retrieve;

    enum NextAction {
        repeat, exit, nextAttendant;
    }

    /**
     * Run Voice Mail for each mailbox until there is nothing left to do.
     * 
     * Keep running the next returned mailbox until there are none left, then exit.
     * 
     */
    @Override
    public void run() {
        VmEslRequestController controller = (VmEslRequestController) getEslRequestController();

        // Wait a bit so audio doesn't start too fast
        controller.sleep(1000);

        String mailboxString = controller.getMailboxString();
        String action = controller.getAction();
        while(mailboxString != null) {
            LOG.info("Starting voicemail for mailbox \""+mailboxString+"\" action=\""+action);

            // personalize locale to cover the welcome message
            User user = controller.getCurrentUser();
            if (user != null)
            {
                String localeString = user.getVmLanguage();
                controller.personalizeLocale(localeString, user);
            }

            mailboxString = voicemail(mailboxString, controller, action);
        }
        LOG.info("Ending voicemail");
    }

    String voicemail(String mailboxString, VmEslRequestController controller, String action) {
        if (action.equals("deposit")) {
            if (!controller.hasValidMailbox()) {
                return null ;
            }

            String result = m_deposit.runAction();
            if (result == null) {
                return null ;
            }

            if (result.equals("retrieve")) {
                controller.resetUser();
            }
            action = result;
        }
        
        if (action.equals("retrieve")) {
            controller.login();
            if (controller.getCurrentUser() == null) {
                return null;
            }
            return m_retrieve.runAction();
        }
        
        return null;
    }

    public void setDepositAction(VmAction deposit) {
        m_deposit = deposit;
    }

    public void setRetrieveAction(VmAction retrieve) {
        m_retrieve = retrieve;
    }

}
