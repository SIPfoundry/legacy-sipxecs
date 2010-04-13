/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * Common base class for all configuration files generators that use Velocity templating engine
 * TemplateConfigurationFile
 */
public abstract class TemplateConfigurationFile extends AbstractConfigurationFile {
    private VelocityEngine m_velocityEngine;
    private String m_templateLocation;

    public TemplateConfigurationFile() {
        super();
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public String getTemplate() {
        return m_templateLocation;
    }

    public void setTemplate(String template) {
        m_templateLocation = template;
    }

    public final void write(Writer output, Location location) throws IOException {
        try {
            VelocityContext context = setupContext(location);
            m_velocityEngine.mergeTemplate(getTemplate(), context, output);
            output.flush();
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Overwrite to set-up Velocity context
     */
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = new VelocityContext();
        context.put("dollar", "$");
        if (location != null) {
            context.put("location", location);
        }
        return context;
    }
}
