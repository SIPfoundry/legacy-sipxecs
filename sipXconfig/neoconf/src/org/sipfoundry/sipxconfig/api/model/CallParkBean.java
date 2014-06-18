/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.model;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.apache.commons.beanutils.BeanUtils;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;

@XmlRootElement(name = "Orbit")
@XmlType(propOrder = {
        "id", "name", "enabled", "extension", "description", "music"
        })
public class CallParkBean {
    private int m_id;
    private String m_name;
    private boolean m_enabled;
    private String m_extension;
    private String m_description;
    private String m_music;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getMusic() {
        return m_music;
    }

    public void setMusic(String music) {
        m_music = music;
    }

    public static CallParkBean convertOrbit(ParkOrbit orbit) {
        if (orbit == null) {
            return null;
        }
        try {
            CallParkBean bean = new CallParkBean();
            BeanUtils.copyProperties(bean, orbit);
            return bean;
        } catch (Exception ex) {
            return null;
        }
    }

    public static boolean populateOrbit(CallParkBean bean, ParkOrbit orbit) {
        try {
            if (orbit != null && bean != null) {
                BeanUtils.copyProperties(orbit, bean);
                return true;
            }
            return false;
        } catch (Exception ex) {
            return false;
        }
    }
}
