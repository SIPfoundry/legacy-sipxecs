/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.attendant;

import java.io.File;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.attendant.Configuration.AttendantConfig;
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


public class Attendant {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for resource bundles keyed by locale
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private org.sipfoundry.sipxivr.Configuration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_aaId;
    private String m_scheduleId;
    private ResourceBundle m_attendantBundle;
    private AttendantConfig m_config;
    private Configuration m_attendantConfig;
    private Schedule m_schedule;
    private ValidUsersXML m_validUsers;
    private TextToPrompts m_ttp;
    private Locale m_locale;

    enum NextAction {
        repeat, exit, nextAttendant;
    }

    /**
     * Create an Attendant.
     * 
     * @param ivrConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI (to determine locale and which attendant
     *        to run)
     */
    public Attendant(org.sipfoundry.sipxivr.Configuration ivrConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
        this.m_aaId = parameters.get("attendant_id");
        this.m_scheduleId = parameters.get("schedule_id");
        this.m_locale = Locale.US; // Default to good ol' US of A

        // Look for "locale" parameter
        String localeString = parameters.get("locale");
        if (localeString == null) {
            // Okay, try "lang" instead
            localeString = parameters.get("lang");
        }

        // A locale was passed in . Parse it into parts and find a Locale to match
        if (localeString != null) {
            String[] localeElements = localeString.split("[_-]"); // Use either _ or - as seperator
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
            m_attendantBundle = s_resourcesByLocale.get(m_locale);
            if (m_attendantBundle == null) {
                // Nope. Find the on disk version, and keep it for next time.
                m_attendantBundle = ResourceBundle.getBundle(
                        "org.sipfoundry.attendant.AutoAttendant", m_locale);
                s_resourcesByLocale.put(m_locale, m_attendantBundle);
            }
        }

        // Find the TextToPrompt class as well
        m_ttp = TextToPrompts.getTextToPrompt(m_locale);
        // Tell it where to find the audio files
        String globalPrefix = m_attendantBundle.getString("global.prefix");
        LOG.debug("global.prefix originally "+globalPrefix) ;
        if (!globalPrefix.startsWith("/")) {
        	String docDir = m_ivrConfig.getDocDirectory();
        	if (!docDir.endsWith("/")) {
        		docDir += "/";
        	}
        	globalPrefix = docDir + globalPrefix;
        }
        LOG.info("global.prefix is "+globalPrefix) ;
        m_ttp.setPrefix(globalPrefix);
        
        // Load the attendant configuration
        m_attendantConfig = Configuration.update(true);

        // Update the valid users list
        m_validUsers = ValidUsersXML.update(true);

        // Load the schedule configuration
        m_schedule = m_attendantConfig.getSchedule(m_scheduleId) ;
    }

    /**
     * Helper function to get the PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param vars
     * @return The appropriate PromptList.
     */
    PromptList getPromptList(String fragment, String... vars) {
        PromptList pl = new PromptList(m_attendantBundle, m_ivrConfig, m_ttp);
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

        String id = null;

        if (m_attendantBundle == null) {
            loadConfig();
        }

        if (m_aaId == null) {
            Date now = Calendar.getInstance().getTime();
            if (m_schedule != null) {
                LOG.info(String.format("Attendant determined from schedule %s", m_schedule.getId()));
                // load the organizationprefs.xml file every time
                // (as it may change without warning
                m_schedule.loadPrefs(m_ivrConfig.getOrganizationPrefs());
            	id = m_schedule.getAttendant(now);
            } else {
                LOG.error(String.format("Cannot find schedule %s in autoattendants.xml.", 
                	m_scheduleId != null ?m_scheduleId : "null")) ;
            }
            if (id == null) {
                LOG.error("Cannot determine which attendant to use from schedule.") ;
            } else {
                LOG.info(String.format("Attendant %s selected", id));
            }
        } else {
            id = m_aaId;
            LOG.info(String.format("Attendant %s determined from URL parameter", id));
        }

        // Wait it bit so audio doesn't start too fast
        Sleep s = new Sleep(m_fses, 1000);
        s.go();

        while (id != null) {
            // Keep running attendants until there are no more to run
            id = attendant(id);
        }
    }

    /**
     * Do the specified Attendant.
     * 
     * @param id The id of the attendant.
     * @return The id of the next attendant, or null if there is no next.
     * @throws Throwable indicating an error or hangup condition.
     */
    String attendant(String id) throws Throwable {
        String nextAttendant = null;

        // Find the configuration for the named attendant
        m_config = m_attendantConfig.getAttendant(id);

        if (m_config == null) {
            LOG.error(String.format("Unable to determine which configuration to use from (%s)",
                    id));
            return null;
        }

        LOG.info("Starting attendant id " + id +" (" + m_config.getName() + ") in locale " + m_locale);

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
            if (m_config.getPrompt() != null && !m_config.getPrompt().contentEquals("")) {
                // Override default main menu prompts with user recorded one
                p = new Play(m_fses);
                PromptList pl = new PromptList();
                pl.addPrompts(m_config.getPrompt());
                p.setPromptList(pl);
            } else {
                // Use default menu
                p = getPlayer("main_menu");
            }
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

            AttendantMenuItem item = null;
            // Check if entered digits match any actions
            for (AttendantMenuItem menuItem : m_config.getMenuItems()) {
                if (menuItem.getDialpad().contentEquals(digits)) {
                    item = menuItem;
                    break;
                }
            }

            if (item != null) {
                // Do the action corresponding to that digit
                NextAction next = doAction(item);
                if (next.equals(NextAction.repeat)) {
                    timeoutCount = 0;
                    invalidCount = 0;
                    continue;
                } else if (next.equals(NextAction.nextAttendant)) {
                    nextAttendant = item.getDialpad();
                    break;
                } else if (next.equals(NextAction.exit)) {
                    break;
                }
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

        LOG.info("Ending attendant " + m_config.getName());
        return nextAttendant;
    }

    /**
     * Do the action from the corresponding menu item.
     * 
     * @param item
     * @return The next action to perform
     * @throws Throwable
     */
    NextAction doAction(AttendantMenuItem item) throws Throwable {
        String dest;
        Transfer xfer;
        
        switch (item.getAction()) {
        case repeat_prompt: 
            LOG.info("Repeat Prompt");
            return NextAction.repeat;
            
        case voicemail_access: 
            // Transfer to the voicemailUrl
            dest = m_ivrConfig.getVoicemailUrl();
            LOG.info("Voicemail Access.  Transfer to " + dest);
            xfer = new Transfer(m_fses, dest);
            xfer.go();
            return NextAction.exit;
        
        case voicemail_deposit: {
            // Transfer to the specific extension's VM.
            // Uses the internal ~~vm~xxxx user to do this.
            dest = extensionToUrl("~~vm~"+item.getExtension()) ;
            LOG.info("Voicemail Deposit.  Transfer to " + dest);
            xfer = new Transfer(m_fses, dest);
            xfer.go();
            return NextAction.exit;
        }
        case dial_by_name: 
            // Enter the Dial By Name dialog.
            LOG.info("Dial by Name");
            if (dialByName()) {
                return NextAction.exit;
            }
            return NextAction.repeat;
        
        case disconnect: 
            LOG.info("Disconnect");
            goodbye();
            return NextAction.exit;
        
        case operator: 
            // Transfer to the operator's address
            dest = m_ivrConfig.getOperatorAddr();
            LOG.info("Operator.  Transfer to " + dest);
            xfer = new Transfer(m_fses, dest);
            xfer.go();
            return NextAction.exit;
        
        case transfer_out: 
            // Transfer to the specified extension
            dest = extensionToUrl(item.getExtension()) ;
            LOG.info("Transfer Out.  Transfer to " + dest);
            xfer = new Transfer(m_fses, dest);
            xfer.go();
            return NextAction.exit;
        
        case transfer_to_another_aa_menu: 
            // "Transfer" to another attendant. Not really a call transfer
            // as it stays in this process. See run().
            LOG.info("Transfer to attendant " + item.getParameter());
            return NextAction.nextAttendant;
        
        default:
        }
        return NextAction.exit;
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
     * Given the dialed digits "spelling" a user name, create a menu of matching users (up to 9)
     * and have the caller enter one of the selections.
     * 
     * @param digits
     * @return true if attendant should hangup
     * @throws Throwable
     */
    boolean selectChoice(String digits) throws Throwable {
        // Lookup the list of validUsers that match the DTMF digits
        Vector<User> matches = m_validUsers.lookupDTMF(digits);

        if (matches.size() == 0) {
            // Indicate no match
            Play p = getPlayer("nomatch");
            p.setDigitMask("");
            p.go();
            return false;
        }

        /*
         * This is an enhancement over the original vxml which will prompt even if only one
         * matches. if (matches.size() == 1) { User u = matches.firstElement() ; transfer(u.uri);
         * return true ; }
         */

        // Build a menu of the matched user's names.
        // Limit the choices to the first 9 (or it gets too long)
        PromptList pl = new PromptList(m_attendantBundle, m_ivrConfig, m_ttp);
        int choices = matches.size();
        if (choices > 9) {
            choices = 9;
        }
        for (int i = 0; i < choices; i++) {
            pl.addFragment("press_n_for", Integer.toString(i + 1));

            User u = matches.get(i);
            // Try to speak the user's recorded name
            String recordedName = getRecordedName(u);
            if (recordedName != null) {
                pl.addPrompts(recordedName);
            } else {
                pl.addFragment("extension", u.getUserName());
            }
        }
        pl.addFragment("enter_different_name");

        // Dialog for the caller to enter one of the choices
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            /*
             * This is an enhancement over the original VoiceXML As it is more consistant if
             * (invalidCount >= config.invalidResponseCount || timeoutCount >=
             * config.noInputCount) { failure() ; return true ; }
             */

            // This one matches existing VoiceXML
            if (invalidCount >= 3 || timeoutCount >= 3) {
                goodbye();
                return true;
            }

            // Play the menu
            Play p = new Play(m_fses, pl);
            p.go();

            // Wait for the caller to enter a digit
            Collect c = new Collect(m_fses, 1, m_config.getInitialTimeout(), 0, 0);
            c.setTermChars("*#");
            c.go();
            String choice = c.getDigits();
            LOG.info("::selectChoice Collected digits=" + choice);

            // See what they entered
            if (choice.length() == 0) {
                timeoutCount++;
                continue;
            }

            if (choice.contentEquals("*")) {
                p = getPlayer("canceled");
                p.go();
                return false;
            }

            if (choice.contentEquals("#")) {
                p = getPlayer("no_entry_matches");
                p.setDigitMask("");
                p.go();
                invalidCount = 0;
                timeoutCount = 0;
                continue;
            }

            if ("123456789".contains(choice)) {
                int selected = Integer.parseInt(choice);
                if (selected <= choices) {
                    User u = matches.get(selected - 1);
                    LOG.info(String.format("Transfer to extension %s (%s)", u.getUserName(), u.getUri()));
                    transfer(u.getUri());
                    return true;
                }
            }

            p = getPlayer("no_entry_matches");
            p.setDigitMask("");
            p.go();
            invalidCount = 0;
            timeoutCount = 0;
            continue;
        }
    }

    /**
     * The Dial by Name dialog.
     * 
     * Prompts the caller to enter digits that "spell" the name of the user.
     * Collects the digits, then calls selectChoice() to see if it matches
     * any of the valid user names.
     * 
     * @return true if attendant should hangup.
     * @throws Throwable
     */
    boolean dialByName() throws Throwable {
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            /*
             * 
             * This is an enhancement over the original VoiceXML 
               if (invalidCount >= config.invalidResponseCount || timeoutCount >= config.noInputCount) { 
                  failure() ;
                  return true ; 
               }
             */

            // This one matches the existing VoiceXML
            if (invalidCount >= 3 || timeoutCount >= 3) {
                goodbye();
                return true;
            }

            // Play the menu
            Play p = getPlayer("dial_by_name");
            p.go();

            // Collect the digits from the caller.  Use the "*" and "#" keys as terminators
            // There are a LONG (10 second) digit timers here, as spelling on the phone
            // is difficult!  The "#" key will terminate any input if the caller is finished.
            Collect c = new Collect(m_fses, 20, 10000, 10000, 2000);
            c.setTermChars("*#");
            c.go();
            String digits = c.getDigits();
            LOG.info("::dialByName Collected digits=" + digits);

            // Timed out.  (No digits)
            if (digits.length() == 0) {
                ++timeoutCount;
                continue;
            }

            if (digits.contentEquals("*")) {
                p = getPlayer("canceled");
                p.go();
                return false;
            }

            if (digits.contentEquals("0") || digits.contentEquals("1")) {
                // This is an enhancement over the original VoiceXML 
                // if (++invalidCount < config.invalidResponseCount)
                {
                    p = getPlayer("invalid_try_again");
                    p.setDigitMask("");
                    p.go();
                }
                continue;
            }

            if (selectChoice(digits)) {
                return true;
            }

            invalidCount = 0;
            timeoutCount = 0;
        }
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
        return m_attendantBundle;
    }

    public void setAttendantBundle(ResourceBundle attendantBundle) {
        m_attendantBundle = attendantBundle;
    }

    public AttendantConfig getConfig() {
        return m_config;
    }

    public void setConfig(AttendantConfig config) {
        m_config = config;
    }

    public Configuration getAttendantConfig() {
        return m_attendantConfig;
    }

    public void setAttendantConfig(Configuration attendantConfig) {
        m_attendantConfig = attendantConfig;
    }

    public Schedule getSchedules() {
        return m_schedule;
    }

    public void setSchedules(Schedule schedule) {
        m_schedule = schedule;
    }

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
