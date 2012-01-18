/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.moh;

import java.util.LinkedHashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.springframework.beans.factory.annotation.Required;

public class MohAddressFactory {
    private static final String LOCAL_FILES_SOURCE_SUFFIX = "l";
    private static final String PORT_AUDIO_SOURCE_SUFFIX = "p";
    private static final String USER_FILES_SOURCE_SUFFIX = "u";
    private static final String NONE_SUFFIX = "n";
    private static final String MOH = "moh";
    private AddressManager m_addressManager;
    private Address m_freeswitchAddress;
    private DomainManager m_domainManager;
    private String m_mohUser;

    public Address getMediaAddress() {
        if (m_freeswitchAddress == null) {
            m_freeswitchAddress = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        }
        return m_freeswitchAddress;
    }

    public String getFilesMohUser() {
        return m_mohUser + USER_FILES_SOURCE_SUFFIX;
    }

    public String getPersonalMohFilesUri(String userName) {
        return getMohUri(m_mohUser + USER_FILES_SOURCE_SUFFIX + userName);
    }

    public String getPortAudioMohUri() {
        return getMohUri(m_mohUser + PORT_AUDIO_SOURCE_SUFFIX);
    }

    public String getLocalFilesMohUri() {
        return getMohUri(m_mohUser + LOCAL_FILES_SOURCE_SUFFIX);
    }

    public String getNoneMohUri() {
        return getMohUri(m_mohUser + NONE_SUFFIX);
    }

    /**
     * The Moh URI used by the system
     */
    public String getDefaultMohUri() {
        return getMohUri(m_mohUser);
    }


    public String getPortAudioMohUriMapping() {
        return getMohUriMapping(PORT_AUDIO_SOURCE_SUFFIX);
    }

    public String getLocalFilesMohUriMapping() {
        return getMohUriMapping(LOCAL_FILES_SOURCE_SUFFIX);
    }

    public String getNoneMohUriMapping() {
        return getMohUriMapping(NONE_SUFFIX);
    }

    /**
     * Build an alias which maps directly to the MOH server
     *
     * IVR@{FS}:{FSPort};action=moh; add "moh=l" for localstream files add "moh=p" for portaudio
     * (sound card) add "moh=u{username} for personal audio files add "moh=n" for disabled MOH
     *
     */
    private String getMohUriMapping(String mohParam) {
        Map<String, String> params = new LinkedHashMap<String, String>();
        params.put("action", MOH);
        if (mohParam != null) {
            params.put(MOH, mohParam);
        }
        return SipUri.format("IVR", getMediaAddress().toString(), params);
    }

    private String getMohUri(String mohParam) {
        return SipUri.format(mohParam, m_domainManager.getDomainName(), false);
    }

    @Required
    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
