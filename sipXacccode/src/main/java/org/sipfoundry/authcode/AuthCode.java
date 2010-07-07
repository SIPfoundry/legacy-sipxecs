/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.authcode;

import java.util.Collection;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.Play;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.freeswitch.Transfer;
import org.sipfoundry.sipxacccode.AccCodeConfiguration;

public class AuthCode {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxauthcode");

    // Global store for resource bundles keyed by locale
    private static final String RESOURCE_NAME = "org.sipfoundry.authcode.AuthCode";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();
    
    private AccCodeConfiguration m_acccodeConfig;
    private Localization m_loc;
    private FreeSwitchEventSocketInterface m_fses;
    private ResourceBundle m_authBundle;
    private Configuration m_config;
    private String m_localeString;

    private Hashtable<String, String> m_parameters;  // The parameters from the sip URI
    
    /**
     * Create an AuthCode object.
     * 
     * @param acccodeConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI (to determine locale and which action
     *        to run)
     */
    public AuthCode(AccCodeConfiguration acccodeConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_acccodeConfig = acccodeConfig;
        this.m_fses = fses;
        this.m_parameters = parameters;
        
        // Look for "locale" parameter
        String m_localeString = m_parameters.get("locale");
        if (m_localeString == null) {
            // Okay, try "lang" instead
            m_localeString = m_parameters.get("lang");
        }
        if (m_localeString == null) {
            // Default to english
            m_localeString = "en";
        }
    }

    /**
     * Load all the needed configuration.
     * 
     */
    void loadConfig() {

        // Load the account code configuration
        m_loc = new Localization(RESOURCE_NAME,
                                   m_localeString, s_resourcesByLocale, m_acccodeConfig, m_fses);
        m_config = Configuration.update(true);

    }

    /**
     * Given an extension, convert to a full URL in this domain
     * 
     * @param extension
     * @return the Url
     */
    public String extensionToUrl(String extension) {
        return "sip:" + extension + "@" + m_acccodeConfig.getSipxchangeDomainName();
    }
    
    public String paramAddToUrl(String url, String name, String value) {
       if (url.toLowerCase().contains("?")) {
          return url + ";" + name + "=" + value;
       } else {
          return url + "?" + name + "=" + value;
       }
    }
    /**
     * Run Authorization Code for each call.
     *
     */
    public void run() throws Throwable {
        if (m_loc == null) {
            loadConfig();
        }

        // Wait a bit so audio doesn't start too fast
        Sleep s = new Sleep(m_fses, 1000);
        s.go();
        // Play the feature start tone.
        m_fses.setRedactDTMF(false);
        m_fses.trimDtmfQueue("") ; // Flush the DTMF queue

        m_loc.play("AuthCode_please_enter", "0123456789#*");

        String currentauthorization;
        String destdial = "";

        currentauthorization = getAuthcode();
        LOG.info("AuthCode entered = " + currentauthorization);
        if (currentauthorization != null) {
            AuthCodeConfig authcode = m_config.getAuthCode(currentauthorization);   
            destdial = getDestDial();
            if (destdial != null) {
                // Build a SIP URL and transfer the call to this destination.
                String destURL = extensionToUrl(destdial);
                //Add account code to URL
                transfer(destURL, authcode.getAuthName(), authcode.getAuthPassword());
            }
        }
        LOG.info("Ending AuthCode");
    }

    /**
     * Do the prompting for Authorization Code.
     * 
     */
    private String getAuthcode() {
        String codeEntered;
        Localization usrLoc = m_loc;
        return codeEntered = new AuthEnter(this).enterAuthorizationCode();
    }


    /**
     * Get the Destination Dial String.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    String getDestDial() {
               
        Localization usrLoc = m_loc;
        return new DestinationEnter(this).enterDestinationDialString();
    }


    /**
     * Transfer the call to the indicated destination.
     * 
     * @param uri
     */
    public void transfer(String uri, String user, String passwd) {
        // Set the SIP authorized username and password to be used for the challenge and
        // for call permissions that will be used.
        new Set(m_fses, "sip_auth_username", user).go();
        new Set(m_fses, "sip_auth_password", passwd).go();
        Transfer xfer = new Transfer(m_fses, uri);
        xfer.go();
        throw new DisconnectException();
    }

    public void failure() {
       LOG.info("failure");
       goodbye();
    }
    /**
     * Play a good bye tone and hangup.
     * 
     */
    public void goodbye() {
        // Play an error hang up tone. 
        m_loc.play("AuthCode_error_hang_up", "");
        new Hangup(m_fses).go();
    }

    public void setAuthorizationCodeBundle(ResourceBundle authcodeBundle) {
        m_authBundle = authcodeBundle;
    }

    public Configuration getConfig() {
        return m_config;
    }

    public void setConfig(Configuration config) {
        m_config = config;
    }

    public void setLocalization(Localization localization) {
        m_loc = localization;
    }
    
    public Localization getLoc() {
        return(m_loc);
    }
    
    public void playError(String errPrompt, String ...vars) {
        m_loc.play("error_beep", "");
        m_fses.trimDtmfQueue("");
        m_loc.play(errPrompt, "0123456789*#", vars);
    }
    
}
