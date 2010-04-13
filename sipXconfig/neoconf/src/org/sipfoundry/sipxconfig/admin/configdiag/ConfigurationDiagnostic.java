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
import java.util.Date;
import java.util.concurrent.Callable;

import org.apache.commons.digester.Digester;
import org.xml.sax.SAXException;

public class ConfigurationDiagnostic implements Callable<Integer> {

    private String m_name;
    private String m_label;
    private String m_description;
    private String m_longDescription;
    private String m_detailedHelp;

    private ConfigurationDiagnosticResult m_result = ConfigurationDiagnosticResult.INITIAL_RESULT;
    private ExternalCommand m_command;
    private ConfigurationDiagnosticResultParser m_resultParser;
    private Date m_startTime;
    private Date m_endTime;
    private String m_stdout;

    private static class TestDescriptorDigester extends Digester {
        private static final String TEST_PATH = "test";
        private static final String RESULTS_PATH = "test/results";
        private static final String COMMAND_PATH = "test/command";
        private static final String RESULT_PATH = RESULTS_PATH + "/result";
        private static final String DESCRIPTION_PATH = "/description";

        public TestDescriptorDigester(ConfigurationDiagnostic cd) {
            push(cd);
            addSetProperties(TEST_PATH);
            addBeanPropertySetter(TEST_PATH + "/label");
            addBeanPropertySetter(TEST_PATH + DESCRIPTION_PATH);
            addBeanPropertySetter(TEST_PATH + "/longDescription");
            addBeanPropertySetter(TEST_PATH + "/detailedHelp");
            addObjectCreate(COMMAND_PATH, ExternalCommand.class);
            addSetNext(COMMAND_PATH, "setCommand");
            addBeanPropertySetter(COMMAND_PATH + "/exec", "command");
            addCallMethod(COMMAND_PATH + "/arg", "addArgument", 0);
            addObjectCreate(RESULTS_PATH, ConfigurationDiagnosticResultParser.class);
            addSetNext(RESULTS_PATH, "setResultParser");
            addObjectCreate(RESULT_PATH, ConfigurationDiagnosticResult.class);
            addSetNext(RESULT_PATH, "addResult");
            addSetProperties(RESULT_PATH, "exit", "exitStatus");
            addBeanPropertySetter(RESULT_PATH + "/status", "statusAsString");
            addBeanPropertySetter(RESULT_PATH + "/msg", "message");
            addBeanPropertySetter(RESULT_PATH + DESCRIPTION_PATH);
        }
    }

    public void loadFromXml(InputStream xmlStream, ExternalCommandContext ecc) throws IOException, SAXException {
        Digester digester = new TestDescriptorDigester(this);
        digester.parse(xmlStream);
        // set the context here... would be cleaner to set it in the digester code above
        m_command.setContext(ecc);
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

    public String getLongDescription() {
        return m_longDescription;
    }

    public void setLongDescription(String longDescription) {
        m_longDescription = longDescription;
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

    public String getFullDescription() {
        return m_longDescription != null ? m_longDescription : m_description;
    }

    public String getStdout() {
        return m_stdout;
    }

    public String getDetailedHelp() {
        return m_detailedHelp;
    }

    public void setDetailedHelp(String detailedHelp) {
        m_detailedHelp = detailedHelp;
    }

    public Integer call() throws Exception {
        m_startTime = new Date();
        m_endTime = null;
        m_result = ConfigurationDiagnosticResult.INPROGRESS_RESULT;
        int result = m_command.execute();
        m_result = m_resultParser.parseResult(result);
        m_stdout = m_command.getStdout();
        m_endTime = new Date();
        return result;
    }
}
