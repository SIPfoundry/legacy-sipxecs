/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * InternalRule
 */
public class InternalRule extends DialingRule {
    public static final String DEFAULT_VOICEMAIL = "101";
    private static final String DEFAULT_VMAIL_PREFIX = "8";
    private static final int DEFAULT_LOCAL_EXT_LEN = 3;

    private String m_voiceMailPrefix = DEFAULT_VMAIL_PREFIX;
    private int m_localExtensionLen = DEFAULT_LOCAL_EXT_LEN;
    private String m_voiceMail = DEFAULT_VOICEMAIL;

    private String m_mediaServerType;
    private MediaServerFactory m_mediaServerFactory;
    private String m_mediaServerHostname;

    public String[] getPatterns() {
        return null;
    }

    public Transform[] getTransforms() {
        return null;
    }

    public DialingRuleType getType() {
        return DialingRuleType.INTERNAL;
    }

    public int getLocalExtensionLen() {
        return m_localExtensionLen;
    }

    public void setLocalExtensionLen(int localExtensionLen) {
        m_localExtensionLen = localExtensionLen;
    }

    /** Return the voicemail extension */
    public String getVoiceMail() {
        return m_voiceMail;
    }

    /** Set the voicemail extension */
    public void setVoiceMail(String voiceMail) {
        m_voiceMail = voiceMail;
    }

    public String getVoiceMailPrefix() {
        return m_voiceMailPrefix;
    }

    public void setVoiceMailPrefix(String voiceMailPrefix) {
        m_voiceMailPrefix = voiceMailPrefix;
    }

    public String getMediaServerType() {
        return m_mediaServerType;
    }

    public void setMediaServerType(String serverType) {
        m_mediaServerType = serverType;
    }

    public Collection<String> getAvailableMediaServers() {
        return m_mediaServerFactory.getBeanIds();
    }

    public String getMediaServerHostname() {
        return m_mediaServerHostname;
    }

    public void setMediaServerHostname(String hostname) {
        m_mediaServerHostname = hostname;
    }

    public void setMediaServerFactory(MediaServerFactory mediaServerFactory) {
        m_mediaServerFactory = mediaServerFactory;
    }

    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        boolean generateVoiceMailRules = StringUtils.isNotBlank(m_voiceMail);
        if (generateVoiceMailRules) {
            MediaServer mediaServer = m_mediaServerFactory.create(m_mediaServerType);
            mediaServer.setHostname(m_mediaServerHostname);
            mediaServer.setServerExtension(m_voiceMail);
            MappingRule voicemail = new MappingRule.Voicemail(m_voiceMail, mediaServer);
            voicemail.setDescription(getDescription());
            rules.add(voicemail);

            if (StringUtils.isNotBlank(m_voiceMailPrefix)) {
                MappingRule transfer = new MappingRule.VoicemailTransfer(m_voiceMailPrefix,
                        m_localExtensionLen, mediaServer);
                transfer.setDescription(getDescription());
                rules.add(transfer);
            }

            MappingRule redirect = new MappingRule.VoicemailRedirect(m_localExtensionLen);
            redirect.setDescription(getDescription());
            rules.add(redirect);
            
            // pass -1 to generate fallback rule that matches any extension
            MappingRule fallback = new MappingRule.VoicemailFallback(-1, mediaServer);
            fallback.setDescription(getDescription());
            rules.add(fallback);
        }
    }

    public boolean isInternal() {
        return true;
    }
}
