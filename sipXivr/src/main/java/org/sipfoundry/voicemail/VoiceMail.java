/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.voicemail;

import java.io.File;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Collect;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.Hangup;
import org.sipfoundry.sipxivr.Play;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.Sleep;
import org.sipfoundry.sipxivr.TextToPrompts;
import org.sipfoundry.sipxivr.Transfer;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;


public class VoiceMail {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for resource bundles keyed by locale
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private org.sipfoundry.sipxivr.Configuration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_parameterName;
    private ResourceBundle m_vmBundle;
    private Configuration m_config;
// TODO     private Configuration m_attendantConfig;
    private ValidUsersXML m_validUsers;
    private TextToPrompts m_ttp;
    private Locale m_locale;

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
        this.m_parameterName = parameters.get("name");
        this.m_locale = Locale.US; // Default to good ol' US of A

        // Look for "locale" parameter
        String localeString = parameters.get("locale");
        if (localeString == null) {
            // Okay, try "lang" instead
            localeString = parameters.get("lang");
        }

        // A locale was passed in . Parse it into parts and find a Locale to match
        if (localeString != null) {
            String[] localeElements = localeString.split("_");
            String lang = "";
            String country = "";
            String variant = "";
            if (localeElements.length >= 3) {
                variant = localeElements[2];
            }
            if (localeElements.length >= 2) {
                country = localeElements[1];
            }
            if (localeElements.length >= 1) {
                lang = localeElements[0];
            }

            m_locale = new Locale(lang, country, variant);
        }

    }

    /**
     * Load all the needed configuration.
     * 
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. The attendant configuration files are loaded (if they changed since last time), and
     * the ValidUsers (also if they changed).
     * 
     * Don't forget the schedules and the organization preferences!
     * 
     */
    void loadConfig() {
        // Load the resources for the given locale.

        // Check to see if we've loaded this one before...
        synchronized (s_resourcesByLocale) {
            m_vmBundle = s_resourcesByLocale.get(m_locale);
            if (m_vmBundle == null) {
                // Nope. Find the on disk version, and keep it for next time.
                m_vmBundle = ResourceBundle.getBundle(
                        "org.sipfoundry.voicemail.VoiceMail", m_locale);
                s_resourcesByLocale.put(m_locale, m_vmBundle);
            }
        }

        // Find the TextToPrompt class as well
        m_ttp = TextToPrompts.getTextToPrompt(m_locale);
        // Tell it where to find the audio files
        String globalPrefix = m_vmBundle.getString("global.prefix");
        if (!globalPrefix.startsWith("/")) {
        	String docDir = m_ivrConfig.getDocDirectory();
        	if (!docDir.endsWith("/")) {
        		docDir += "/";
        	}
        	globalPrefix = docDir + globalPrefix;
        }
        m_ttp.setPrefix(globalPrefix);
        
        // Load the Voice Mail configuration
        m_config = Configuration.update(true);

        // Update the valid users list
        m_validUsers = ValidUsersXML.update(true);
    }

    /**
     * Helper function to get the PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param vars
     * @return The appropriate PromptList.
     */
    PromptList getPromptList(String fragment, String... vars) {
        PromptList pl = new PromptList(m_vmBundle, m_ivrConfig, m_ttp);
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

        voicemail();
    }

    /**
     * Do the VoiceMail action.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    void voicemail() throws Throwable {
        LOG.info("Starting voicemail in locale " + m_locale);

        String digits;
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            // Check for a failure condition
            if (invalidCount >= m_config.getInvalidResponseCount()
                    || timeoutCount >= m_config.getNoInputCount()) {
                failure();
                break;
            }

            // Play the initial prompt, or main menu.
            Play p = null;
            // Use default menu
            p = getPlayer("main_menu");
            p.go();

            // Wait for the caller to enter a selection.
            Collect c = new Collect(m_fses, m_config.getMaximumDigits(), m_config.getInitialTimeout(),
                    m_config.getInterDigitTimeout(), m_config.getExtraDigitTimeout());
            c.setTermChars("*#");
            c.go();
            digits = c.getDigits();
            LOG.info("Collected digits=" + digits);

            // See if it timed out (no digits)
            if (digits.equals("")) {
                timeoutCount++;
                continue;
            }

            // None of the above...must be an extension
            // For a nicer implementation, uncomment this
            // if (digits.length() >= 2)
            // but to match the original VoiceXML...
            if (true) {
                // See if the entered digits matches a dialable extension
                // (keeps AA users from entering long distance numbers, 900 numbers,
                // call pickup, paging, etc.)
                User user = m_validUsers.isValidUser(digits);
                if (user != null) {
                    String uri = user.getUri();
                    LOG.info(String.format("Transfer to extension %s (%s)", digits, uri));
                    // It's valid, transfer the call there.
                    transfer(uri);
                    break;
                }

                LOG.info("Extension " + digits + " is not valid");
                Play pc2 = getPlayer("invalid_extension");

                pc2.setDigitMask("");
                pc2.go();
                invalidCount++;
                continue;
            }

            // What they entered has no corresponding action.
            LOG.info("Invalid entry");
            Play e = getPlayer("invalid_entry");
            e.setDigitMask("");
            e.go();
            invalidCount++;
        }

        LOG.info("Ending voicemail");
    }


    /**
     * Play the "please hold" prompt and then transfer.
     * 
     * @param uri
     * @throws Throwable
     */
    void transfer(String uri) throws Throwable {
        Play pc2 = getPlayer("please_hold");
        pc2.setDigitMask("");
        pc2.go();
        Transfer xfer = new Transfer(m_fses, uri);
        xfer.go();
    }

    /**
     * Get the path with the user's recorded name.
     * 
     * @param u
     * @return the path, or null if it doesn't exist.
     */
    String getRecordedName(User u) {
        // The recorded name is stored in the "name.wav" file in the user's directory
        String name = m_ivrConfig.getMailstoreDirectory() + "/" + u.getUserName() + "/name.wav";
        File f = new File(name);
        if (f.exists()) {
            return name;
        }
        return null;
    }


    /**
     * Say good bye and hangup.
     * 
     * @throws Throwable
     */
    void goodbye() throws Throwable {
        Play p = getPlayer("goodbye");
        p.setDigitMask("");
        p.go();
        Hangup h = new Hangup(m_fses);
        h.go();
    }

    /**
     * Perform the configured "failure" behavior, which can be either just hangup
     * or transfer to a destination after playing a prompt.
     * 
     * @throws Throwable
     */
    void failure() throws Throwable {
        LOG.info("Input failure");

        if (m_config.isTransferOnFailure()) {
            Play p = new Play(m_fses);
            PromptList pl = new PromptList();
            pl.addPrompts(m_config.getTransferPrompt());
            p.setDigitMask("");
            p.go();
            
            String dest = m_config.getTransferURL();
            if (!dest.toLowerCase().contains("sip:")) {
                LOG.error("transferUrl should be a sip: URL.  Assuming extension");
                dest = "sip:" + dest + "@"+ m_ivrConfig.getSipxchangeDomainName();
            }

            LOG.info("Transfer on falure to " + dest);
            Transfer xfer = new Transfer(m_fses, dest);
            xfer.go();
        }

        // This is an enhancement over the original VoiceXML 
        // goodbye() ;

        // This one matches the existing VoiceXML
        Hangup h = new Hangup(m_fses);
        h.go();
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

/* TODO
    public Configuration getAttendantConfig() {
        return m_attendantConfig;
    }

    public void setAttendantConfig(Configuration attendantConfig) {
        m_attendantConfig = attendantConfig;
    }
*/

    public TextToPrompts getTtp() {
        return m_ttp;
    }

    public void setTtp(TextToPrompts ttp) {
        m_ttp = ttp;
    }

    public ValidUsersXML getValidUsers() {
        return m_validUsers;
    }

    public void setValidUsers(ValidUsersXML validUsers) {
        m_validUsers = validUsers;
    }
}
