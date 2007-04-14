/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    public static final String BEAN_NAME = "dialPlanConfigGenerator";

    private MappingRules m_mappingRules;
    private AuthRules m_authRules;
    private FallbackRules m_fallbackRules;
    private ForwardingRules m_forwardingRules;
    private DialingRuleProvider m_dialingRuleProvider;
    private List m_attendantScheduleFiles = new ArrayList();
    private Map m_files = new HashMap();


    public ConfigGenerator() {
        // this is usually overwritten by spring configuration file
        setMappingRules(new MappingRules());
        setAuthRules(new AuthRules());
        setFallbackRules(new FallbackRules());
        setForwardingRules(new ForwardingRules());
    }
    
    public ForwardingRules getForwardingRules() {
        return m_forwardingRules;
    }

    public void setForwardingRules(ForwardingRules forwardingRules) {
        m_forwardingRules = forwardingRules;
        m_files.put(m_forwardingRules.getType(), m_forwardingRules);
    }

    public void setAuthRules(AuthRules authRules) {
        m_authRules = authRules;
        m_files.put(ConfigFileType.AUTH_RULES, m_authRules);
    }

    public void setMappingRules(MappingRules mappingRules) {
        m_mappingRules = mappingRules;
        m_files.put(ConfigFileType.MAPPING_RULES, mappingRules);
    }

    public void setFallbackRules(FallbackRules fallbackRules) {
        m_fallbackRules = fallbackRules;
        m_files.put(ConfigFileType.FALLBACK_RULES, m_fallbackRules);
    }

    public void setDialingRuleProvider(DialingRuleProvider dialingRuleProvider) {
        m_dialingRuleProvider = dialingRuleProvider;
    }
    
    private void generate(EmergencyRouting er) {
        if (er == null) {
            return;
        }
        List rules = er.asDialingRulesList();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_authRules.generate(rule);
        }
    }

    public void generate(DialPlanContext plan, EmergencyRouting er) {
        generateXmlFromDialingRules(plan, er);

        List attendantRules = plan.getAttendantRules();
        for (Iterator i = attendantRules.iterator(); i.hasNext();) {
            AttendantRule ar = (AttendantRule) i.next();
            if (ar.isEnabled()) {
                AttendantScheduleFile file = new AttendantScheduleFile();
                file.generate(ar);
                m_attendantScheduleFiles.add(file);
            }
        }
    }

    /** Given the DialPlanContext and the EmergencyRouting, generate XML */
    private void generateXmlFromDialingRules(DialPlanContext plan, EmergencyRouting er) {
        // Get rules from dialing rule providers and the dial plan
        List rules = new ArrayList();
        if (m_dialingRuleProvider != null) {
            rules.addAll(m_dialingRuleProvider.getDialingRules());            
        }
        rules.addAll(plan.getGenerationRules());
        
        m_mappingRules.begin();
        m_authRules.begin();
        m_fallbackRules.begin();
        m_forwardingRules.begin();

        generate(er);

        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_mappingRules.generate(rule);
            m_authRules.generate(rule);
            m_fallbackRules.generate(rule);
            m_forwardingRules.generate(rule);
        }

        m_mappingRules.end();
        m_authRules.end();
        m_fallbackRules.end();    
        m_forwardingRules.end();
    }
    
    /**
     * Retrieves configuration file content as stream.
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     * @param type type of the configuration file
     */
    public String getFileContent(ConfigFileType type) {
        XmlFile file = (XmlFile) m_files.get(type);
        return file.getFileContent();
    }

    public void activate(SipxReplicationContext sipxReplicationContext, String scriptsDirectory) {
        for (Iterator i = m_files.values().iterator(); i.hasNext();) {
            XmlFile file = (XmlFile) i.next();
            sipxReplicationContext.replicate(file);
        }
        activateAttendantRules(scriptsDirectory);
    }

    private void activateAttendantRules(String scriptsDirectory) {
        try {
            for (Iterator i = m_attendantScheduleFiles.iterator(); i.hasNext();) {
                XmlFile file = (XmlFile) i.next();
                file.writeToFile(new File(scriptsDirectory), file.getFileBaseName());
            }
        } catch (IOException e) {
            throw new UserException("Error when generating auto attendant schedule.", e);
        }
    }
}
