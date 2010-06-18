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
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.sipxacccode.DigitCollect;

public class AuthEnter {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxauthcode");

    private int maxErrors = 2;
    private AuthCode m_authcode;
    private Localization m_loc;
    private Configuration m_config;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_userEnteredAuth;        // Authorization Code entered
    
    public AuthEnter(AuthCode authcode) {
        m_authcode = authcode;
        m_loc = authcode.getLoc();
        m_config = authcode.getConfig();
        m_fses = m_loc.getFreeSwitchEventSocketInterface();
    }
    
    public String enterAuthorizationCode() {

        String displayUri = m_fses.getDisplayUri();
        LOG.debug("AuthEnter::enterAuthorizationCode  - Enter Authorization Code from "+displayUri);

//        m_fses.setRedactDTMF(false);
//        m_fses.trimDtmfQueue("") ; // Flush the DTMF queue
        String account = getAuthdigits() ;
        return account ;
    }
    
    /**
     * Get the Authorization Code to use.
     * 
     * @return
     */
    private String getAuthdigits() {
        int errorCount = 0;
        String authdigstr = null;

        // Prompt to enter the authorization code to use.
        // PromptList authcodePl = m_loc.getPromptList("AuthCode_please_enter");
        PromptList authcodePl = new PromptList(m_loc);
        boolean playAcctPrompt = true;

        for(;;) {
            if (errorCount > (maxErrors - 1)) {
                m_authcode.failure();
                return null;
            }
            DigitCollect dc = new DigitCollect(m_loc);
            authdigstr = dc.collectDtmf(authcodePl, 10);
            
            if ((authdigstr == null)  || (authdigstr == "") || (m_config.getAuthCode(authdigstr) == null)) {
                // The authorization code entered is not valid, play the invalid tone
                m_loc.play("AuthCode_invalid", "");
                new Sleep(m_fses, 1000).go();
                ++errorCount;
                continue;
            }
            m_userEnteredAuth = authdigstr;
            break ;
        }
        LOG.debug("Authorization code successfully entered");
        return m_userEnteredAuth;
    }
    
}
