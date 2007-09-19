/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.io.File;
import java.io.PrintWriter;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.apache.velocity.exception.ParseErrorException;
import org.apache.velocity.exception.ResourceNotFoundException;

public class DomainConfigGenerator {

    private static final String OUTPUT_FILENAME = "domain-config";

    private VelocityEngine m_velocityEngine;
    private String m_templateLocation = "domain/domain-config.vm";
    private String m_outputDir;

    public void generate(Domain domain) {
        File outputFile = new File(m_outputDir, OUTPUT_FILENAME);
        Writer writer = null;
        try {
            writer = new PrintWriter(outputFile);
            generate(domain, writer);
        } catch (Exception ioe) {
            throw new RuntimeException(ioe);
        } finally {
            IOUtils.closeQuietly(writer);
        }
    }

    public void generate(Domain domain, Writer output) {
        try {
            VelocityContext context = new VelocityContext();
            context.put("domain", domain);
            getVelocityEngine().mergeTemplate(getTemplate(), context, output);
            output.flush();
        } catch (ResourceNotFoundException e) {
            throw new RuntimeException(e);
        } catch (ParseErrorException e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public String getOutputDir() {
        return m_outputDir;
    }

    public void setOutputDir(String outputDir) {
        m_outputDir = outputDir;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
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
}
