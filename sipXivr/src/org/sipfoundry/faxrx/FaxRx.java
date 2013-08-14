/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.faxrx;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FaxReceive;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxivr.SipxIvrApp;

public class FaxRx extends SipxIvrApp {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private ValidUsers m_validUsers;
    private FaxProcessor m_faxProcessor;

    @Override
    public void run() {
        // run linger only for fax, otherwise we end up with hunged FS session
        FaxRxEslRequestController controller = (FaxRxEslRequestController) getEslRequestController();
        controller.linger();
        // Wait a bit
        controller.sleep(2000);

        receive(controller);
    }

    private void receive(FaxRxEslRequestController controller) {
        File faxPathName = null;
        FaxReceive faxReceive = null;
        String mailboxId = controller.getMailboxId();
        String locale = controller.getLocaleString();

        LOG.info("faxrx::Starting mailbox (" + mailboxId + ") in locale " + locale);

        User user = m_validUsers.getUser(mailboxId);
        if (user == null) {
            LOG.error("FaxReceive: no user found for mailbox " + mailboxId);
            return;
        }

        try {
            faxPathName = File.createTempFile("fax_" + getTimestamp() + "_", ".tiff");
            controller.invokeSet("fax_enable_t38_request", "true");
            controller.invokeSet("fax_enable_t38", "true");
            faxReceive = controller.receiveFax(faxPathName.getAbsolutePath());

        } catch (IOException e) {
            LOG.error("FaxReceive: failed to receive fax " + e.getMessage());
            return;
        }

        finally {
            m_faxProcessor.queueFaxProcessing(user, faxPathName, faxReceive.getRemoteStationId(),
                    controller.getChannelCallerIdName(), controller.getChannelCallerIdNumber(),
                    faxReceive.faxTotalPages(), faxReceive.rxSuccess(), faxReceive.getResultCode(),
                    faxReceive.getResultText());
        }
    }

    private String getTimestamp() {
        SimpleDateFormat sdf = new SimpleDateFormat();
        sdf.applyPattern("yyyy-MM-dd-HH-mm-ss");
        return sdf.format(new Date());
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    public void setFaxProcessor(FaxProcessor processor) {
        m_faxProcessor = processor;
    }

}
