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
    private static final String FAX_RULE_DESCRIPTION = "Fax Routing rule";

    private String m_voiceMailPrefix = DEFAULT_VMAIL_PREFIX;
    private int m_localExtensionLen = DEFAULT_LOCAL_EXT_LEN;
    private String m_voiceMail = DEFAULT_VOICEMAIL;

    private String m_mediaServerType;
    private MediaServerFactory m_mediaServerFactory;
    private String m_mediaServerHostname;
    private String m_did;

    @Override
    public String[] getPatterns() {
        return null;
    }

    @Override
    public Transform[] getTransforms() {
        return null;
    }

    @Override
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

    @Override
    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        if (StringUtils.isBlank(m_voiceMail)) {
            return;
        }
        MediaServer mediaServer = m_mediaServerFactory.create(m_mediaServerType);
        mediaServer.setHostname(m_mediaServerHostname);
        mediaServer.setServerExtension(m_voiceMail);
        MappingRule voicemail = new MappingRule.Voicemail(m_voiceMail, m_did, mediaServer);
        voicemail.setDescription(getDescription());
        if (getSchedule() != null) {
            voicemail.setSchedule(getSchedule());
        }
        rules.add(voicemail);
        if (StringUtils.isNotBlank(m_voiceMailPrefix)) {
            MappingRule transfer = new MappingRule.VoicemailTransfer(m_voiceMailPrefix, m_localExtensionLen,
                    mediaServer);
            transfer.setDescription(getDescription());
            if (getSchedule() != null) {
                transfer.setSchedule(getSchedule());
            }

            rules.add(transfer);
        }

        MappingRule fallback = new MappingRule.VoicemailFallback(mediaServer);
        fallback.setDescription(getDescription());
        if (getSchedule() != null) {
            fallback.setSchedule(getSchedule());
        }

        rules.add(fallback);

        MappingRule faxForward = new MappingRule.FaxForward(mediaServer);
        faxForward.setDescription(FAX_RULE_DESCRIPTION);
        rules.add(faxForward);
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }

    public String getDid() {
        return m_did;
    }

    public void setDid(String did) {
        m_did = did;
    }

}
