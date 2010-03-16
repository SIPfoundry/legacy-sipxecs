/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.commons.freeswitch;

import java.io.File;
import java.util.HashMap;
import java.util.Locale;
import java.util.MissingResourceException;
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
    private final Logger LOG;
    private String m_bundleName;
    private HashMap<Locale, ResourceBundle> m_resourcesByLocale;
    private ResourceBundle m_bundle;
    private Locale m_locale;
    private TextToPrompts m_ttp;
    private FreeSwitchConfigurationInterface m_config;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_prefix;
    private String m_localeString;

    public Localization(String bundleName, String localeString, HashMap<Locale, ResourceBundle> resourcesByLocale,
            FreeSwitchConfigurationInterface config, FreeSwitchEventSocketInterface fses) {

        // Load the resources for the given locale.

        m_bundleName = bundleName;
        m_resourcesByLocale = resourcesByLocale;
        m_locale = Locale.US; // Default to good ol' US of A
        m_config = config;
        m_fses = fses;
        LOG = config.getLogger();
        changeLocale(localeString);
        m_localeString = localeString;
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
        m_config = origLoc.m_config;
        m_fses = origLoc.m_fses;
        LOG = origLoc.LOG;
        m_localeString = origLoc.m_localeString;
        changeLocale(m_localeString);
    }

    public void changeLocale(String localeString) {
        // A locale was passed in . Parse it into parts and find a Locale to match
        if (localeString != null) {
            // convert any dashes to underscores, as sometimes locales are misrepresented
            String ls = localeString.replace('-', '_');
            String[] localeElements = ls.split("_");
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
            LOG.debug(String.format("Localization::changeLocale to %s,%s,%s found %s",
                    lang, country, variant, m_locale.toString()));
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
                LOG.debug(String.format("Localization:changeLocale Loaded resource bundle %s from %s",
                        m_bundleName, fromWhere));
                m_bundle = newBundle;
            } else {
                LOG.warn("Localization::changeLocale Cannot find resource bundle "+m_bundleName+" for locale "+m_locale);
            }
        }

        String ttpClass;
        String docDir = "";
        String tmpGlobalPrefix = "";
        try {
            ttpClass = m_bundle.getString("global.ttpClassName");
        } catch (MissingResourceException e) {
            ttpClass = null;
        }
        
        // Find the TextToPrompt class as well
        m_ttp = TextToPrompts.getTextToPrompt(m_locale, ttpClass);
        
        // Tell it where to find the audio files
        String globalPrefix = m_bundle.getString("global.prefix");
        if (globalPrefix.endsWith("/")) {
            // Trim trailing "/" (will put it back in a bit)
            globalPrefix = globalPrefix.substring(0, globalPrefix.length()-1);
        }
        
        if (!globalPrefix.startsWith("/")) {
            docDir = m_config.getDocDirectory();
            if (!docDir.endsWith("/")) {
                docDir += "/";
            }
            tmpGlobalPrefix = globalPrefix;
            globalPrefix = docDir + globalPrefix;
        }

        if (m_bundle != null) {
            Locale actual = m_bundle.getLocale();
            if (actual == null || !actual.equals(m_locale)) {
                // This bundle's locale isn't exactly what we asked for.

                // If the path doesn't end with the locale, add the locale
                // to it.  This is in case the Properties file isn't there
                // and the top level one was picked up instead.  The prompts
                // are still there, just not the .properties file
                if (localeString != null) {
                    String suffix = "_"+localeString;
                    String path= "";
                    if (!globalPrefix.endsWith(suffix)) {
                        if (tmpGlobalPrefix.indexOf("/")>=0) // call pilot TUI
                            path = docDir + tmpGlobalPrefix.replaceFirst("/", suffix+"/");
                        else	// standard TUI
                            path = globalPrefix + suffix;
                        File testFile = new File(path);
                        if (testFile.exists()) {
                            globalPrefix = path;
                        }
                    }
                }
            }
        }
        globalPrefix += "/";
        m_prefix = globalPrefix;
        m_ttp.setPrefix(m_prefix);
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

    public void setTtp(TextToPrompts ttp) {
    	m_ttp = ttp;
    }
    
    public FreeSwitchConfigurationInterface getConfig() {
        return m_config;
    }

    public FreeSwitchEventSocketInterface getFreeSwitchEventSocketInterface() {
        return m_fses;
    }

    public String getPrefix() {
        return m_prefix;
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
