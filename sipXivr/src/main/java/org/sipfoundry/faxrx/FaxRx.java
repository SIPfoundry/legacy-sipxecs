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
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FaxReceive;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrConfiguration;

public class FaxRx {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for AutoAttendant resource bundles keyed by locale
    private static final String RESOURCE_NAME = "org.sipfoundry.attendant.AutoAttendant";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private IvrConfiguration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private ValidUsersXML m_validUsers;
    private String m_localeString;
    private String m_mailboxid;
    private Localization m_loc;

    /**
     * 
     * @param ivrConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI (to determine locale and which Moh id to
     *        use)
     */
    public FaxRx(IvrConfiguration ivrConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
        this.m_mailboxid = parameters.get("mailbox");

        // Look for "locale" parameter
        m_localeString = parameters.get("locale");
        if (m_localeString == null) {
            // Okay, try "lang" instead
            m_localeString = parameters.get("lang");
        }
    }

    /**
     * Load all the needed configuration.
     * 
     */
    void loadConfig() {
        // Load the resources for the given locale.
        m_loc = new Localization(RESOURCE_NAME, m_localeString, s_resourcesByLocale, m_ivrConfig, m_fses);

        // Update the valid users list
        try {
            m_validUsers = ValidUsersXML.update(LOG, true);
        } catch (Exception e) {
            System.exit(1); // If you can't trust validUsers, who can you trust?
        }
    }

    public void run() {

        // run linger only for fax, otherwise we end up with hunged FS session
        m_fses.cmdResponse("linger");

        if (m_loc == null) {
            loadConfig();
        }

        // Wait a bit
        Sleep s = new Sleep(m_fses, 2000);
        s.go();

        receive();
    }

    private void receive() {
        File faxPathName = null;
        FaxReceive faxReceive = null;

        LOG.info("faxrx::Starting mailbox (" + m_mailboxid + ") in locale " + m_loc.getLocale());

        User user = m_validUsers.getUser(m_mailboxid);
        if (user == null) {
            LOG.error("FaxReceive: no user found for mailbox " + m_mailboxid);
            return;
        }

        try {
            faxPathName = File.createTempFile("fax_" + timestamp() + "_", ".tiff");
            new Set(m_fses, "fax_enable_t38_request", "true").go();
            new Set(m_fses, "fax_enable_t38", "true").go();
            faxReceive = new FaxReceive(m_fses, faxPathName.getAbsolutePath());
            faxReceive.go();

        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        finally {
            FaxProcessor.INSTANCE.queueFaxProcessing(user, faxPathName, faxReceive.getRemoteStationId(),
                    m_fses.getVariable("channel-caller-id-name"), m_fses.getVariable("channel-caller-id-number"),
                    faxReceive.faxTotalPages(), faxReceive.rxSuccess(), faxReceive.getResultCode(),
                    faxReceive.getResultText());
        }
    }

    private String timestamp() {
        SimpleDateFormat sdf = new SimpleDateFormat();
        sdf.applyPattern("yyyy-MM-dd-HH-mm-ss");
        return sdf.format(new Date());
    }
}
