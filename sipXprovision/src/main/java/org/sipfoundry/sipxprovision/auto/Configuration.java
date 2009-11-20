/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision.auto;

import java.io.FileInputStream;
import java.io.PrintWriter;
import java.lang.reflect.Field;
import java.util.Properties;

import org.apache.log4j.PropertyConfigurator;
import org.apache.commons.codec.binary.Base64;

import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.util.Hostname;

/**
 * The configuration object for the sipXprovision service.
 *
 * @author Paul Mossman
 */
public class Configuration {

    private static final String DEFAULT_STRING = "unknown";

    protected static final String SUPERADMIN_USERNAME = "superadmin";

    private String m_LogLevel = DEFAULT_STRING;

    private String m_SipDomainName = DEFAULT_STRING;

    private String m_SharedSecret = DEFAULT_STRING;

    private String m_Logfile = DEFAULT_STRING;

    private String m_TftpRoot = DEFAULT_STRING;

    private boolean m_Debug = false;

    private int m_ServletPort = -1;

    private String m_ProvisionSipUsername = DEFAULT_STRING;

    private String m_ProvisionSipPassword = DEFAULT_STRING;

    private String m_ConfigurationUri = DEFAULT_STRING;

    private static final String DEBUG_BY_DEFAULT = "false";

    public Configuration() {

        // Passed on the command-line.
        String var_dir = System.getProperty("var.dir");
        if (null != var_dir) {

            m_TftpRoot = var_dir + "/configserver/phone/profile/tftproot";
        }
        
        // domain-config
        Properties domain_config = loadProperties("/domain-config");
        if (null != domain_config) {

            m_SipDomainName = domain_config.getProperty("SIP_DOMAIN_NAME", DEFAULT_STRING);
            m_SharedSecret = domain_config.getProperty("SHARED_SECRET", DEFAULT_STRING);
        }

        // sipxprovision-config
        Properties prov_config = loadProperties("/sipxprovision-config");
        if (null != prov_config) {

            m_LogLevel = prov_config.getProperty("log.level", DEFAULT_STRING);
            m_Logfile = prov_config.getProperty("log.file", DEFAULT_STRING);
            m_Debug = (new Boolean(prov_config.getProperty("provision.debug", DEBUG_BY_DEFAULT))).booleanValue();
            m_ServletPort = (new Integer(prov_config.getProperty("provision.servlet.port", "-1"))).intValue();
            m_ProvisionSipUsername = prov_config.getProperty("provision.username", DEFAULT_STRING);
            m_ProvisionSipPassword = prov_config.getProperty("provision.password", DEFAULT_STRING);
            m_ConfigurationUri  = prov_config.getProperty("provision.configUrl", DEFAULT_STRING);
        }
    }

    protected static Properties loadProperties(String path_under_conf_dir) {

        Properties result = null;

        String file_path = System.getProperty("conf.dir") + path_under_conf_dir;
        try {
            FileInputStream fis = new FileInputStream(file_path);
            result = new Properties();
            result.load(fis);

        }
        catch (Exception e) {
            System.err.println("Failed to read '" + file_path + "':");
            e.printStackTrace(System.err);
        }

        return result;
    }

    public String getLog4jLevel() {
        return SipFoundryLayout.mapSipFoundry2log4j(m_LogLevel).toString();
    }

    public boolean isDebugOn() {
        return m_Debug;
    }

    public void configureLog4j() {

       Properties props = new Properties();
       props.setProperty("log4j.rootLogger", "warn, file");
       props.setProperty("log4j.logger.Queue", getLog4jLevel());
       props.setProperty("log4j.logger.Servlet", getLog4jLevel());
       props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender");
       props.setProperty("log4j.appender.file.File", getLogfile());
       props.setProperty("log4j.appender.file.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
       props.setProperty("log4j.appender.file.layout.facility", "");
       PropertyConfigurator.configure(props);
    }

    public String getTftpPath() {
        return m_TftpRoot;
    }

    public int getServletPort() {
        return m_ServletPort;
    }

    public String getServletUriPath() {
        return "";
    }

    public String getSipDomainName() {
        return m_SipDomainName;
    }

    public String getLogfile() {
        return m_Logfile;
    }

    static final String HOSTNAME = Hostname.get();

    public String getHostname() {
        return HOSTNAME;
    }

    public String getConfigurationUri() {
        return m_ConfigurationUri + "/sipxconfig";

    }

    public String getConfigurationRestPhonesUri() {
        return getConfigurationUri() + "/rest/phone";
    }

    public String getConfigurationRestCredentials() {
        return SUPERADMIN_USERNAME + ":" + m_SharedSecret;
    }

    public String getBase64ConfigurationRestCredentials() {
        return new String(Base64.encodeBase64((getConfigurationRestCredentials()).getBytes()));
    }

    public String getProvisionSipUsername() {
        return m_ProvisionSipUsername;
    }

    public String getProvisionSipPassword() {
        return m_ProvisionSipPassword;
    }

    public long getQueueDebugPostProvisionSleepTime() {
        return isDebugOn() ? 50 : 0;
    }

    public void dumpConfiguration(PrintWriter out) {

        out.println("Configuration:");
        Field[] fields = getClass().getDeclaredFields();

        for (Field field : fields) {
            try {
                Object value = field.get(this);
                if (field.getName() == "m_SharedSecret" ||
                    field.getName() == "m_ProvisionSipPassword") {
                    value = new String("<<<secret>>>");
                }
                out.println(" - " + field.getName() + " = '" + value + "'");

            }
            catch (IllegalArgumentException e) {
                e.printStackTrace(out);
            }
            catch (IllegalAccessException e) {
                e.printStackTrace(out);
            }
        }
        out.flush();
    }

    public static void main(String[] args) {

        Configuration config = new Configuration();

        config.dumpConfiguration(new PrintWriter(System.out, true));
    }

}
