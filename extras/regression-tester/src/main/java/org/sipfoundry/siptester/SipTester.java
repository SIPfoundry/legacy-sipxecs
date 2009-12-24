package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.message.RequestExt;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.Timer;
import java.util.concurrent.ConcurrentSkipListSet;

import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.header.HeaderFactory;
import javax.sip.header.ViaHeader;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;

import org.apache.log4j.Appender;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class SipTester {

    private static SipTesterConfig testerConfig;

    private static SipStackBean sipStackBean;

    private static SutConfig sutConfig;

    private static Appender appender;

    public static final Timer timer = new Timer();

    private static Hashtable<String, String> actualUserToTestUserMap = new Hashtable<String, String>();

    private static Logger logger = Logger.getLogger(SipTester.class.getPackage().getName());

    private static ValidUsersXML sutValidUsers;

    private static String sutDomainName;
    
    private static PrintWriter schedule;
    
    protected static  Map<String,SipClientTransaction> clientTransactionMap = new HashMap<String,SipClientTransaction>();
    
    protected static  Map<String,SipServerTransaction> serverTransactionMap = new HashMap<String,SipServerTransaction>();

    protected static Map<String,SipDialog> sipDialogs = new HashMap<String,SipDialog>();

    static {
        try {
            appender = new SipFoundryAppender(new SipFoundryLayout(), "sipxtester.log");
            logger.addAppender(appender);
            schedule = new PrintWriter( new File ("schedule.xml"));
        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }
    }

    /*
     * Key is ipAddress:port. This is the map from the original machine. TODO - fill this up.
     */
    private static Hashtable<String, SutUA> endpoints = new Hashtable<String, SutUA>();

    public static Endpoint getEndpoint(String sourceAddress, int port) {
        String key = sourceAddress + ":" + port;
        if (endpoints.get(key) != null) {
            return endpoints.get(key).getEndpoint();
        } else {
            return null;
        }
    }

    public static Collection<Endpoint> getEndpoints() {
        Collection<Endpoint> retval = new HashSet<Endpoint>();
        for (SutUA sutUa : endpoints.values()) {
            retval.add(sutUa.getEndpoint());
        }
        return retval;
    }


    public static SipTesterConfig getTesterConfig() {
        return testerConfig;
    }

    public static SipStackBean getStackBean() {
        return sipStackBean;
    }
    public static SipDialog getDialog(String dialogId) {
        if ( dialogId == null ) return null;
        else {
             SipDialog sipDialog = sipDialogs.get(dialogId) ;
             if ( sipDialog != null ) {
                 return sipDialog;
             } else {
                 sipDialog = new SipDialog();
                 sipDialogs.put(dialogId, sipDialog);
                 return sipDialog;
             }
        }
    }
    public static Collection<SipServerTransaction> findMatchingServerTransaction(
            SipClientTransaction sipClientTransaction) {
        String callId = ((RequestExt) sipClientTransaction.getSipRequest().getSipRequest())
                .getCallIdHeader().getCallId();
        String method = ((RequestExt) sipClientTransaction.getSipRequest().getSipRequest())
                .getMethod();

        logger.debug("Checking "
                + ((RequestExt) sipClientTransaction.getSipRequest().getSipRequest())
                        .getFirstLine());

        ConcurrentSkipListSet<SipServerTransaction> retval = new ConcurrentSkipListSet<SipServerTransaction>();
        for (Endpoint endpoint : getEndpoints()) {

            /*
             * TODO: use the References header here for transitive closure. We also need to add
             * the via header branch to the References header.
             */
            Iterator<SipServerTransaction> it1 = endpoint.getServerTransactions().iterator();
            while (it1.hasNext()) {
                SipServerTransaction st = it1.next();
                String callId1 = st.getSipRequest().getSipRequest().getCallIdHeader().getCallId();
                String method1 = st.getSipRequest().getSipRequest().getMethod();
                if (callId1.equals(callId) && method1.equals(method)) {
                    if (st.getSipRequest().getSipRequest().getHeader("References") == null) {
                        String bottomBranch;
                        ViaHeader bottomVia = null;
                        Iterator<ViaHeader> it = st.getSipRequest().getSipRequest().getHeaders(
                                ViaHeader.NAME);
                        while (it.hasNext()) {
                            bottomVia = it.next();
                        }
                        bottomBranch = bottomVia.getBranch();
                        if (bottomBranch.equals(sipClientTransaction.getSipRequest()
                                .getSipRequest().getTopmostViaHeader().getBranch())) {
                            retval.add(st);
                            logger.debug("Found " + endpoint.getSutUA().getIpAddress()
                                    + " port = " + endpoint.getSutUA().getPort());

                        }
                        break;
                    } else
                        throw new SipTesterException("TODO -- handle B2BUA traces");
                }

            }
        }

        return retval;
    }

    public static AddressFactory getAddressFactory() {
        return sipStackBean.getAddressFactory();
    }

    public static HeaderFactory getHeaderFactory() {
        return sipStackBean.getHeaderFactory();
    }

    public static MessageFactory getMessageFactory() {
        return sipStackBean.getMessageFactory();
    }

    public static void addUserMap(String actualUser, String testUser) {
        actualUserToTestUserMap.put(actualUser, testUser);
    }

    public static String getTestUser(String actualUser) {
        String testUser = actualUserToTestUserMap.get(actualUser);
        if ( testUser == null ) {
           for ( User user : sutValidUsers.GetUsers()) {
               if (user.getAliases().contains(actualUser)) {
                   testUser = actualUserToTestUserMap.get(user.getUserName());
                   break;
               }
           }
        }
        return testUser;
    }

    public static SutUA getSutUA(String user) {
        for (SutUA sutUa : SipTester.endpoints.values()) {
            if (sutUa.registrations.contains(user))
                return sutUa;
        }
        return null;
    }

    public static void fail(String string, Exception ex) {
        logger.error(string, ex);
        System.err.println("Exception during processing ");
        ex.printStackTrace();
        System.exit(-1);
    }

    public static void fail(String string) {
        System.err.println(string);
        System.exit(-1);
    }

    public static ValidUsersXML getSutValidUsers() {
        return sutValidUsers;
    }

    /**
     * @param sutDomainName the sutDomainName to set
     */
    public static void setSutDomainName(String sutDomainName) {
        SipTester.sutDomainName = sutDomainName;
    }

    /**
     * @return the sutDomainName
     */
    public static String getSutDomainName() {
        return sutDomainName;
    }

    /**
     * Generate a mapping between actual users and test users. Test users must not be actual
     * users. These two spaces are kept separate.
     * 
     * @throws Exception
     */
    public static void mapUsers() throws Exception {
        HashSet<String> actualUsers = new HashSet<String>();

        for (User user : SipTester.sutValidUsers.GetUsers()) {
            System.out.println(user.getIdentity());
            SipURI sipUri = (SipURI) getAddressFactory().createURI("sip:" + user.getIdentity());
            if (!testerConfig.getTestUsers().contains(sipUri.getUser())
                    && !sipUri.getUser().startsWith("*")) {
                actualUsers.add(sipUri.getUser());
            }
        }

        Iterator<String> it = SipTester.getTesterConfig().getTestUsers().iterator();
        Iterator<String> it1 = actualUsers.iterator();

        /*
         * Note: if we dont have enough test users, we will throw exception.
         */
        try {
            while (it1.hasNext()) {
                String testUser = it.next();
                String actualUser = it1.next();
                SipTester.addUserMap(actualUser, testUser);
            }
        } catch (Exception ex) {
            throw new SipTesterException(
                    "Could not map actual to test users - check configuration.", ex);
        }
    }

    /**
     * @param args
     */
    public static void main(String[] args) throws Exception {
        try {

            /*
             * The config dir name is where the sipx config information for our system is stored.
             */
            String traceprefix = System.getProperty("sut.prefix.dir", "testcase");
            String traceConfDir = traceprefix + "/etc/sipxpbx/";

            String testerConfigFile = System.getProperty("testerConfig", "tester-config.xml");
            String sutConfigFile = System.getProperty("sutconfig", traceprefix
                    + "/sut-config.xml");
            if (!new File(testerConfigFile).exists() || !new File(sutConfigFile).exists()) {
                System.err.println("Missing config file");
                return;
            }

            testerConfig = new TesterConfigParser().parse("file:" + testerConfigFile);
            sutConfig = new SutConfigParser().parse("file:" + sutConfigFile);
            ValidUsersXML.setValidUsersFileName(traceprefix + "/etc/sipxpbx/validusers.xml");
            sutValidUsers = ValidUsersXML.update(logger, true);

            // TEST only
            String confDir = System.getProperty("conf.dir", "/usr/local/sipx/etc/sipxpbx");

            ValidUsersXML.setValidUsersFileName(confDir + "/validusers.xml");
            ValidUsersXML.update(logger, true);
            File traceDomainConfigFile = new File(traceConfDir + "/domain-config");
            BufferedReader bufferedReader = new BufferedReader(new FileReader(
                    traceDomainConfigFile));
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                String[] parts = line.split(":");
                if (parts[0].trim().equals("SIP_DOMAIN_NAME")) {
                    setSutDomainName(parts[1].trim());
                }
            }

            bufferedReader.close();
            /*
             * Read the domain config file for the current system.
             */
            File domainConfigFile = new File(confDir + "/domain-config");

            bufferedReader = new BufferedReader(new FileReader(domainConfigFile));
            while ((line = bufferedReader.readLine()) != null) {
                String[] parts = line.split(":");
                if (parts[0].trim().equals("SIP_DOMAIN_NAME")) {
                    testerConfig.setSipxProxyDomain(parts[1].trim());
                }
            }
            for (SutUA sutUa : sutConfig.getSutUACollection()) {
                String key = sutUa.getIpAddress() + ":" + sutUa.getPort();

                int port = SipTesterConfig.getPort();
                String ipAddress = testerConfig.getTesterIpAddress();
                Endpoint endpoint = new Endpoint(ipAddress, port);
                sipStackBean = new SipStackBean(endpoint);
                sutUa.setEndpoint(endpoint);
                endpoint.setSutUA(sutUa);
                SipTester.endpoints.put(key, sutUa);
                ListeningPointAddressImpl tcpListeningPointAddress = new ListeningPointAddressImpl(
                        endpoint, "tcp");
                ListeningPointAddressImpl udpListeningPointAddress = new ListeningPointAddressImpl(
                        endpoint, "udp");
                sipStackBean.addListeningPoint(tcpListeningPointAddress);
                sipStackBean.addListeningPoint(udpListeningPointAddress);
                sipStackBean.init();

                /*
                 * Initialize the stack before the listening point and provider can be accessed.
                 */
                endpoint.addListeningPoint(tcpListeningPointAddress.getListeningPoint());
                endpoint.addListeningPoint(udpListeningPointAddress.getListeningPoint());

                endpoint.setProvider("udp", udpListeningPointAddress.getSipProvider());

                endpoint.setProvider("tcp", tcpListeningPointAddress.getSipProvider());

                endpoint.setSipStack(sipStackBean);

            }

            System.out.println("Analyzing trace from file");
            // LogFileReader logFileReader = new LogFileReader(traceprefix + "/var/log/sipxpbx");
            // logFileReader.readTraces();
            TraceAnalyzer traceAnalyzer = new TraceAnalyzer(traceprefix
                    + "/var/log/sipxpbx/merged.xml");
            traceAnalyzer.analyze();
            System.out.println("Completed reading trace");

            long startTime = Long.MAX_VALUE;

            /*
             * The list of transactions that are runnable.
             */
            ConcurrentSkipListSet<SipClientTransaction> runnable = new ConcurrentSkipListSet<SipClientTransaction>();
            for (SutUA sutUa : endpoints.values()) {
                Endpoint endpoint = sutUa.getEndpoint();
                logger.debug("endpoint " + endpoint.getSutUA().getIpAddress() + "/"
                        + endpoint.getSutUA().getPort());
                endpoint.printServerTransactions();
                Iterator<SipClientTransaction> it = endpoint.getClientTransactions().iterator();
                while (it.hasNext()) {

                    SipClientTransaction ct = it.next();
                    ct.printTransaction();
                    /*
                     * This transaction is runnable.
                     */
                    runnable.add(ct);

                    if (ct.getDelay() < startTime) {
                        startTime = ct.getDelay();
                    }
                    logger.debug("method = " + ct.getSipRequest().getSipRequest().getMethod());
                    HashSet<String> dialogIds = ct.getDialogIds();
                    logger.debug("dialog Ids " + dialogIds);
                    for (String dialogId : dialogIds) {
                        SipDialog dialog = getDialog(dialogId);
                        logger.debug("dialog = " + dialog);
                        dialog.addSipClientTransaction(ct);
                    }
                    Collection<SipServerTransaction> serverTransactions = findMatchingServerTransaction(ct);
                    if (!serverTransactions.isEmpty()) {
                        logger.debug("ClientTransaction "
                                + ct.getSipRequest().getSipRequest().getMethod() + " time "
                                + ct.getDelay());

                        Iterator<SipServerTransaction> sstIt = serverTransactions.iterator();

                        while (sstIt.hasNext()) {
                            SipServerTransaction sst = sstIt.next();
                            logger.debug("sst = "
                                    + sst.getSipRequest().getSipRequest().getFirstLine()
                                    + " time = " + sst.getDelay() + " dialogId "
                                    + sst.getDialogId());
                            String dialogId = sst.getDialogId();
                            if ( dialogId != null ) {

                                SipDialog dialog = getDialog(dialogId);

                                dialog.addSipServerTransaction(sst);
                            }
                        }

                    }
                    ct.addMatchingServerTransactions(serverTransactions);

                }

            }

            mapUsers();

            /*
             * Determine the happensBefore set of each tx that is runnable.
             */
            Iterator<SipClientTransaction> runIt = runnable.iterator();

            /*
             * For each element of the runnable set, record all the previous clientTransactions
             * that must run. This builds the dependency list of requests and responses
             * that must be sent or received before this transaction can run.
             */
            while (runIt.hasNext()) {
                SipClientTransaction currentTx = runIt.next();
                Iterator<SipClientTransaction> innerIt = runnable.iterator();
                while (innerIt.hasNext()) {
                    SipClientTransaction previousTx = innerIt.next();            
                    for (SipResponse sipResponse : previousTx.sipResponses) {
                        if (sipResponse.getTime() < currentTx.getTime()) {
                            currentTx.addHappensBefore(sipResponse);
                            /*
                             * Augment the set of client transactions that
                             */
                            sipResponse.addPostCondition(currentTx);
                        }
                    }
                    
                 }
                Iterator<SipClientTransaction> it1 = runnable.iterator();
                while(it1.hasNext()) {
                    SipClientTransaction previousTx = it1.next();
                    for (SipServerTransaction sst : previousTx.getMatchingServerTransactions()) {
                        if ( sst.getTime() < currentTx.getTime() ) {
                           currentTx.addHappensBefore(sst.getSipRequest());
                           sst.getSipRequest().addPostCondition(currentTx);
                        }
                    }
                }

            }

            runIt = runnable.iterator();

            logger.debug("=============== DEPENDENCY MAP =================");
            while (runIt.hasNext()) {
                SipClientTransaction currentTx = runIt.next();
                currentTx.printTransaction();
            }

            schedule.flush();
            
            logger.debug("==============SERVER TRANSACTIONS =============");
            for (Endpoint endpoint : getEndpoints()) {
                endpoint.printServerTransactions();
            }

            logger.debug("Map = " + SipTester.actualUserToTestUserMap);
            System.out.println("startTime " + startTime);

            System.out.println("UserMap = " + SipTester.actualUserToTestUserMap);

            // --- Mappings are completed at this point. Now we can start the emulation -----

            for (Endpoint endpoint : getEndpoints()) {

                endpoint.getStackBean().getSipListener().sendRegistration();
            }

            Thread.sleep(500);

            for (SutUA sutUa : endpoints.values()) {
                sutUa.getEndpoint().runEmulatedCallFlow(startTime);
            }

            while (true) {
                Thread.sleep(1000);
                boolean doneFlag = true;
                for (SutUA sutUa : endpoints.values()) {
                    if (!sutUa.getEndpoint().doneFlag) {
                        doneFlag = false;
                    }
                }
                if (doneFlag) {
                    System.out.println("all done!");
                    System.exit(1);
                }
            }

        } catch (Exception ex) {
            ex.printStackTrace();
            System.exit(-1);
        }
    }

    public static Endpoint getEndpoint(ListeningPointExt listeningPoint) {
        for (Endpoint endpoint : SipTester.getEndpoints()) {
            if (endpoint.getListeningPoints().contains(listeningPoint))
                return endpoint;
        }
        return null;
    }

    public static boolean isExcluded(String method) {
        if (method.equals(Request.SUBSCRIBE) || method.equals(Request.ACK))
            return true;
        else
            return false;
    }
    
    public static SipServerTransaction getSipServerTransaction(String transactionId) {
        return SipTester.serverTransactionMap.get(transactionId);
    }

    public static PrintWriter getPrintWriter() {
        return schedule;
    }

    public static SipClientTransaction getSipClientTransaction(String transactionId) {
        return clientTransactionMap.get(transactionId);
    }

}
