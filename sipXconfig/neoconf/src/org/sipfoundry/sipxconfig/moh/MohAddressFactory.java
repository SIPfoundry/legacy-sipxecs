/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.moh;

import java.util.LinkedHashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.ivr.Ivr;
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
        return getMediaAddress(null);
    }

    public Address getMediaAddress(Location location) {
        Address addr = null;
        if (location == null) {
            addr = m_addressManager.getSingleAddress(Ivr.SIP_ADDRESS);
        } else {
            addr = m_addressManager.getSingleAddress(Ivr.SIP_ADDRESS, location);
        }
        if (addr != null) {
            return addr;
        }
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
        return getMohUri(getPortAudioUser());
    }

    public String getLocalFilesMohUri() {
        return getMohUri(getLocalFilesUser());
    }

    public String getNoneMohUri() {
        return getMohUri(getNoneUser());
    }

    public String getDefaultUser() {
        return m_mohUser;
    }

    public String getLocalFilesUser() {
        return m_mohUser + LOCAL_FILES_SOURCE_SUFFIX;
    }

    public String getPortAudioUser() {
        return m_mohUser + PORT_AUDIO_SOURCE_SUFFIX;
    }

    public String getNoneUser() {
        return m_mohUser + NONE_SUFFIX;
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
        return SipUri.format("IVR", getMediaAddress().getAddress(), params);
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
