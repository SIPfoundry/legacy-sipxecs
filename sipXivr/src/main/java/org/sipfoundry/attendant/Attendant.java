/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.attendant.Configuration.AttendantConfig;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.freeswitch.Transfer;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;


public class Attendant {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for AutoAttendant resource bundles keyed by locale
    private static final String RESOURCE_NAME="org.sipfoundry.attendant.AutoAttendant";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private IvrConfiguration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_aaId;
    private String m_scheduleId;
    private AttendantConfig m_config;
    private Configuration m_attendantConfig;
    private Schedule m_schedule;
    private ValidUsersXML m_validUsers;
    private TextToPrompts m_ttp;
    private String m_localeString;
    private Localization m_loc;

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
    public Attendant(IvrConfiguration ivrConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
        this.m_aaId = parameters.get("attendant_id");
        this.m_scheduleId = parameters.get("schedule_id");

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
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. The attendant configuration files are loaded (if they changed since last time), and
     * the ValidUsers (also if they changed).
     * 
     * Don't forget the schedules and the organization preferences!
     * 
     */
    void loadConfig() {            
        try {
            m_loc = new Localization("AutoAttendant", 
                    m_localeString, s_resourcesByLocale, m_ivrConfig, m_fses);
        } catch (MissingResourceException e) {
            // Use the built in one as a last resort
            m_loc = new Localization(RESOURCE_NAME, 
                    m_localeString, s_resourcesByLocale, m_ivrConfig, m_fses);
        }
                
        // Load the attendant configuration
        m_attendantConfig = Configuration.update(true);

        // Update the valid users list
        try {
            m_validUsers = ValidUsersXML.update(LOG, true);
        } catch (Exception e) {
            System.exit(1); // If you can't trust validUsers, who can you trust?
        }

        // Load the schedule configuration
        m_schedule = m_attendantConfig.getSchedule(m_scheduleId) ;
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
    public void run() {

        String id = null;

        if (m_loc == null) {
            loadConfig();
        }

        if (m_aaId == null) {
            // See if a special attendant is defined
            id = m_attendantConfig.getSpecialAttendantId() ;
            if (id != null) {
                // Always use the special AA if specialOperation is in effect
                LOG.info("Attendant::run Special Operation AutoAttendant is in effect.");
            } else {
                Date now = Calendar.getInstance().getTime();
                if (m_schedule != null) {
                    LOG.info(String.format("Attendant::run Attendant determined from schedule %s", m_schedule.getId()));
                	id = m_schedule.getAttendant(now);
                } else {
                    LOG.error(String.format("Attendant::run Cannot find schedule %s in autoattendants.xml.", 
                    	m_scheduleId != null ?m_scheduleId : "null")) ;
                }
                if (id == null) {
                    LOG.error("Attendant::run Cannot determine which attendant to use from schedule.") ;
                } else {
                    LOG.info(String.format("Attendant::run Attendant %s selected", id));
                }
            }
        } else {
            id = m_aaId;
            LOG.info(String.format("Attendant::run Attendant %s determined from URL parameter", id));
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
     */
    String attendant(String id){
        String nextAttendant = null;

        // Find the configuration for the named attendant
        m_config = m_attendantConfig.getAttendant(id);

        if (m_config == null) {
            LOG.error(String.format("Attendant::attendant Unable to determine which configuration to use from (%s)",
                    id));
            return null;
        }

        LOG.info("Attendant::attendant Starting attendant id " + id +" (" + m_config.getName() + ") in locale " + m_loc.getLocale());

        String digits;
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            // Check for a failure condition
            if (invalidCount > m_config.getInvalidResponseCount()
                    || timeoutCount > m_config.getNoInputCount()) {
                failure();
                break;
            }

            // Play the initial prompt, or main menu.
            PromptList pl = null;
            if (m_config.getPrompt() != null && !m_config.getPrompt().contentEquals("")) {
                // Override default main menu prompts with user recorded one
                pl = m_loc.getPromptList();
                pl.addPrompts(m_config.getPrompt());
            } else {
                // Use default menu
                pl = m_loc.getPromptList("main_menu");
            }
            m_loc.play(pl, "0123456789#*");

            // Wait for the caller to enter a selection.
            Collect c = new Collect(m_fses, m_config.getMaximumDigits(), m_config.getInitialTimeout(),
                    m_config.getInterDigitTimeout(), m_config.getExtraDigitTimeout());
            c.setTermChars("*#");
            c.go();
            digits = c.getDigits();
            LOG.info("Attendant::attendant Collected digits=" + digits);

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
                    nextAttendant = item.getParameter();
                    break;
                } else if (next.equals(NextAction.exit)) {
                    break;
                }
                continue;
            }

            // None of the above...must be an extension.
            // For a nicer implementation, uncomment this
            // if (digits.length() >= 2)
            // but to match the original VoiceXML...
            if (true) {
                // See if the entered digits matches a dialable extension
                // (keeps AA users from entering long distance numbers, 900 numbers,
                // call pickup, paging, etc.)
                User user = m_validUsers.getUser(digits);
                if (user != null) {
                    String uri = user.getUri();
                    LOG.info(String.format("Attendant::attendant Transfer to extension %s (%s)", digits, uri));
                    // It's valid, transfer the call there.
                    transfer(uri);
                    break;
                }

                LOG.info("Attendant::attendant Extension " + digits + " is not valid");
                // "That extension is not valid."
                m_loc.play("invalid_extension", "");
                invalidCount++;
                continue;
            }

            // What they entered has no corresponding action.
            LOG.info("Attendant::attendant Invalid entry "+digits);
            // "Invalid entry.  Try again."
            m_loc.play("invalid_entry", "");
            invalidCount++;
        }

        LOG.info("Attendant::attendant Ending attendant " + m_config.getName());
        return nextAttendant;
    }

    /**
     * Do the action from the corresponding menu item.
     * 
     * @param item
     * @return The next action to perform
     */
    NextAction doAction(AttendantMenuItem item) {
        String dest;
        Transfer xfer;
        
        switch (item.getAction()) {
        case repeat_prompt: 
            LOG.info("Attendant::doAction Repeat Prompt");
            return NextAction.repeat;
            
        case voicemail_access: 
            // Transfer to the voicemailUrl
            dest = item.getParameter();
            LOG.info("Attendant::doAction Voicemail Access.  Transfer to " + dest);
            xfer = new Transfer(m_fses, dest);
            xfer.go();
            return NextAction.exit;
        
        case voicemail_deposit: {
            // Transfer to the specific extension's VM.
            
            // Lookup the extension (it may be an alias)
            String extension = item.getExtension();
            User u = m_validUsers.getUser(extension);
            if (u != null) {
                // Use the internal ~~vm~xxxx user to do this.
                dest = extensionToUrl("~~vm~"+u.getUserName());
                LOG.info("Attendant::doAction Voicemail Deposit.  Transfer to " + dest);
                xfer = new Transfer(m_fses, dest);
                xfer.go();
            } else {
                LOG.error("Attendant::doAction Voicemail Deposit cannot find user for extension "+extension);
            }
            return NextAction.exit;
        }
        case dial_by_name: 
            // Enter the Dial By Name dialog.
            LOG.info("Attendant::doAction Dial by Name");
            DialByName dbn = new DialByName(m_loc, m_config, m_validUsers);
            DialByNameChoice choice = dbn.dialByName() ;
            if (choice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                return NextAction.repeat;
            }
            Vector<User> u = choice.getUsers();
            if (u == null) {
                goodbye();
                return NextAction.exit;
            }
            LOG.info(String.format("Attendant::doAction Transfer to extension %s (%s)", u.get(0).getUserName(), u.get(0).getUri()));
            transfer(u.get(0).getUri());
            return NextAction.repeat;
        
        case disconnect: 
            LOG.info("Attendant::doAction Disconnect");
            goodbye();
            return NextAction.exit;
        
        case operator: 
            // Transfer to the operator's address
            dest = m_ivrConfig.getOperatorAddr();
            LOG.info("Attendant::doAction Operator.  Transfer to " + dest);
            transfer(dest);
            return NextAction.exit;
        
        case transfer_out: 
            // Transfer to the specified extension
            String extensionOrOther = item.getExtension();
            if (extensionOrOther.contains("@")) {
                // If it's got an @, assume its a SIP url (or close enough)
                dest = extensionOrOther ;
            } else {
                // Assume it's an extension, and tack on "sip:" and @ourdomain
                dest = extensionToUrl(extensionOrOther) ;
            }
            LOG.info("Attendant::doAction Transfer Out.  Transfer to " + dest);
            transfer(dest);
            return NextAction.exit;
        
        case transfer_to_another_aa_menu: 
            // "Transfer" to another attendant. Not really a call transfer
            // as it stays in this thread. See run().
            LOG.info("Attendant::doAction Transfer to attendant " + item.getParameter());
            return NextAction.nextAttendant;
        
        default:
        }
        return NextAction.exit;
    }

    /**
     * Play the "please hold" prompt and then transfer.
     * 
     * @param uri
     */
    void transfer(String uri) {
        m_loc.play("please_hold", "");
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
     */
    void goodbye() {
        m_loc.play("goodbye", "");
        Hangup h = new Hangup(m_fses);
        h.go();
    }

    /**
     * Perform the configured "failure" behavior, which can be either just hangup
     * or transfer to a destination after playing a prompt.
     * 
     */
    void failure() {
        LOG.info("Attendant::failure");
        if (m_config.isTransferOnFailure()) {
            String transferPrompt = m_config.getTransferPrompt();
            if (transferPrompt != null){
                PromptList pl = m_loc.getPromptList();
                pl.addPrompts(m_config.getTransferPrompt());
                m_loc.play(pl, "");
            }

            String dest = m_config.getTransferURL();
            if (!dest.toLowerCase().contains("sip:")) {
                LOG.error("Attendant::failure transferUrl should be a sip: URL.  Assuming extension");
                dest = extensionToUrl(dest) ;
            }
            LOG.info("Attendant::failure Transfer on falure to " + dest);

            m_loc.play("please_hold","");
            String domainPart = ValidUsersXML.getDomainPart(dest);
            String transferDomain = m_ivrConfig.getSipxchangeDomainName();
            if (domainPart.equalsIgnoreCase(transferDomain)){
                String userpart = ValidUsersXML.getUserPart(dest);
                User user = m_validUsers.getUser(userpart);
                if (user != null) {
                    String uri = user.getUri();
                    LOG.info(String.format("Attendant::attendant Transfer to extension %s (%s)", dest, uri));
                    // It's valid, transfer the call there.
                    Transfer xfer = new Transfer(m_fses, dest);
                    xfer.go();
                } 
                else {
                    LOG.info("Attendant::attendant Extension " + dest + " is not valid");
                    // "That extension is not valid."
                    m_loc.play("invalid_extension", "");
                }
            } else
            {
                Transfer xfer = new Transfer(m_fses, dest);
                xfer.go();
            }
        }

        // This is an enhancement over the original VoiceXML 
        goodbye() ;

        Hangup h = new Hangup(m_fses);
        h.go();
    }

    public Localization getLocalization() {
        return m_loc;
    }

    public void setLocalization(Localization localization) {
        m_loc = localization;
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
