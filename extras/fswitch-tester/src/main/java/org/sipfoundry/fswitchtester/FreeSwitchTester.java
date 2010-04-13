/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.URL;
import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;

import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.address.Hop;

import org.apache.log4j.Level;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServer;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.WebServer;

/**
 * Main tester entry point.
 * 
 * Invoke this class with the following arguments
 * 
 * -DtesterConfig="testerconfig.xml" -DclientId="x"
 * 
 * Where x is a reference to a specific client in the testerconfig.xml directory. The first such
 * client is the controller. It will tell all its peers to start running the test after a specific
 * timeout.
 * 
 * @author mranga
 * 
 */
public class FreeSwitchTester {

    private static String logLevel = Level.DEBUG.toString();

    private static String logFile = "logs/fswitchtest.log";

    private static TesterConfig testerConfig;

    private static HopImpl proxyHop;

    private static int mediaPort;

    protected static Timer timer = new Timer();

    private static HashSet<ConferenceTestClient> clients = new HashSet<ConferenceTestClient>();

    private static String clientId;

    private static WebServer webServer;

    private static String TESTER = "FreeSwitchTester";

    /**
     * @return the logLevel
     */
    public static String getLogLevel() {
        return logLevel;
    }

    public static String getLogFile() {
        return logFile;
    }

    public static int getClientId() {
        return testerConfig.getTesterClientConfig().getClientId();
    }

    public static String getSipxProxyDomain() {
        return testerConfig.getSipxProxyDomain();
    }

    public static Hop getSipxProxyHop() {
        return FreeSwitchTester.proxyHop;
    }

    public static TesterConfig getTesterConfig() {
        return FreeSwitchTester.testerConfig;
    }

    public static String getSipxProxyAddress() {
        return testerConfig.getSipxProxyAddress();
    }

    public static String getTesterHost() {
        return testerConfig.getTesterClientConfig().getTesterHost();
    }

    public static int allocateMediaPort() {
        int retval = mediaPort;
        mediaPort += 2;
        return retval;
    }

    public static int getLastAllocatedMediaPort() throws FreeSwitchTesterException {
        if (mediaPort == testerConfig.getTesterRtpBasePort()) {
            throw new FreeSwitchTesterException("No port allocated");

        }
        return mediaPort - 2;
    }

    public static int getTestDuration() {
        return testerConfig.getTestDuration();
    }

    private static Double sGetCumulativeResult() {
        if ( ConferenceTestClient.resultList.size() == 0 ) {
            return new Double (0.0);
        }
        double result = 0;
        for (Double d : ConferenceTestClient.resultList) {
            result += d;
        }
        return result/ConferenceTestClient.resultList.size();
    }

    private static Double sGetCumulativeJitter() {
        double result = 0;
        if ( ConferenceTestClient.jitterResultList.size() ==  0 ) {
            return new Double(0.0);
        }
        for (Double d : ConferenceTestClient.jitterResultList) {
            result += d;
        }
        return result/ConferenceTestClient.jitterResultList.size();
    }
    private static void sStart() {
        try {
            for (ConferenceTestClient client : clients) {
                client.sendInvite();
                //Thread.sleep(1000);
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
    
    

    /**
     * XML RPC method (must be non-static).
     * 
     * @return -- the cumulative result.
     */
    public Double getCumulativeResult() {
        return sGetCumulativeResult();
    }
    
    public Double getCumulativeJitter() {
        return sGetCumulativeJitter();
    }

    /**
     * XML RPC method (must be non-static).
     * 
     */

    public String start() {
        sStart();
        return "OK";
    }

    /**
     * XML RPC method (must be non-static).
     * 
     */
    public String connect() {
        return "OK";

    }

    /**
     * XML RPC method (must be non-static).
     * 
     * @return -- the cumulative result.
     */
    public String stop() {

        timer.schedule(new TimerTask() {
            public void run() {
                System.exit(0);
            }
        }, 1000);
        return "OK";
    }

    /**
     * @param args
     */
    public static void main(String[] args) throws Exception {
        String testerFileName = null;
        testerFileName = System.getProperties().getProperty("testerConfig");
        clientId = System.getProperties().getProperty("clientId");
        if (clientId == null) {
            System.out.println("Missing clientId");
            System.exit(0);
        }
        if (testerFileName == null) {
            testerFileName = "testerconfig.xml";
        }
        testerConfig = new TesterConfigParser().parse(testerFileName);

        proxyHop = new HopImpl(testerConfig.getSipxProxyAddress(), testerConfig
                .getSipxProxyPort(), "udp");
        testerConfig.setClientId(clientId);

        if (testerConfig.getTesterClientConfig() == null) {
            System.err.println("Check configuration -- config for " + clientId + " not found.");
            System.exit(-1);
        }
        if (testerConfig.getTesterClientConfig() == null) {
            System.err.println("Cannot find configuration for client " + clientId);
        }
        mediaPort = testerConfig.getTesterRtpBasePort();

        SipListenerImpl listener = new SipListenerImpl();

        for (int i = 0; i < testerConfig.getConferenceSize(); i++) {
            String host = testerConfig.getTesterHost();
            int port = testerConfig.getTesterSipBasePort() + 2 * i;
            String transport = "udp";
            System.out.println("index = " + i);
            ListeningPoint listeningPoint = ProtocolObjects.sipStack.createListeningPoint(host,
                    port, transport);
            SipProvider provider = ProtocolObjects.sipStack.createSipProvider(listeningPoint);
            provider.addSipListener(listener);
            ConferenceTestClient conferenceTestClient = new ConferenceTestClient(i, provider,
                    port);
            if (i < testerConfig.getTesterClientConfig().getSenderCount()) {
                conferenceTestClient.setSender(true);
            }

           
            listener.addSlave(provider, conferenceTestClient);
            clients.add(conferenceTestClient);

        }

        // If this is a distributed tester, start a web server.

        if (testerConfig.getTesterClients().size() != 1) {
            webServer = new WebServer(testerConfig.getXmlRpcPort(), InetAddress
                    .getByName(testerConfig.getTesterHost()));

            PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

            handlerMapping.addHandler(TESTER, FreeSwitchTester.class);

            XmlRpcServer server = webServer.getXmlRpcServer();

            XmlRpcServerConfigImpl serverConfig = new XmlRpcServerConfigImpl();
            serverConfig.setKeepAliveEnabled(true);

            server.setConfig(serverConfig);
            server.setHandlerMapping(handlerMapping);
            webServer.start();
        } else {
            sStart();
        }

        int clientCount = 0;
        
        for ( TesterClientConfig clientConfig : testerConfig.getTesterClients() ) {
            clientCount += clientConfig.getUserAgentCount();
        }
        

        if (testerConfig.isFirstClient() && testerConfig.getTesterClients().size() > 1) {
            for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                String serverAddress = clientConfig.getTesterHost();
                int port = clientConfig.getXmlRpcPort();
                XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
                config.setServerURL(new URL("http://" + serverAddress + ":" + port));
                XmlRpcClient client = new XmlRpcClient();
                client.setConfig(config);
                clientConfig.setXmlRpcClient(client);
            }
            boolean success = false;
            while (!success) {
                try {
                    for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                        if (clientConfig != FreeSwitchTester.getTesterConfig()
                                .getTesterClientConfig()) {
                            clientConfig.getXmlRpcClient().execute(TESTER + ".connect",
                                    (Object[]) null);
                        }
                    }
                    success = true;
                } catch (Exception ex) {
                    // ex.printStackTrace();
                    Thread.sleep(5 * 1000);
                    System.out.println("Retrying to contact slave tester");
                }
            }

            for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                if (clientConfig != FreeSwitchTester.getTesterConfig().getTesterClientConfig()) {

                    clientConfig.getXmlRpcClient().execute(TESTER + ".start", (Object[]) null);
                } else {
                    sStart();
                }

            }
            Thread.sleep(testerConfig.getTestDuration() * 1000);
            double cumulativeResult = sGetCumulativeResult();
            double worstCase = cumulativeResult;
            for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                if (clientConfig != FreeSwitchTester.getTesterConfig().getTesterClientConfig()) {
                    Double result = (Double) clientConfig.getXmlRpcClient().execute(
                            TESTER + ".getCumulativeResult", (Object[]) null);
                    if ( result > 0 ) {
                        cumulativeResult += result;
                        if ( result > worstCase) worstCase = result;
                    }
                    
                }
            }
          
            Double averageResult = cumulativeResult / testerConfig.getTesterClients().size();
            File resultFile = new File("power.txt");
            boolean exists = resultFile.exists();
            
            FileWriter fileWriter = new FileWriter(resultFile, true);
            
           
            PrintWriter pwriter = new PrintWriter(fileWriter);
            if ( !exists) {
                pwriter.println("# Conference size vs. Power deviation ");
            }
            
            
            pwriter.println(clientCount + " " + averageResult);
            fileWriter.close();
            
            resultFile = new File("worst-case-power-deviation.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
            pwriter.println(clientCount + " " + worstCase);
            fileWriter.close();
            
            
            Double cumulativeJitter = sGetCumulativeJitter();
            Double worstCaseJitter = cumulativeJitter;
            for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                if (clientConfig != FreeSwitchTester.getTesterConfig().getTesterClientConfig()) {
                    Double result = (Double) clientConfig.getXmlRpcClient().execute(
                            TESTER + ".getCumulativeJitter", (Object[]) null);
                    if ( result > 0 ) {
                        cumulativeJitter += result;
                        if ( result > worstCaseJitter ) {
                            worstCaseJitter = result;
                        }
                    }
                }
            }
            double averageJitter = 0.0;

            averageJitter = cumulativeJitter /testerConfig.getTesterClients().size() ;
            resultFile = new File("jitter.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
            pwriter.println(clientCount + " "  + averageJitter);
            fileWriter.close();
           
            resultFile = new File("worst-case-jitter.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
            pwriter.println(clientCount + " "  + worstCaseJitter);
       
            fileWriter.close();
            
            for (TesterClientConfig clientConfig : testerConfig.getTesterClients()) {
                if (clientConfig != FreeSwitchTester.getTesterConfig().getTesterClientConfig()) {
                    clientConfig.getXmlRpcClient().execute(TESTER + ".stop", (Object[]) null);
                }
            }
            
            
            System.out.println("See power.txt and jitter.txt for the result");
            System.exit(0);
        } else if (testerConfig.isFirstClient()) {
            Thread.sleep(testerConfig.getTestDuration() * 1000);
            ProtocolObjects.stop();
            File resultFile = new File("power.txt");
            FileWriter fileWriter = new FileWriter(resultFile, true);
            PrintWriter pwriter = new PrintWriter(fileWriter);
            double cumulativeResult = sGetCumulativeResult();
            double averageResult = cumulativeResult ;
            pwriter.println(clientCount + "  " + averageResult);
            fileWriter.close();
            
            resultFile = new File("worst-case-power-deviation.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
          
            pwriter.println(clientCount + "  " + averageResult);
            fileWriter.close();
            
            resultFile = new File("jitter.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
            cumulativeResult = sGetCumulativeJitter();
            averageResult = cumulativeResult ;

            pwriter.println(clientCount + "  " + averageResult);
            fileWriter.close();
            
            resultFile = new File("worst-case-jitter.txt");
            fileWriter = new FileWriter(resultFile, true);
            pwriter = new PrintWriter(fileWriter);
            cumulativeResult = sGetCumulativeJitter();
            averageResult = cumulativeResult ;

            pwriter.println(clientCount + "  " + averageResult);
            fileWriter.close();
            
            System.exit(0);
        } else {
            System.out.println("Waiting for command");
        }

    }

   

}
