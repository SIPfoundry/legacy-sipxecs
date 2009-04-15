/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

import java.util.HashMap;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.IvrChoice.IvrChoiceReason;

public class DialByName {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for DialByNamePrompts resource bundles keyed by locale
    private static final String RESOURCE_NAME = "org.sipfoundry.sipxivr.DialByNamePrompts";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private Localization m_loc;
    private ApplicationConfiguraton m_config;
    private ValidUsersXML m_validUsers;

    public DialByName(Localization localization, ApplicationConfiguraton config, ValidUsersXML validUsers) {
        // Load the DialByNameProperties bundle
        m_loc = new Localization(RESOURCE_NAME, s_resourcesByLocale, localization);
        m_config = config;
        m_validUsers= validUsers;
    }
    
    /**
     * Given the dialed digits "spelling" a user name, create a menu of matching users (up to 9)
     * and have the caller enter one of the selections.
     * 
     * @param digits
     * @return The selected user, null if none
     */
    DialByNameChoice selectChoice(String digits){
        // Lookup the list of validUsers that match the DTMF digits
        Vector<User> matches = m_validUsers.lookupDTMF(digits);

        if (matches.size() == 0) {
            // Indicate no match
            // "No name in the directory matches the name you entered."
            m_loc.play("dial_by_name_nomatch", "");
            // create a SUCCESS return as they didn't timeout or enter bad data
            // but the choice is null
            return new DialByNameChoice(null, digits, IvrChoiceReason.SUCCESS);
        }

        /*
         * This is an enhancement over the original vxml which will prompt even if only one
         * matches. if (matches.size() == 1) { User u = matches.firstElement() ; transfer(u.uri);
         * return true ; }
         */

        // Build a menu of the matched user's names.
        // Limit the choices to the first 9 (or it gets too long)
        PromptList pl = new PromptList(m_loc);
        StringBuilder digitMask = new StringBuilder();
        int choices = matches.size();
        if (choices > 9) {
            choices = 9;
        }
        for (int i = 0; i < choices; i++) {
            String digit = Integer.toString(i+1);

            User u = matches.get(i);
            // Try to speak the user's recorded name
            String recordedName = new Mailbox(u, m_loc).getRecordedName();
            String namePrompts;
            if (recordedName != null) {
                namePrompts = recordedName;
            } else {
                PromptList ext = new PromptList(m_loc);
                // "Extension {extension}"
                ext.addFragment("extension", u.getUserName());
                namePrompts = ext.toString();
            }
            // "Press {number} for {name}"
            pl.addFragment("press_n_for", digit, namePrompts);
            digitMask.append(digit);
        }
        // "To cancel and enter a different name, press *."
        pl.addFragment("dial_by_name_enter_different_name");

        // Dialog for the caller to enter one of the choices
        int timeoutCount = 0;
        for (;;) {
            if (timeoutCount > m_config.getNoInputCount()) {
                return new DialByNameChoice(null, "", IvrChoiceReason.FAILURE);
            }

            // Play the menu
            m_loc.play(pl, digitMask+"*");

            // Wait for the caller to enter a digit
            Collect c = new Collect(m_loc.getFreeSwitchEventSocketInterface(), 1, 
                    m_config.getInitialTimeout(), 0, 0);
            c.setTermChars("*#");
            c.go();
            String choice = c.getDigits();
            LOG.info("DialByName::selectChoice Collected digits=" + choice);

            // See what they entered
            if (choice.length() == 0) {
                timeoutCount++;
                continue;
            }

            // Reset timeout counter, they entered something
            timeoutCount = 0;

            if (choice.contentEquals("*")) {
                // "Canceled."
                m_loc.play("canceled", "");
                return new DialByNameChoice(null, "*", IvrChoiceReason.CANCELED);
            }

            if (choice.contentEquals("#")) {
                // "No entry matches that selection."
                m_loc.play("no_entry_matches", "");
                continue;
            }

            if ("123456789".contains(choice)) {
                int selected = Integer.parseInt(choice);
                if (selected <= choices) {
                    User u = matches.get(selected - 1);
                    LOG.info(String.format("DialByName::selectChoice returns extension %s (%s)", u.getUserName(), u.getUri()));
                    return new DialByNameChoice(u, choice, IvrChoiceReason.SUCCESS);
                }
            }

            // "No entry matches that selection."
            m_loc.play("no_entry_matches", "");
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
     * @return User selected by dialog, else null
     */
    public DialByNameChoice dialByName() {
        int timeoutCount = 0;
        for (;;) {
            
            if (timeoutCount > m_config.getNoInputCount()) {
                return new DialByNameChoice(null, "", IvrChoiceReason.FAILURE);
            }

            // "Please spell the name of the person, last name first. 
            // "Press seven for 'q' and nine for 'z' To cancel, press star"
            m_loc.play("dial_by_name", "0123456789*");

            // Collect the digits from the caller.  Use the "*" and "#" keys as terminators
            // There are a LONG (10 second) digit timers here, as spelling on the phone
            // is difficult!  The "#" key will terminate any input if the caller is finished.
            Collect c = new Collect(m_loc.getFreeSwitchEventSocketInterface(), 20, 10000, 10000, 2000);
            c.setTermChars("*#");
            c.go();
            String digits = c.getDigits();
            LOG.info("DialByName::dialByName Collected digits=" + digits);

            // Timed out.  (No digits)
            if (digits.length() == 0) {
                ++timeoutCount;
                continue;
            }
            
            // Reset the timeout counter, they entered something
            timeoutCount = 0 ;

            if (digits.contentEquals("*")) {
                // "Canceled."
                m_loc.play("canceled", "");
                return new DialByNameChoice(null, "*", IvrChoiceReason.CANCELED);
            }

            if (digits.contentEquals("0") || digits.contentEquals("1")) {
                // "That extension is not valid"
                m_loc.play("invalid_try_again", "");
                continue;
            }
            
            // See if the digits they dialed matched anyone, and let them select among
            // all the possibilities
            DialByNameChoice choice = selectChoice(digits);

            if (choice.getIvrChoiceReason() == IvrChoiceReason.CANCELED) {
                // They canceled the selectChoice menu, back to dialByName
                continue ;
            }
            
            if (choice.getIvrChoiceReason() == IvrChoiceReason.SUCCESS && 
                    choice.getUser() == null) {
                // Matched no names
                continue;
            }
            return choice;
        }
    }

}
