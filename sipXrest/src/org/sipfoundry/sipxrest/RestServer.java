/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.io.File;
import java.util.Timer;

import javax.sip.address.AddressFactory;
import javax.sip.header.HeaderFactory;
import javax.sip.message.MessageFactory;

import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.sipfoundry.commons.jetty.SocketFactory;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.restconfig.RestServerConfig;
import org.sipfoundry.commons.restconfig.RestServerConfigFileParser;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class RestServer {

    private static Logger logger = Logger.getLogger(RestServer.class);

    static final String PACKAGE = "org.sipfoundry.sipxrest";

    private static String configFileName = "/etc/sipxpbx/sipxrest-config.xml";

    private static Appender appender;

    private static RestServerConfig restServerConfig;

    private static HttpServer webServer;

    public static final Timer timer = new Timer();

    private static RestServiceFinder restServiceFinder;

    private static AccountManagerImpl accountManager;

    private static SipStackBean sipStackBean;


    public static RestServerConfig getRestServerConfig() {
        return restServerConfig;
    }


    /**
     * @param appender the appender to set
     */
    public static void setAppender(Appender appender) {
        RestServer.appender = appender;
    }

    /**
     * @return the appender
     */
    public static Appender getAppender() {
        return appender;
    }


    private static void initWebServer() throws Exception {
        webServer = new HttpServer();
        Logger.getLogger("org.mortbay").setLevel(Level.OFF);


        // create a listener for the public port.
        SocketListener publicSocketListener = SocketFactory.createSocketListener(restServerConfig.getPublicHttpPort());
        SocketListener socketListener = SocketFactory.createSocketListener(restServerConfig.getHttpPort());
        webServer.addListener(publicSocketListener);
        webServer.addListener(socketListener);

        HttpContext httpContext = new HttpContext();
        httpContext.setContextPath("/");
        httpContext.setInitParameter("org.restlet.application",
                RestServerApplication.class.getName());
        ServletHandler servletHandler = new ServletHandler();
        Class<?> servletClass = com.noelios.restlet.ext.servlet.ServerServlet.class;
        servletHandler.addServlet("rest", "/*", servletClass.getName());

        httpContext.addHandler(servletHandler);

        webServer.addContext(httpContext);
        webServer.start();
    }

    /**
     * @return the messageFactory
     */
    public static MessageFactory getMessageFactory() {
        return sipStackBean.getMessageFactory();
    }


    /**
     * @return the addressFactory
     */
    public static AddressFactory getAddressFactory() {
        return sipStackBean.getAddressFactory();
    }


    /**
     * @return the headerFactory
     */
    public static HeaderFactory getHeaderFactory() {
        return sipStackBean.getHeaderFactory();
    }



    public static RestServiceFinder getServiceFinder() {
        return restServiceFinder;
    }

    public static AccountManagerImpl getAccountManager() {
        return accountManager;
     }
    /**
     * @param args
     */
    public static void main(String[] args) throws Exception {

        String configDir = System.getProperties().getProperty("conf.dir",  "/etc/sipxpbx");
        configFileName = configDir + "/sipxrest-config.xml";

        if (!new File(configFileName).exists()) {
            System.err.println("Cannot find the config file");
            System.exit(-1);
        }

        restServerConfig = new RestServerConfigFileParser().parse("file://"
                + configFileName);
        Logger.getLogger(PACKAGE).setLevel(
                SipFoundryLayout.mapSipFoundry2log4j(restServerConfig.getLogLevel()));
        setAppender(new SipFoundryAppender(new SipFoundryLayout(),
                RestServer.getRestServerConfig().getLogDirectory()
                +"/sipxrest.log"));
        Logger.getLogger(PACKAGE).addAppender(getAppender());

        accountManager = new AccountManagerImpl();
        sipStackBean = new SipStackBean();

        restServiceFinder = new RestServiceFinder();

        restServiceFinder.search(System.getProperty("plugin.dir"));

        try {
            UnfortunateLackOfSpringSupportFactory.initialize();
        } catch (Exception e) {
            logger.error(e);
        }

        initWebServer();

        logger.debug("Web server started.");
    }


    /**
     * @return the sipStack
     */
    public static SipStackBean getSipStack() {
        return sipStackBean;
    }

    public static String getRealm() {
        return DomainConfiguration.getSipRealm();
    }
}
