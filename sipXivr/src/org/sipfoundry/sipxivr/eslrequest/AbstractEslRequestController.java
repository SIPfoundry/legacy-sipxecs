/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxivr.eslrequest;

import java.util.Hashtable;
import java.util.Locale;
import java.util.MissingResourceException;

import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.DisconnectException;
import org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.freeswitch.Transfer;

public abstract class AbstractEslRequestController implements EslRequestController {
    private FreeSwitchConfigurationInterface m_fsConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private Localization m_localization;
    private String m_uuid;
    private String m_localeString;
    private String m_sipxChangeDomainName;

    public abstract void extractParameters(Hashtable<String, String> parameters);

    public abstract void loadConfig();

    @Override
    public final void init(Hashtable<String, String> parameters) {
        extractCommonParameters(parameters);
        extractParameters(parameters);
        loadConfig();
    }

    public void initLocalization(String resourceName, String alternate) {
        try {
            m_localization = new Localization(resourceName, m_localeString, m_fsConfig, m_fses);
        } catch (MissingResourceException e) {
            if (alternate != null) {
                m_localization = new Localization(alternate, m_localeString, m_fsConfig, m_fses);
            }
        }  
    }

    private void extractCommonParameters(Hashtable<String, String> parameters) {
        m_uuid = parameters.get("uuid");
        // Look for "locale" parameter
        m_localeString = parameters.get("locale");
        if (m_localeString == null) {
            // Okay, try "lang" instead
            m_localeString = parameters.get("lang");
        }
    }

    public String collectDigits(int maxDigits, int firstDigitTimer, int interDigitTimer, int extraDigitTimer,
            String termChars) {
        Collect collect = new Collect(getFsEventSocket(), maxDigits, firstDigitTimer, interDigitTimer,
                extraDigitTimer);
        collect.setTermChars(termChars);
        collect.go();
        return collect.getDigits();
    }

    public void sleep(int ms) {
        Sleep s = new Sleep(m_fses, ms);
        s.go();
    }

    public void play(PromptList pl, String digitMask) {
        m_localization.play(pl, "0123456789#*");
    }

    public void play(String fragment, String digitMask) {
        m_localization.play(fragment, digitMask);
    }

    public void transfer(String dest, boolean playGreeting) {
        transfer(dest, playGreeting, false);
    }

    public void transfer(String dest, boolean playGreeting, boolean disconnect) {
        if (playGreeting) {
            m_localization.play("please_hold", "");
        }
        Transfer xfer;
        if (m_uuid != null) {
            xfer = new Transfer(m_fses, m_uuid, dest);
        } else {
            xfer = new Transfer(m_fses, dest);
        }
        xfer.go();
        if (disconnect) {
            throw new DisconnectException();
        }
    }

    public void goodbye() {
        goodbye(true);
    }

    public void goodbye(boolean play) {
        if (play) {
            m_localization.play("goodbye", "");
        }
        hangup();
    }

    public void hangup() {
        Hangup h = new Hangup(m_fses);
        h.go();
    }

    public void set(String variable, String value) {
        Set set = new Set(getFsEventSocket(), m_fses.getVariable("Unique-ID"), variable, value);
        set.start();
    }

    public void invokeSet(String variable, String value) {
        new Set(m_fses, variable, value).go();
    }

    public void playError(String errPrompt, String... vars) {
        m_localization.play("error_beep", "");
        m_fses.trimDtmfQueue("");
        m_localization.play(errPrompt, "0123456789*#", vars);
    }

    public void changeLocale(String localeString) {
        m_localization.changeLocale(localeString);
    }

    public void setRedactDTMF(boolean redactDTMF) {
        m_fses.setRedactDTMF(redactDTMF);
    }

    public void trimDtmfQueue(String digitMask) {
        m_fses.trimDtmfQueue(digitMask);
    }

    public String getDisplayUri() {
        return m_fses.getDisplayUri();
    }

    public Locale getLocale(String localeString) {
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
        return new Locale(lang, country, variant);
    }

    public String getCallerUniqueId() {
        return getVariable("Caller-Unique-ID");
    }

    public String getSipReqParams() {
        return getVariable("variable_sip_req_params");
    }

    public String getSipReqUri() {
        return getVariable("variable_sip_req_uri");
    }

    public String getChannelCallerIdName() {
        return getVariable("channel-caller-id-name");
    }

    public String getChannelCallerIdNumber() {
        return getVariable("channel-caller-id-number");
    }

    public String getCallForwardReason() {
        return getVariable("call-forward-reason");
    }

    public String getChannelUniqueId() {
        return getVariable("channel-unique-id");
    }

    private String getVariable(String key) {
        return m_fses.getVariable(key);
    }

    /**
     * Given an extension, convert to a full URL in this domain
     * 
     * @param extension
     * @return the Url
     */
    public String extensionToUrl(String extension) {
        return "sip:" + extension + "@" + m_sipxChangeDomainName;
    }

    public void setFsConfig(FreeSwitchConfigurationInterface config) {
        m_fsConfig = config;
    }

    public FreeSwitchConfigurationInterface getFsConfig() {
        return m_fsConfig;
    }

    public void setFsEventSocket(FreeSwitchEventSocketInterface fses) {
        m_fses = fses;
    }

    public FreeSwitchEventSocketInterface getFsEventSocket() {
        return m_fses;
    }

    public Localization getLocalization() {
        return m_localization;
    }

    public void setLocalization(Localization localization) {
        m_localization = localization;
    }

    public Locale getLocale() {
        return m_localization.getLocale();
    }

    public PromptList getPromptList() {
        return m_localization.getPromptList();
    }

    public PromptList getPromptList(String fragment) {
        return m_localization.getPromptList(fragment);
    }

    public PromptList getPromptList(String fragment, String ...vars) {
        return m_localization.getPromptList(fragment, vars);
    }

    public String getUuid() {
        return m_uuid;
    }

    public void setUuid(String uuid) {
        m_uuid = uuid;
    }

    public String getLocaleString() {
        return m_localeString;
    }

    public void setLocaleString(String locale) {
        m_localeString = locale;
    }

    public void setSipxchangeDomainName(String domainName) {
        m_sipxChangeDomainName = domainName;
    }

    public String getSipxchangeDomainName() {
        return m_sipxChangeDomainName;
    }

}
