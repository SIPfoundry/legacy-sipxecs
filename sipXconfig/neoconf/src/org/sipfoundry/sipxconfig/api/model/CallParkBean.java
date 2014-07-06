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

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;

@XmlRootElement(name = "Orbit")
@XmlType(propOrder = {
        "id", "name", "enabled", "extension", "description", "music", "server"
        })
@JsonPropertyOrder({
    "id", "name", "enabled", "extension", "description", "music", "server"
    })
public class CallParkBean {
    private int m_id;
    private String m_name;
    private boolean m_enabled;
    private String m_extension;
    private String m_description;
    private String m_music;
    private String m_server;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public String getName() {
        return m_name;
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

    public String getServer() {
        return m_server;
    }

    public void setServer(String server) {
        m_server = server;
    }

    public static CallParkBean convertOrbit(ParkOrbit orbit) {
        if (orbit == null) {
            return null;
        }
        CallParkBean bean = new CallParkBean();
        bean.setId(orbit.getId());
        bean.setName(orbit.getName());
        bean.setEnabled(orbit.isEnabled());
        bean.setExtension(orbit.getExtension());
        bean.setDescription(orbit.getDescription());
        bean.setMusic(orbit.getMusic());
        bean.setServer(orbit.getLocation().getFqdn());
        return bean;
    }

    public static void populateOrbit(CallParkBean bean, ParkOrbit orbit, Location location) {
        orbit.setName(bean.getName());
        orbit.setEnabled(bean.isEnabled());
        orbit.setExtension(bean.getExtension());
        orbit.setDescription(bean.getDescription());
        orbit.setMusic(bean.getMusic());
        orbit.setLocation(location);
    }
}
