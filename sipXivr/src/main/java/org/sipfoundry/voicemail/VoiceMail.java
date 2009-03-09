/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.voicemail;

import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Collect;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.Play;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.Sleep;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;


public class VoiceMail {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for resource bundles keyed by locale
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private org.sipfoundry.sipxivr.Configuration m_ivrConfig;
    private Localization m_localization;
    private FreeSwitchEventSocketInterface m_fses;
    private ResourceBundle m_vmBundle;
    private Configuration m_config;
    private ValidUsersXML m_validUsers;

    private Hashtable<String, String> m_parameters;

    private String m_action; // "deposit" or "retrieve"

    enum NextAction {
        repeat, exit, nextAttendant;
    }

    /**
     * Create a VoiceMail object.
     * 
     * @param ivrConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI (to determine locale and which action
     *        to run)
     */
    public VoiceMail(org.sipfoundry.sipxivr.Configuration ivrConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
        this.m_parameters = parameters;

        // Look for "locale" parameter
        String localeString = parameters.get("locale");
        if (localeString == null) {
            // Okay, try "lang" instead
            localeString = parameters.get("lang");
        }

        // Load the resources for the given localeString
        m_localization = new Localization("org.sipfoundry.voicemail.VoiceMail", localeString, 
                s_resourcesByLocale, ivrConfig);
    }

    /**
     * Load all the needed configuration.
     * 
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. The attendant configuration files are loaded (if they changed since last time), and
     * the ValidUsers (also if they changed).
     * 
     */
    void loadConfig() {

        // Load the Voice Mail configuration
        m_config = Configuration.update(true);

        // Update the valid users list
        m_validUsers = ValidUsersXML.update(true);
    }

    /**
     * Helper function to get the PromptList from the bundle.
     * 
     * @return The appropriate PromptList.
     */
    PromptList getPromptList() {
        PromptList pl = new PromptList(m_localization);
        return pl;
    }

    /**
     * Helper function to get the PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param vars
     * @return The appropriate PromptList.
     */
    PromptList getPromptList(String fragment, String... vars) {
        PromptList pl = getPromptList();
        pl.addFragment(fragment, vars);
        return pl;
    }

    /**
     * Helper function to get a Player with a PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param vars
     * @return
     */
    Play getPlayer(String fragment, String... vars) {
        Play p = new Play(m_fses);
        p.setPromptList(getPromptList(fragment, vars));
        return p;
    }

    /**
     * Given an extension, convert to a full URL in this domain
     * 
     * @param extension
     * @return the Url
     */
    String extensionToUrl(String extension) {
        return "sip:" + extension + "@" + m_ivrConfig.getSipxchangeDomainName();
    }
    /**
     * Run each Attendant until there is nothing left to do. If the SIP URL didn't pass in a
     * particular attendant name, use the current time of day and the schedule to find which
     * attendant to run.
     * 
     * Keep running the next returned attendant until there are none left, then exit.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    public void run() throws Throwable {
        if (m_vmBundle == null) {
            loadConfig();
        }

        // Wait it bit so audio doesn't start too fast
        Sleep s = new Sleep(m_fses, 1000);
        s.go();
        String mailboxString = m_parameters.get("mailbox");
        m_action = m_parameters.get("action");

        while(mailboxString != null) {
            LOG.info("Starting voicemail for mailbox \""+mailboxString+"\" action=\""+m_action+"\" in locale " + m_localization.getLocale());
            mailboxString = voicemail(mailboxString);
        }
        LOG.info("Ending voicemail");
    }

    /**
     * Do the VoiceMail action.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    String voicemail(String mailboxString) throws Throwable {
        User user = m_validUsers.isValidUser(mailboxString) ;
        Mailbox mailbox = null;
        if (user != null) {
           mailbox = new Mailbox(user, m_localization);
        }
        if (mailbox == null) {
            // TODO ask for valid mailbox number
            return null ;
        }
        if (m_action.equals("deposit")) {
            return depositVoicemail(mailbox) ;
        }
        
        return null;
    }


    private String depositVoicemail(Mailbox mailbox) throws Throwable {
        LOG.info("Mailbox "+mailbox.getUser().getUserName()+" Depost Voicemail");
        
        // {user's greeting}
        // When you are finished, press 1 for more options.
        // To reach the operator, dial 0 at anytime.
        Greeting greeting = new Greeting(mailbox) ;
        Play p = greeting.getPlayer(m_fses) ;
        p.getPromptList().addFragment("VoiceMail_options");
        p.go();

        // Allow caller to barge the greeting with "#", "*" enters retrieval mode
        Collect c = new Collect(m_fses, 1, 100, 0, 0);
        c.setTermChars("*#");
        c.go();
        String digits = c.getDigits();  
        LOG.info("Collected digits=" + digits);

        return null;
    }

    public ResourceBundle getAttendantBundle() {
        return m_vmBundle;
    }

    public void setAttendantBundle(ResourceBundle attendantBundle) {
        m_vmBundle = attendantBundle;
    }

    public Configuration getConfig() {
        return m_config;
    }

    public void setConfig(Configuration config) {
        m_config = config;
    }

    public ValidUsersXML getValidUsers() {
        return m_validUsers;
    }

    public void setValidUsers(ValidUsersXML validUsers) {
        m_validUsers = validUsers;
    }
}
