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

import org.apache.log4j.Logger;

public class Localization {
    /**
     * Load and hold all the needed configuration.
     * 
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. 
     * 
     */
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    private ResourceBundle m_bundle;
    private Locale m_locale;
    private TextToPrompts m_ttp;
    private org.sipfoundry.sipxivr.Configuration m_ivrConfig;

    public Localization(String bundleName, String localeString, HashMap<Locale, ResourceBundle> resourcesByLocale,
            org.sipfoundry.sipxivr.Configuration ivrConfig) {
        
        // Load the resources for the given locale.

        m_locale = Locale.US; // Default to good ol' US of A
        m_ivrConfig = ivrConfig;
        
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

        // Check to see if we've loaded this one before...
        synchronized (resourcesByLocale) {
            m_bundle = resourcesByLocale.get(m_locale);
            if (m_bundle == null) {
                // Nope. Find the on disk version, and keep it for next time.
                m_bundle = ResourceBundle.getBundle(
                        bundleName, m_locale);
                resourcesByLocale.put(m_locale, m_bundle);
            }
        }
        LOG.debug("Loaded resource bundle "+m_bundle.getClass().getSimpleName());

        // Find the TextToPrompt class as well
        m_ttp = TextToPrompts.getTextToPrompt(m_locale);
        // Tell it where to find the audio files
        String globalPrefix = m_bundle.getString("global.prefix");
        if (!globalPrefix.startsWith("/")) {
            String docDir = m_ivrConfig.getDocDirectory();
            if (!docDir.endsWith("/")) {
                docDir += "/";
            }
            globalPrefix = docDir + globalPrefix;
        }
        m_ttp.setPrefix(globalPrefix);
    }

    public ResourceBundle getBundle() {
        return m_bundle;
    }

    public Locale getLocale() {
        return m_locale;
    }

    public TextToPrompts getTtp() {
        return m_ttp;
    }

    public org.sipfoundry.sipxivr.Configuration getIvrConfig() {
        return m_ivrConfig;
    }


}
