/*
 * 
 * 
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.authcode;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Vector;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.sipxacccode.DigitCollect;

public class DestinationEnter {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxauthcode");

    AuthCode m_authcode;
    Localization m_loc;
    FreeSwitchEventSocketInterface m_fses;
    String m_userEnteredDestination;        // Destination Dial String entered.
    
    public DestinationEnter(AuthCode authcode) {
        m_authcode = authcode;
        m_loc = authcode.getLoc();
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
    }
    
    public String enterDestinationDialString() {

        String displayUri = m_fses.getDisplayUri();
        LOG.debug("DestinationEnter::enterDestinationDialString - Enter Destination Dial String from "+displayUri);

        m_fses.setRedactDTMF(false);
        m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
        String destdialstring = getDestinationDigits() ;
        LOG.debug("Destination dial string = " + destdialstring);
        return destdialstring ;
    }
    
    /**
     * Get the Destination Dial String.
     * 
     * @return
     */
    private String getDestinationDigits() {
        int errorCount = 0;
        String destdialstr = null;

        // Prompt to enter the destination dial string to use.
        PromptList dialstringPl = m_loc.getPromptList("AuthCode_please_enter_dialstring");

        for(;;) {
            if (errorCount > 1) {
                m_authcode.failure();
                return null;
            }

            // Note:  Not using collectDigits() here, as it doesn't allow initial "#" to barge,
            // and "*" to cancel doesn't really make sense.  Just treat as invalid.
            DigitCollect dc = new DigitCollect(m_loc);
            destdialstr = dc.collectDtmf(dialstringPl, 24);
            
            if (destdialstr == "") {
                return null;
            }
            
            if (destdialstr == null) {
                // WRONG, do it again!
                
                m_loc.play("AuthCode_invalid", "");
                ++errorCount;
                continue;
            }
            m_userEnteredDestination = destdialstr;
            break ;
        }
        return m_userEnteredDestination;
    }
}
