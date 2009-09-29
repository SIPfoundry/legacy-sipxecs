/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Map;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;

/**
 * Profile generator that is using Velocity template engine to generate the profile.
 */
public class VelocityProfileGenerator extends AbstractProfileGenerator {
    private VelocityEngine m_velocityEngine;

    /**
     * Char encoding for the templates velocity templates directory: in most cases they are
     * limited to ASCII.
     */
    private String m_templateEncoding = "US-ASCII";

    private String m_profileEncoding = "UTF-8";

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    /**
     * Most phones/devices are not very clear on what type of encoding they use in the profiles.
     * We set it to UTF-8 by default. It is required by standard to be supported on all Java
     * platforms and it *should* work with most phones.
     *
     * The only case where profile encoding does matter is when profile contain non-ASCII
     * characters, which happens most often in names, phonesbooks, speeddials.
     *
     */
    public void setProfileEncoding(String profileEncoding) {
        m_profileEncoding = profileEncoding;
    }

    @Override
    protected void generateProfile(ProfileContext context, OutputStream out) {
        VelocityContext velocityContext = new VelocityContext();
        Set<Map.Entry<String, Object>> entries = context.getContext().entrySet();
        for (Map.Entry<String, Object> entry : entries) {
            velocityContext.put(entry.getKey(), entry.getValue());
        }
        velocityContext.put("dollar", "$");

        String template = context.getProfileTemplate();
        try {
            Writer output = new OutputStreamWriter(out, m_profileEncoding);
            m_velocityEngine.mergeTemplate(template, m_templateEncoding, velocityContext, output);
            output.flush();
        } catch (Exception e) {
            if (e instanceof RuntimeException) {
                throw (RuntimeException) e;
            }
            throw new RuntimeException("Error using velocity template " + template, e);
        }
    }
}
