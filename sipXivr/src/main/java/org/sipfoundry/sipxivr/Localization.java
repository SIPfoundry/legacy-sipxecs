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
     * The Bundle with the resources is located based on locale, as is the TextToPrompts
     * class. 
     * 
     */
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    
    private String m_bundleName;
    private HashMap<Locale, ResourceBundle> m_resourcesByLocale;
    private ResourceBundle m_bundle;
    private Locale m_locale;
    private TextToPrompts m_ttp;
    private Configuration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;

    public Localization(String bundleName, String localeString, HashMap<Locale, ResourceBundle> resourcesByLocale,
            Configuration ivrConfig, FreeSwitchEventSocketInterface fses) {
        
        // Load the resources for the given locale.

        m_bundleName = bundleName;
        m_resourcesByLocale = resourcesByLocale;
        m_locale = Locale.US; // Default to good ol' US of A
        m_ivrConfig = ivrConfig;
        m_fses = fses;
        changeLocale(localeString);
    }
    
    /**
     * Copy a new Localization with a different bundle, but the rest is the same
     * @param bundleName
     * @param origLoc
     */
    public Localization(String bundleName, HashMap<Locale, ResourceBundle> resourcesByLocale, Localization origLoc) {
        // Load the resources for the given locale.

        m_bundleName = bundleName;
        m_resourcesByLocale = resourcesByLocale;
        m_locale = origLoc.m_locale;
        m_ivrConfig = origLoc.m_ivrConfig;
        m_fses = origLoc.m_fses;
        changeLocale(null);
    }
    
    public void changeLocale(String localeString) {
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
        synchronized (m_resourcesByLocale) {
            String fromWhere = "cache";
            ResourceBundle newBundle = m_resourcesByLocale.get(m_locale);
            if (newBundle == null) {
                fromWhere = "disk";
                // Nope. Find the on disk version, and keep it for next time.
                newBundle = ResourceBundle.getBundle(
                        m_bundleName, m_locale);
                m_resourcesByLocale.put(m_locale, newBundle);
            }
            if (newBundle != null) {
                LOG.debug(String.format("Localization::changeLocale Loaded resource bundle %s from %s",
                        m_bundleName, fromWhere));
                m_bundle = newBundle;
            } else {
                LOG.warn("Localization::changeLocale Cannot find resource bundle "+m_bundleName+" for locale "+m_locale);
            }
        }

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

    public Configuration getIvrConfig() {
        return m_ivrConfig;
    }
    
    public FreeSwitchEventSocketInterface getFreeSwitchEventSocketInterface() {
        return m_fses;
    }
    
    /**
     * Helper function to get the PromptList from the bundle.
     * 
     * @return The appropriate PromptList.
     */
    public PromptList getPromptList() {
        PromptList pl = new PromptList(this);
        return pl;
    }

    /**
     * Helper function to get the PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param vars
     * @return The appropriate PromptList.
     */
    public PromptList getPromptList(String fragment, String... vars) {
        PromptList pl = getPromptList();
        pl.addFragment(fragment, vars);
        return pl;
    }

    /**
     * Helper function to get a Player with a PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param digitMask
     * @param vars
     * @return
     */
    public Play getPlayer(String fragment, String digitMask, String... vars) {
        Play p = new Play(m_fses);
        p.setPromptList(getPromptList(fragment, vars));
        p.setDigitMask(digitMask);
        return p;
    }

    /**
     * Helper function to just Play with a PromptList from the bundle given a fragment name.
     * 
     * @param fragment
     * @param digitMask
     * @param vars
     */
    public void play(String fragment, String digitMask, String... vars) {
        Play p = new Play(m_fses);
        p.setPromptList(getPromptList(fragment, vars));
        p.setDigitMask(digitMask);
        p.go();
    }

    /**
     * Helper function to just Play given a PromptList
     * 
     * @param prompt list
     * @param digitMask
     */
    public void play(PromptList pl, String digitMask) {
        Play p = new Play(m_fses);
        p.setPromptList(pl);
        p.setDigitMask(digitMask);
        p.go();
    }

}
