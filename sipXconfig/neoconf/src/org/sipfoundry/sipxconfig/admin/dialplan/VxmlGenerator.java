/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.apache.velocity.exception.ParseErrorException;
import org.apache.velocity.exception.ResourceNotFoundException;

import org.sipfoundry.sipxconfig.domain.DomainManager;

public class VxmlGenerator {
    
    private String m_promptsDirectory;
    
    private String m_scriptsDirectory;

    private VelocityEngine m_velocityEngine;

    private DomainManager m_domainManager;
    
    private String m_template = "sipxvxml/autoattendant.vm";

    public String getTemplate() {
        return m_template;
    }    

    public void setTemplate(String template) {
        m_template = template;
    }    

    public String getScriptsDirectory() {
        return m_scriptsDirectory;
    }

    public void setScriptsDirectory(String scriptsDirectory) {
        m_scriptsDirectory = scriptsDirectory;
    }

    public String getPromptsDirectory() {
        return m_promptsDirectory;
    }   

    public void setPromptsDirectory(String promptsDirectory) {
        m_promptsDirectory = promptsDirectory;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }
    
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
    
    public void generate(AutoAttendant aa) {
        FileWriter out = null;
        try {
            File scriptsDir = new File(getScriptsDirectory());
            scriptsDir.mkdirs();
            File scriptsFile = new File(scriptsDir, aa.getScriptFileName());
            out = new FileWriter(scriptsFile);
            generate(aa, out);            
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(out);
        }        
    }
    
    public void generate(AutoAttendant aa, Writer out) {
        try {
            VelocityContext context = new VelocityContext();
            context.put("attendant", aa);
            context.put("vxml", this);            
            context.put("domainName", m_domainManager.getDomain().getName());
            getVelocityEngine().mergeTemplate(getTemplate(), context, out);
        } catch (ResourceNotFoundException e) {
            throw new RuntimeException(e);
        } catch (ParseErrorException e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}
