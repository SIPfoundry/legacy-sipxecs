package org.sipfoundry.restServer;

import java.io.File;
import java.util.Timer;

import javax.net.ssl.SSLServerSocket;

import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpListener;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.http.SslListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.mortbay.util.InetAddrPort;
import org.mortbay.util.ThreadedServer;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

public class RestServer {
    
    private static Logger logger = Logger.getLogger(RestServer.class);
    
    static final String PACKAGE = "org.sipfoundry.restServer";
    
    private static String configFileName = "/etc/sipxpbx/sipxrest.xml";

    private static String accountFileName = "/etc/sipxpbx/validusers.xml";

    private static AccountManagerImpl accountManager;

    private static Appender appender;
    
    private static RestServerConfig restServerConfig;

    private static HttpServer webServer;
    
    public static boolean isSecure = true;
    
    public static final Timer timer = new Timer();
    
  
    public static RestServerConfig getRestServerConfig() {
        return restServerConfig;
    }

    public static AccountManagerImpl getAccountManager() {
        return accountManager;
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
        InetAddrPort inetAddrPort = new InetAddrPort(restServerConfig.getIpAddress(),
                restServerConfig.getHttpPort());
        InetAddrPort externalInetAddrPort = new InetAddrPort(restServerConfig.getIpAddress(),
                restServerConfig.getPublicHttpPort());
        Logger.getLogger("org.mortbay").setLevel(Level.OFF);
        
     
        
        if (isSecure) {
                SslListener sslListener = new SslListener(inetAddrPort);     
                String keystore = System.getProperties().getProperty(
                        "javax.net.ssl.keyStore");
                logger.debug("keystore = " + keystore);
                sslListener.setKeystore(keystore);
                String algorithm = System.getProperties().getProperty(
                        "jetty.x509.algorithm");
                logger.debug("algorithm = " + algorithm);
                sslListener.setAlgorithm(algorithm);
                String password = System.getProperties().getProperty(
                        "jetty.ssl.password");
                sslListener.setPassword(password);
                String keypassword = System.getProperties().getProperty(
                        "jetty.ssl.keypassword");
                sslListener.setKeyPassword(keypassword);
                sslListener.setMaxThreads(32);
                sslListener.setMinThreads(4);
                sslListener.setLingerTimeSecs(30000);
                
                
                ((ThreadedServer) sslListener).open();

                String[] cypherSuites = ((SSLServerSocket) sslListener
                        .getServerSocket()).getSupportedCipherSuites();

              
                ((SSLServerSocket) sslListener.getServerSocket())
                        .setEnabledCipherSuites(cypherSuites);

                String[] protocols = ((SSLServerSocket) sslListener
                        .getServerSocket()).getSupportedProtocols();

              
                ((SSLServerSocket) sslListener.getServerSocket())
                        .setEnabledProtocols(protocols);
             
                webServer.addListener(sslListener);
                sslListener.start(); 
        } 


        // create a listener for the unsecure port.
        logger.debug("External Inet Port = " + externalInetAddrPort.toString());
        SocketListener socketListener = new SocketListener(externalInetAddrPort);
        socketListener.setMaxThreads(32);
        socketListener.setMinThreads(4);
        socketListener.setLingerTimeSecs(30000);
        webServer.addListener(socketListener);
        socketListener.start();

        HttpContext httpContext = new HttpContext();

        httpContext.setContextPath("/");
        httpContext.setInitParameter("org.restlet.application", 
                RestServerApplication.class.getName());
        ServletHandler servletHandler = new ServletHandler();
        Class<?> servletClass = com.noelios.restlet.ext.servlet.ServerServlet.class;
        servletHandler.addServlet("restServer", "/*",
                servletClass.getName());
        
             
        httpContext.addHandler(servletHandler);
        
       
        webServer.addContext(httpContext);
        
        webServer.start();

    }

    /**
     * @param args
     */
    public static void main(String[] args) throws Exception {

        accountFileName = System.getProperties().getProperty("conf.dir", "/etc/sipxpbx")
                + "/validusers.xml";

        configFileName = System.getProperties().getProperty("conf.dir",  "/etc/sipxpbx")
                + "/sipxrest.xml";

        if (!new File(accountFileName).exists()) {
            System.err.println("Cannot find the accounts file");
            System.exit(-1);
        }

        if (!new File(configFileName).exists()) {
            System.err.println("Cannot find the accounts file");
            System.exit(-1);
        }

        restServerConfig = new ConfigFileParser().createRestServerConfig("file://"
                + configFileName);
        accountManager = new AccountManagerImpl();
        Logger.getLogger(PACKAGE).setLevel(Level.toLevel(restServerConfig.getLogLevel()));
        setAppender(new SipFoundryAppender(new SipFoundryLayout(),
                RestServer.getRestServerConfig().getLogDirectory()
                +"/sipxrestServer.log"));
        Logger.getLogger(PACKAGE).addAppender(getAppender());
      
        initWebServer();
       
        logger.debug("Web server started.");

    }

}
