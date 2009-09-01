/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.voicemail;

import java.util.Collection;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.DisconnectException;
import org.sipfoundry.sipxivr.DistributionList;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.Hangup;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.Menu;
import org.sipfoundry.sipxivr.PersonalAttendant;
import org.sipfoundry.sipxivr.Play;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.Record;
import org.sipfoundry.sipxivr.Sleep;
import org.sipfoundry.sipxivr.Transfer;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;


public class VoiceMail {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for VoiceMail resource bundles keyed by locale
    private static final String RESOURCE_NAME = "org.sipfoundry.voicemail.VoiceMail";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private org.sipfoundry.sipxivr.Configuration m_ivrConfig;
    private Localization m_loc;
    private FreeSwitchEventSocketInterface m_fses;
    private ResourceBundle m_vmBundle;
    private Configuration m_config;
    private ValidUsersXML m_validUsers;
    private HashMap<String, DistributionList> m_sysDistLists;

    private Hashtable<String, String> m_parameters;  // The parameters from the sip URI
    
    private Vector<Message> m_messages; // Message to be delivered at hangup

    private String m_action; // "deposit" or "retrieve"

    private Mailbox m_mailbox ;
    
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
        this.m_messages = new Vector<Message>();

        // Look for "locale" parameter
        String localeString = parameters.get("locale");
        if (localeString == null) {
            // Okay, try "lang" instead
            localeString = parameters.get("lang");
        }

        // Load the resources for the given localeString
        m_loc = new Localization(RESOURCE_NAME, localeString, 
                s_resourcesByLocale, ivrConfig, fses);
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
     * Given an extension, convert to a full URL in this domain
     * 
     * @param extension
     * @return the Url
     */
    String extensionToUrl(String extension) {
        return "sip:" + extension + "@" + m_ivrConfig.getSipxchangeDomainName();
    }
    
    /**
     * Run Voice Mail for each mailbox until there is nothing left to do.
     *
     * Keep running the next returned mailbox until there are none left, then exit.
     * 
     */
    public void run() throws Throwable {
        if (m_vmBundle == null) {
            loadConfig();
        }

        // Wait a bit so audio doesn't start too fast
        Sleep s = new Sleep(m_fses, 1000);
        s.go();
        String mailboxString = m_parameters.get("mailbox");
        if (mailboxString == null) {
            // Use the From: user as the mailbox
            mailboxString = m_fses.getFromUser();
        }
        m_action = m_parameters.get("action");

        while(mailboxString != null) {
            LOG.info("Starting voicemail for mailbox \""+mailboxString+"\" action=\""+m_action+"\" in locale " + m_loc.getLocale());
            mailboxString = voicemail(mailboxString);
        }
        LOG.info("Ending voicemail");
    }

    /**
     * Do the VoiceMail action.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    String voicemail(String mailboxString) {
        User user = m_validUsers.isValidUser(mailboxString) ;
        Mailbox mailbox = null;
        if (user != null) {
           mailbox = new Mailbox(user);
        }
        
        m_mailbox = mailbox;
        if (m_action.equals("deposit")) {
            if (mailbox == null) {
                // That extension is not valid.
                LOG.info("Extension "+mailboxString+" is not valid.");
                m_loc.play("invalid_extension", "");
                goodbye();
                return null ;
            }

            if (!user.hasVoicemail()) {
                LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" does not have voicemail permission.");
                // That extension is not valid.
                m_loc.play("invalid_extension", "");
                goodbye();
                return null ;
            }
            // Create the mailbox if it isn't there
            Mailbox.createDirsIfNeeded(m_mailbox);
            try {
                
                String result = new Deposit(this).depositVoicemail() ;
                if (result == null) {
                    goodbye();
                    return null ;
                }
                m_action = result;
                
            } catch (DisconnectException e) {
            } finally {
                try {
                    // Let FS finish any recording's it might be doing
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
                // Deliver any messages that are pending
                for (Message message : m_messages) {
                    message.storeInInbox();
                }
            }
        }
        
        if (m_action.equals("retrieve")) {
            return new Retrieve(this).retrieveVoiceMail();
        }
        
        return null;
    }


    /**
     * Select a distribution list from the list of lists.  Boy is this confuzing!
     * @return A list of users on the distribution list, null on error
     */
    Vector<User> selectDistributionList() {
        String validDigits = "123456789";
        Distributions d = null;
        
        // See if the new way to get distribution lists is being used.
        HashMap<String, DistributionList> dlists = m_mailbox.getUser().getDistributionLists();
        if (dlists == null) {
            // Use the old way (distributionListsFile in the user's mailbox directory)
            DistributionsReader dr = new DistributionsReader();
            d = dr.readObject(m_mailbox.getDistributionListsFile()) ;
            if (d != null) {
                validDigits = d.getIndices();
            }
        } else {
            // Delete the old distributions.xml file so it isn't accidentally used later
            m_mailbox.getDistributionListsFile().delete();
        }
        

        for(;;) {
            // "Please select the distribution list.  Press * to cancel."
            PromptList pl = m_loc.getPromptList("deposit_select_distribution");
            Menu menu = new VmMenu(this);
            IvrChoice choice = menu.collectDigit(pl, validDigits);
    
            if (!menu.isOkay()) {
                return null;
            }
            String digit = choice.getDigits();
            LOG.info("Mailbox "+m_mailbox.getUser().getUserName()+" selectDistributionList ("+digit+")");
            
            Collection<String> userNames = null;
            if (dlists != null) {
                DistributionList list = dlists.get(digit);
                if (list != null) {
                    userNames = list.getList(m_sysDistLists);
                }
            } else if (d != null) {
                userNames = d.getList(digit);
            }
                
            if (userNames != null) {
                Vector<User> users = new Vector<User>();
                for (String userName : userNames) {
                    User u = m_validUsers.isValidUser(userName);
                    if (u != null && u.hasVoicemail()) {
                        users.add(u);
                    }
                }
                
                return users;
            }

            // "The list you have selected is not valid"
            m_loc.play("deposit_distribution_notvalid", "");
        }
    }
    

    /**
     * Get the operator URL defined for this user.
     * Uses the PersonalAttendant's operator if defined, else the systems.
     * @param pa
     * @return
     */
    String getOperator(PersonalAttendant pa) {
        String transferUrl;
        // Try the Personal Attendant's definition of operator
        transferUrl = pa.getOperator();
        if (transferUrl == null) {
            // Try the system's definition of operator
            transferUrl = m_ivrConfig.getOperatorAddr();
        }
        return transferUrl;
    }

    /**
     * Record a message into a file named wavName
     * 
     * @param wavName
     * @return The Recording object
     */
    Record recordMessage(String wavName) {
        // Flush any typed ahead digits
        m_fses.trimDtmfQueue("") ;
        LOG.info(String.format("Recording message (%s)", wavName));
        Record rec = new Record(m_fses, m_loc.getPromptList("beep"));
        rec.setRecordFile(wavName) ;
        rec.setRecordTime(300); // TODO a better time here?
        rec.setDigitMask("0123456789*#"); // Any digit can stop the recording
        rec.go();
        return rec;
    }
    
 
    /**
     * Transfer to the operator
     */
    void operator() {
        transfer(getOperator(m_mailbox.getPersonalAttendant()));
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
     * Say good bye and hangup.
     * 
     */
    void goodbye() {
        LOG.info("good bye");
        // Thank you.  Goodbye.
        m_loc.play("goodbye", "");
        new Hangup(m_fses).go();
    }

    /**
     * Perform the configured "failure" behavior, which can be either just hangup
     * or transfer to a destination after playing a prompt.
     * 
     */
    void failure() {
        LOG.info("Input failure");

        if (m_config.isTransferOnFailure()) {
            new Play(m_fses, m_config.getTransferPrompt()).go();
            
            String dest = m_config.getTransferURL();
            if (!dest.toLowerCase().contains("sip:")) {
                LOG.error("transferUrl should be a sip: URL.  Assuming extension");
                dest = extensionToUrl(dest) ;
            }

            LOG.info("Transfer on falure to " + dest);
            new Transfer(m_fses, dest).go();
        }

        goodbye() ;
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
    
    public Localization getLoc() {
        return m_loc;
    }
    
    public void setLoc(Localization loc) {
        m_loc = loc;
    }
    
    public HashMap<String, DistributionList> getSysDistList() {
        return m_sysDistLists;
    }
    
    /**
     * Get the list of messages to be delivered at hangup.
     * @return
     */
    public Vector<Message> getMessages() {
        return m_messages;
    }

    public Mailbox getMailbox() {
        return m_mailbox;
    }
}
