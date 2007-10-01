/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;
import java.util.Date;

import org.apache.commons.digester.Digester;
import org.xml.sax.SAXException;

public class ConfigurationDiagnostic implements ExecutionTask, Serializable {

    private static final String TEST_PATH = "test";
    private static final String RESULTS_PATH = "test/results";
    private static final String COMMAND_PATH = "test/command";
    private static final String RESULT_PATH = RESULTS_PATH + "/result";
    
    private String m_name;
    private String m_label;
    private String m_description;
    private ConfigurationDiagnosticResult m_result;
    private ExternalCommand m_command;
    private ConfigurationDiagnosticResultParser m_resultParser;
    private Date m_startTime;
    private Date m_endTime;

    public ConfigurationDiagnostic() {
        m_result = ConfigurationDiagnosticResult.UNKNOWN_RESULT;
    }

    public void loadFromXml(InputStream xmlStream) throws IOException, SAXException {
        Digester digester = new Digester();

        digester.push(this);
        digester.addSetProperties(TEST_PATH);
        digester.addBeanPropertySetter(TEST_PATH + "/label");
        digester.addBeanPropertySetter(TEST_PATH + "/description");
        digester.addObjectCreate(COMMAND_PATH, ExternalCommand.class);
        digester.addSetNext(COMMAND_PATH, "setCommand");
        digester.addBeanPropertySetter(COMMAND_PATH + "/exec", "command");
        digester.addCallMethod(COMMAND_PATH + "/arg", "addArgument", 0);
        digester.addObjectCreate(RESULTS_PATH, ConfigurationDiagnosticResultParser.class);
        digester.addSetNext(RESULTS_PATH, "setResultParser");
        digester.addObjectCreate(RESULT_PATH, ConfigurationDiagnosticResult.class);
        digester.addSetNext(RESULT_PATH, "addResult");
        digester.addSetProperties(RESULT_PATH, "exit", "exitStatus");
        digester.addBeanPropertySetter(RESULT_PATH + "/status", "statusAsString");
        digester.addBeanPropertySetter(RESULT_PATH + "/msg", "message");
        digester.parse(xmlStream);
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public ConfigurationDiagnosticResult getResult() {
        return m_result;
    }

    public ExternalCommand getCommand() {
        return m_command;
    }

    public void setCommand(ExternalCommand command) {
        m_command = command;
    }

    public void setResultParser(ConfigurationDiagnosticResultParser parser) {
        m_resultParser = parser;
    }

    public ConfigurationDiagnosticResultParser getResultParser() {
        return m_resultParser;
    }

    public Date getStartTime() {
        return m_startTime;
    }

    public Date getEndTime() {
        return m_endTime;
    }

    public void execute() {
        m_startTime = new Date();
        m_endTime = null;
        m_result = ConfigurationDiagnosticResult.INPROGRESS_RESULT;
        m_result = m_resultParser.parseResult(m_command.execute());
        m_endTime = new Date();
    }
}
