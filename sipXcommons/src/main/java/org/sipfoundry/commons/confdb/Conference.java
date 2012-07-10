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
package org.sipfoundry.commons.confdb;

import org.springframework.data.annotation.Id;

public class Conference {
    @Id
    private String m_id;

    private String m_confOwner;
    private String m_confName;
    private String m_confDescription;
    private boolean m_moderated;
    private boolean m_public;
    private boolean m_membersOnly;
    private boolean m_autoRecord;
    private String m_extension;
    private String m_pin;
    private String m_uri;

    public String getId() {
        return m_id;
    }
    public void setId(String id) {
        m_id = id;
    }
    public String getConfOwner() {
        return m_confOwner;
    }
    public void setConfOwner(String confOwner) {
        m_confOwner = confOwner;
    }
    public String getConfName() {
        return m_confName;
    }
    public void setConfName(String confName) {
        m_confName = confName;
    }
    public String getConfDescription() {
        return m_confDescription;
    }
    public void setConfDescription(String confDescription) {
        m_confDescription = confDescription;
    }
    public boolean isModerated() {
        return m_moderated;
    }
    public void setModerated(boolean moderated) {
        m_moderated = moderated;
    }
    public boolean isPublic() {
        return m_public;
    }
    public void setPublic(boolean public1) {
        m_public = public1;
    }
    public boolean isMembersOnly() {
        return m_membersOnly;
    }
    public void setMembersOnly(boolean membersOnly) {
        m_membersOnly = membersOnly;
    }
    public boolean isAutoRecord() {
        return m_autoRecord;
    }
    public void setAutoRecord(boolean autoRecord) {
        m_autoRecord = autoRecord;
    }
    public String getExtension() {
        return m_extension;
    }
    public void setExtension(String extension) {
        m_extension = extension;
    }
    public String getPin() {
        return m_pin;
    }
    public void setPin(String pin) {
        m_pin = pin;
    }
    public String getUri() {
        return m_uri;
    }
    public void setUri(String uri) {
        m_uri = uri;
    }
}
