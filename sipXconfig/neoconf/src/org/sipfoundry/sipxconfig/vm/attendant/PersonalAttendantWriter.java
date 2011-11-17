/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import java.io.StringWriter;
import java.util.Map;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.vm.LocalMailbox;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant.AttendantProfileContext;

public class PersonalAttendantWriter {
    private ProfileGenerator m_generator;
    private VelocityEngine m_velocityEngine;
    private CoreContext m_coreContext;

    public void write(LocalMailbox mailbox, PersonalAttendant pa) {
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(mailbox.getUserDirectory().getPath());
        String domain = m_coreContext.getDomainName();
        pa.generatePropertiesProfile(location, domain, m_generator);
    }

    public void write(PersonalAttendant pa, StringWriter writer) {
        String domain = m_coreContext.getDomainName();
        AttendantProfileContext context = new AttendantProfileContext(pa, domain,
                "sipxivr/PersonalAttendant.properties.vm");
        VelocityContext velocityContext = new VelocityContext();
        Set<Map.Entry<String, Object>> entries = context.getContext().entrySet();
        for (Map.Entry<String, Object> entry : entries) {
            velocityContext.put(entry.getKey(), entry.getValue());
        }
        velocityContext.put("dollar", "$");

        String template = context.getProfileTemplate();
        try {
            m_velocityEngine.mergeTemplate(template, velocityContext, writer);
        } catch (Exception e) {
            if (e instanceof RuntimeException) {
                throw (RuntimeException) e;
            }
            throw new RuntimeException("Error using velocity template " + template, e);
        }
    }

    public void setGenerator(ProfileGenerator generator) {
        m_generator = generator;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
