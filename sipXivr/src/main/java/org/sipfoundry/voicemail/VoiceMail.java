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
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Collect;
import org.sipfoundry.sipxivr.DialByName;
import org.sipfoundry.sipxivr.DialByNameChoice;
import org.sipfoundry.sipxivr.DisconnectException;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketInterface;
import org.sipfoundry.sipxivr.Hangup;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.Play;
import org.sipfoundry.sipxivr.PromptList;
import org.sipfoundry.sipxivr.Record;
import org.sipfoundry.sipxivr.Sleep;
import org.sipfoundry.sipxivr.Transfer;
import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;


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

    private Hashtable<String, String> m_parameters;
    
    private Vector<Message> m_messages; // Message to be delivered at hangup

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
           mailbox = new Mailbox(user, m_loc);
        }
        
        if (mailbox == null) {
            // That extension is not valid.
            m_loc.play("invalid_extension", "");
            goodbye();
            return null ;
        }
        
        if (m_action.equals("deposit")) {
            try {
                return depositVoicemail(mailbox) ;
            } catch (DisconnectException e) {
            } finally {
                // Deliver any messages that are pending
                for (Message message : m_messages) {
                    message.storeInInbox();
                }
                // TODO Light MWI
            }
        } else {
            // TODO other actions
        }
        
        return null;
    }


    /**
     * The depositVoicemail dialog for the give mailbox
     * 
     * @param mailbox
     * @return
     * @throws Throwable
     */
    String depositVoicemail(Mailbox mailbox) {
        // FreeSWITCH breaks up the original From URI, rebuild it as much as possible
        // (header/field parameters are missing, but I don't think VoiceMail cares)
        String callerId = m_fses.getVariable("Caller-Caller-ID-Name");
        String fromUri = m_fses.getVariable("variable_sip_from_uri");
        String displayUri = String.format("\"%s\" <sip:%s>", callerId, fromUri);
        LOG.info("Mailbox "+mailbox.getUser().getUserName()+" Deposit Voicemail from "+displayUri);
        
        PersonalAttendant pa = new PersonalAttendant(mailbox) ;
        String localeString = pa.getLanguage();
        if (localeString != null) {
            LOG.debug("Changing locale for this call to "+localeString);
            m_loc.changeLocale(localeString);
        }
        
        // {user's greeting}
        Greeting greeting = new Greeting(mailbox) ;
        PromptList pl = greeting.getPromptList() ;

        // When you are finished, press 1 for more options.
        // To reach the operator, dial 0 at any time.
        pl.addFragment("VoiceMail_options");
        
        // Allow caller to barge with 0, *, and any defined Personal Attendant digit
        // Also, allow barge to recording with "#"
        m_loc.play(pl, "#0*"+pa.getValidDigits());

        Collect c = new Collect(m_fses, 1, 100, 0, 0);
        c.setTermChars("#");
        c.go();
        String digits = c.getDigits();  
        LOG.info("depositVoicemail Collected digits=" + digits);
        
        if (digits.equals("*")) {
            // TODO retrieval mode
            return null;
        }
        
        // See if the digit they pressed was defined in the Personal Attendant
        String transferUrl = null;
        if (digits.equals("0")) {
            transferUrl = getOperator(pa);
        } else {
            // See if the Personal Attendant defined that digit to mean anything
            transferUrl = pa.getMenuValue(digits) ;
        }
        
        if (transferUrl != null) {
            LOG.info(String.format("Transfer to %s", transferUrl));
            transfer(transferUrl);
            return null ;
        }
        
        // Time to record a message
        String wavName = "/tmp/dog.wav"; // TODO where to put and name temp files?
        Message message = new Message(mailbox, wavName, displayUri, Priority.NORMAL);
        m_messages.add(message) ;
        
        boolean recorded = false ;
        
        for(;;) {
            // Record the message
            if (!recorded) {
                Record rec = recordMessage(wavName);
                
                if (rec.getDigits() == "0") {
                    // Don't save the message.
                    message.setIsToBeStored(false);
                    transfer(getOperator(pa));
                    return null;
                }
                recorded = true ;
                m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
            }
            
            // Confirm caller's intent for this message

            // To play this message, press 1.  
            // To send this message, press 2. 
            // To delete this message and try again, press 3.  
            // To cancel, press *."
            pl = m_loc.getPromptList("deposit_options");
            String digit = menu(pl, "0123*");

            // bad entry, timeout or
            // "*" means cancel
            if (digit == null || digit.equals("*")) {
                // Don't save the message.
                message.setIsToBeStored(false);
                goodbye();
                return null;
            }
            
            // "0" means transfer to operator
            if (digit.equals("0")) {
                // Don't save the message.
                message.setIsToBeStored(false);
                transfer(getOperator(pa));
                return null;
            }
            
            // "1" means play the message
            if (digit.equals("1")) {
                LOG.info(String.format("Playing back message (%s)", wavName));
                new Play(m_fses, wavName).go();
                continue ;
            }
            
            // "2" means send the message.
            if (digit.equals("2")) {
                message.storeInInbox(); // TODO failure (too short)
                break ;
            }
            
            // "3" means "erase" and re-record
            if (digit.equals("3")) {
                recorded = false ;
                continue ;
            }
        }

        // "Your message has been recorded."
        m_loc.play("deposit_recorded", "");

        // Message sent, now see what else they want to do
        MoreOptions(mailbox, pa, message);
        
        return null;
    }

    /**
     * See if the caller wants to send this message to other mailboxes
     * @param mailbox
     */
    void MoreOptions(Mailbox mailbox, PersonalAttendant pa, Message existingMessage) {
        Vector<User> userList = new Vector<User>() ;
        
        MoreOptions:
        for(;;) {
            // "To deliver this message to another address, press 1."
            // "If you are finished, press *."
    
            PromptList pl = m_loc.getPromptList("deposit_more_options");
            String digit = menu(pl, "01*");
    
            // bad entry, timeout or
            // "*" means cancel
            if (digit == null || digit.equals("*")) {
                goodbye();
                return;
            }
    
            // "0" means transfer to operator
            if (digit.equals("0")) {
                transfer(getOperator(pa));
                return;
            }
            
            int timeoutCount = 0;
            for(;;) {
                if (timeoutCount > m_config.getNoInputCount()) {
                    goodbye();
                    return ;
                }
                
                // "Please dial an extension."
                // "Press 8 to use a distribution list,"
                // "Or press 9 to use the dial by name directory."
                m_loc.play("deposit_copy_message", "0123456789*");
                
                Collect c = new Collect(m_fses, 10, m_config.getInitialTimeout(), 
                        m_config.getInterDigitTimeout(), m_config.getExtraDigitTimeout());
                c.setTermChars("#*");
                c.go();
                String digits = c.getDigits();  
                LOG.info("copy message Collected digits=" + digits);
                
                if (digits == null) {
                    timeoutCount++;
                    continue ;
                }
                
                // Reset timeout counter, they entered something
                timeoutCount = 0;

                // "*" means cancel
                if (digits.equals("*")) {
                    continue MoreOptions; // Gotta love lables!  Take that Dijkstra!
                }

                // "0" means transfer to operator
                if (digits.equals("0")) {
                    transfer(getOperator(pa));
                    return;
                }
                
                if (digits.equals("8")) {
                    Vector<User> users = selectDistributionList(mailbox, pa);
                    if (users == null) {
                        continue;
                    }
                    userList.addAll(users);
                    break ;
                    
                } else if (digits.equals("9")) {
                    // Do the DialByName dialog
                    DialByName dbn = new DialByName(m_loc, m_config, m_validUsers);
                    DialByNameChoice choice = dbn.dialByName();
                    
                    // If they canceled DialByName, backup
                    if (choice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                        continue ;
                    }
                    
                    // If an error occurred, hangup
                    if (choice.getUser() == null) {
                        goodbye();
                        continue;
                    }
                    
                    // Add the selected user to the list
                    userList.add(choice.getUser());
                    break ;
                    
                } else {
                    User user = m_validUsers.isValidUser(digits);
                    if (user == null) {
                        // "that extension is not valid"
                        m_loc.play("invalid_extension", "");
                        continue ;
                    }
                    userList.add(user) ;
                }
                break;
            }
             
            // Store the message with each user in the list
            for (User user : userList) {
                Mailbox otherBox = new Mailbox(user, m_loc);
                Message otherMessage = new Message(otherBox, existingMessage);
                otherMessage.storeInInbox();
            }
            m_loc.play("deposit_copied", "");
        }

    }
    
    /**
     * Select a distribution list from the list of lists.  Boy is this confuzing!
     * @param mailbox
     * @param pa
     * @return A list of users on the distribution list, null on error
     */
    Vector<User> selectDistributionList(Mailbox mailbox, PersonalAttendant pa) {
        PromptList pl = m_loc.getPromptList("deposit_select_distribution");
        String digit = menu(pl, "0*");

        // bad entry, timeout or
        // "*" means cancel
        if (digit == null || digit.equals("*")) {
            goodbye(); // TODO * goes back a level, not exit
            return null;
        }

        // "0" means transfer to operator
        if (digit.equals("0")) {
            transfer(getOperator(pa));
            return null;
        }

        // TODO actually select a list!
        return null;
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
     * @throws Throwable
     */
    Record recordMessage(String wavName) {
        // Flush any typed ahead digits
        m_fses.trimDtmfQueue("") ;
        LOG.info(String.format("Recording message (%s)", wavName));
        Record rec = new Record(m_fses, m_loc.getPromptList("beep"));
        rec.setRecordFile(wavName) ;
        rec.setRecordTime(60); // TODO a better time here?
        rec.setDigitMask("0123456789*#"); // Any digit can stop the recording
        rec.go();
        return rec;
    }
    
    String menu(PromptList pl, String validDigits) {
        String digit;
        int invalidCount = 0;
        int timeoutCount = 0;
        for (;;) {
            // Check for a failure condition
            if (invalidCount > m_config.getInvalidResponseCount()
                    || timeoutCount > m_config.getNoInputCount()) {
                failure();
                break;
            }

            // Play the prompts
            Play p = new Play(m_fses, pl);
            p.setDigitMask(validDigits);
            p.go();

            // Wait for the caller to enter a selection.
            Collect c = new Collect(m_fses, 1, m_config.getInitialTimeout(), 0, 0);
            c.setTermChars("#");
            c.go();
            digit = c.getDigits();
            LOG.info("menu Collected digit=" + digit);

            // See if it timed out (no digits)
            if (digit.equals("")) {
                timeoutCount++;
                continue;
            }

            // Check if entered digits match any actions
            if (validDigits.contains(digit)) {
                return digit;
            }

            // What they entered has no corresponding action.
            LOG.info("menu Invalid entry");
            m_loc.play("invalid_try_again", "");
            invalidCount++;
        }
        return null;
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
}
