/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;
import java.io.FileWriter;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.HotellingManager;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

public class HotellingManagerImpl implements HotellingManager {
    private static final String ERROR = "Error using velocity template %s";
    private VelocityEngine m_velocityEngine;
    private String m_profileTemplate;
    private String m_directoryTemplate;
    private String m_userCfg;
    private String m_userDirectoryXml;
    private CoreContext m_coreContext;
    private PhonebookManager m_phonebookManager;
    private SpeedDialManager m_speedDialManager;

    /**
     * Char encoding for the templates velocity templates directory: in most cases they are
     * limited to ASCII.
     */
    private String m_templateEncoding = "US-ASCII";

    @Override
    public void generate(User user) {
        if ("1".equals(user.getSettings().getSetting("hotelling/enable").getValue())) {

            VelocityContext velocityContext = new VelocityContext();

            velocityContext.put("pintoken", user.getPintoken());
            velocityContext.put("displayName",
                    StringUtils.defaultIfEmpty(String.format("%s %s", user.getFirstName(), user.getLastName()), ""));
            velocityContext.put("uid", user.getUserName());
            velocityContext.put("label", user.getUserName());
            velocityContext.put("dollar", "$");

            DirectoryConfiguration dc = new DirectoryConfiguration(m_phonebookManager.getEntries(
                    m_phonebookManager.getAllPhonebooksByUser(user), user), m_speedDialManager.getSpeedDialForUser(
                            user, false));
            VelocityContext velocityContextDirectory = new VelocityContext();
            velocityContextDirectory.put("cfg", dc);

            try {
                Writer output = new FileWriter(new File(String.format(m_userCfg, user.getUserName())));
                m_velocityEngine.mergeTemplate(m_profileTemplate, m_templateEncoding, velocityContext, output);
                output.flush();
            } catch (Exception e) {
                if (e instanceof RuntimeException) {
                    throw (RuntimeException) e;
                }
                throw new RuntimeException(String.format(ERROR, m_profileTemplate), e);
            }
            try {
                Writer outputDirectory = new FileWriter(new File(String.format(m_userDirectoryXml,
                        user.getUserName())));
                m_velocityEngine.mergeTemplate(m_directoryTemplate, m_templateEncoding, velocityContextDirectory,
                        outputDirectory);
                outputDirectory.flush();
            } catch (Exception e) {
                if (e instanceof RuntimeException) {
                    throw (RuntimeException) e;
                }
                throw new RuntimeException(String.format(ERROR, m_directoryTemplate), e);
            }
        } else {
            File userCfg = new File(String.format(m_userCfg, user.getUserName()));
            if (userCfg.exists()) {
                userCfg.delete();
            }
            File directoryXml = new File(String.format(m_userDirectoryXml, user.getUserName()));
            if (directoryXml.exists()) {
                directoryXml.delete();
            }
        }
    }

    public void setProfileTemplate(String profileTemplate) {
        m_profileTemplate = profileTemplate;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    @Override
    public void generate(Group g) {
        if (!g.getResource().equals(User.GROUP_RESOURCE_ID)) {
            return;
        }
        // TODO: use parallel processing here, as in ReplicationManager?
        for (User u : m_coreContext.getGroupMembers(g)) {
            generate(u);
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    public void setUserCfg(String userCgf) {
        m_userCfg = userCgf;
    }

    public void setUserDirectoryXml(String userDirectoryXml) {
        m_userDirectoryXml = userDirectoryXml;
    }

    public void setDirectoryTemplate(String directoryTemplate) {
        m_directoryTemplate = directoryTemplate;
    }

}
