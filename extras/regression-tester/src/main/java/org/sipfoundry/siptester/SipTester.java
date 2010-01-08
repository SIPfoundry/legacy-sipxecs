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
import java.util.concurrent.atomic.AtomicBoolean;

import javax.sip.address.AddressFactory;
import javax.sip.address.SipURI;
import javax.sip.header.ExtensionHeader;
import javax.sip.header.HeaderFactory;
import javax.sip.header.ViaHeader;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Appender;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class SipTester {

    private static SipTesterConfig testerConfig;

    private static SipStackBean sipStackBean;

    private static TraceConfig monitoredInterfaces;

    private static Appender appender;

    public static final Timer timer = new Timer();

    private static Logger logger = Logger.getLogger(SipTester.class.getPackage().getName());

    private static ValidUsersXML sutValidUsers;

    private static String sutDomainName;

    private static PrintWriter schedule;

    protected static Map<String, SipClientTransaction> clientTransactionMap = new HashMap<String, SipClientTransaction>();

    protected static Map<String, SipServerTransaction> serverTransactionMap = new HashMap<String, SipServerTransaction>();

    protected static Map<String, SipDialog> sipDialogs = new HashMap<String, SipDialog>();

    private static Map<String, CapturedLogPacket> capturedPacketMap = new HashMap<String, CapturedLogPacket>();

    static {
        try {
            appender = new SipFoundryAppender(new SipFoundryLayout(), "sipxtester.log");
            logger.addAppender(appender);
            schedule = new PrintWriter(new File("schedule.xml"));
        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }
    }

    /*
     * Key is ipAddress:port. This is the map from the original machine. TODO - fill this up.
     */
    private static Hashtable<String, TraceEndpoint> endpoints = new Hashtable<String, TraceEndpoint>();

    static AtomicBoolean failed = new AtomicBoolean(false);

    private static TestMap testMaps;

    public static EmulatedEndpoint getEndpoint(String sourceAddress, int port) {
        String key = sourceAddress + ":" + port;
        if (endpoints.get(key) != null) {
            return endpoints.get(key).getEmulatedEndpoint();
        } else {
            return null;
        }
    }

    public static Collection<EmulatedEndpoint> getEndpoints() {
        Collection<EmulatedEndpoint> retval = new HashSet<EmulatedEndpoint>();
        for (TraceEndpoint sutUa : endpoints.values()) {
            retval.add(sutUa.getEmulatedEndpoint());
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
        if (dialogId == null)
            return null;
        else {
            SipDialog sipDialog = sipDialogs.get(dialogId);
            if (sipDialog != null) {
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

        logger.debug("Checking " + sipClientTransaction.getTransactionId() + " method " + method);

        ConcurrentSkipListSet<SipServerTransaction> retval = new ConcurrentSkipListSet<SipServerTransaction>();

        /*
         * TODO: use the References header here for transitive closure. We also need to add the
         * via header branch to the References header.
         */
        Iterator<SipServerTransaction> it1 = SipTester.serverTransactionMap.values().iterator();
        logger.debug("serverTransactionMap size = " + SipTester.serverTransactionMap.size());
        while (it1.hasNext()) {
            SipServerTransaction st = it1.next();
            if (st.getEndpoint() != sipClientTransaction.getEndpoint()) {
                String callId1 = st.getSipRequest().getSipRequest().getCallIdHeader().getCallId();
                String method1 = st.getSipRequest().getSipRequest().getMethod();
                ExtensionHeader extensionHeader = (ExtensionHeader) st.getSipRequest()
                        .getSipRequest().getHeader("References");
                String branchId = null;
                String referencesCallId = null;
                if (extensionHeader != null) {
                    /*
                     * Check if the server transaction has a references header that matches the
                     * client transaction. If so we add the server transaction to our set.
                     */
                    String value = extensionHeader.getValue().trim();
                    String[] parts1 = value.split(";rel=");
                    referencesCallId = parts1[0];
                    String[] parts2 = parts1[1].split(";x-sipx-branch=");
                    String rel = parts2[0];
                    if (parts2.length == 2) {
                        branchId = parts2[1].toLowerCase();
                    }
                    logger.debug("referencesHeader branchId = " + branchId);
                    logger.debug("referencesHeader callId = " + referencesCallId);
                }
                boolean found = false;

                if (method1.equals(method)) {
                    if (callId1.equalsIgnoreCase(callId)) {
                        ViaHeader viaHeader = null;
                        Iterator<ViaHeader> it = st.getSipRequest().getSipRequest().getHeaders(
                                ViaHeader.NAME);
                        while (it.hasNext()) {
                            viaHeader = it.next();
                            branchId = viaHeader.getBranch();
                           
                        }
                        logger.debug("checking branch Id "  + branchId);
                        if (branchId.equalsIgnoreCase(sipClientTransaction.getTransactionId()) 
                               ) {
                            retval.add(st);
                            found = true;           
                        }
                    } else {
                        logger.debug("callId does not match " + callId + " / " + callId1);
                    }
                    if (extensionHeader != null && !found && referencesCallId.equals(callId) && 
                        method1.equals(method) && branchId != null) {  
                        logger.debug("branchId = " + branchId);
                        logger.debug("st.matchingClientTransaction = "  + st.getMatchingClientTransaction());
                        if (branchId.equalsIgnoreCase(sipClientTransaction.getTransactionId()) 
                                ) {
                            retval.add(st);
                        }

                    }
                     logger.debug("Checked : " +    st.getSipRequest().getSipRequest());
                     
                }
            }

        }

        if (retval.isEmpty()) {
            logger.debug("no matching server transaction was found");
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

    public static String getMappedUser(String traceUser) {
        String testUser = testMaps.getMappedUser(traceUser);
        if (testUser == null) {
            for (User user : sutValidUsers.GetUsers()) {
                if (user.getAliases().contains(traceUser)) {
                    testUser = testMaps.getMappedUser(user.getUserName());
                    break;
                }
            }
        }
        if (testUser == null)
            return traceUser;
        else
            return testUser;
    }

    public static String getMappedAddress(String traceAddress) {
        String mappedAddress = testMaps.getMappedAddress(traceAddress);
        if (mappedAddress == null) {
            logger.debug("Address Not mapped " + traceAddress);
            return traceAddress;
        } else {
            return mappedAddress;
        }

    }

    public static TraceEndpoint getSutUA(String user) {
        for (TraceEndpoint sutUa : SipTester.endpoints.values()) {
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

    public static ValidUsersXML getTraceValidUsers() {
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
    public static String getTraceDomainName() {
        return sutDomainName;
    }

    public static void addCapturedPacket(CapturedLogPacket capturedPacket) {
        SipTester.capturedPacketMap.put(capturedPacket.getFrameId(), capturedPacket);
    }

    /**
     * @param args
     */
    public static void main(String[] args) throws Exception {
        try {

            /*
             * The config dir name is where the sipx config information for our system is stored.
             */
            String traceprefix = System.getProperty("test.prefix");
            if (traceprefix == null || !new File(traceprefix).isDirectory()) {
                System.err.println("Missing test directory  - please specify trace.dir");
            }
            String traceConfDir = traceprefix + "/trace/etc/sipxpbx/";

            String testerConfigFile = System.getProperty("testerConfig", "tester-config.xml");
            String sutConfigFile = traceprefix + "/monitored-interfaces.xml";
            String testMapsFile = traceprefix + "/test-maps.xml";

            if (!new File(testerConfigFile).exists() || !new File(sutConfigFile).exists()) {
                System.err.println("Missing config file");
                return;
            }

            testMaps = new TestMapParser().parse("file:" + testMapsFile);

            testerConfig = new TesterConfigParser().parse("file:" + testerConfigFile);
            monitoredInterfaces = new SutConfigParser().parse("file:" + sutConfigFile);
            ValidUsersXML
                    .setValidUsersFileName(traceprefix + "/trace/etc/sipxpbx/validusers.xml");
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
            for (TraceEndpoint traceEndpoint : monitoredInterfaces.getTraceEndpoints()) {
                String key = traceEndpoint.getIpAddress() + ":" + traceEndpoint.getPort();

                int port = traceEndpoint.getEmulatedPort();
                String ipAddress = testerConfig.getTesterIpAddress();
                EmulatedEndpoint endpoint = new EmulatedEndpoint(ipAddress, port);
                sipStackBean = new SipStackBean(endpoint);
                traceEndpoint.setEmulatedEndpoint(endpoint);
                endpoint.setSutUA(traceEndpoint);
                SipTester.endpoints.put(key, traceEndpoint);
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

            System.out.println("Analyzing trace from file: " + traceprefix
                    + "/trace/var/log/sipxpbx/merged.xml");

            TraceAnalyzer traceAnalyzer = new TraceAnalyzer(traceprefix
                    + "/trace/var/log/sipxpbx/merged.xml");
            traceAnalyzer.analyze();
            System.out.println("Completed reading trace");

            long startTime = Long.MAX_VALUE;

            /*
             * The list of transactions that are runnable.
             */
            ConcurrentSkipListSet<SipClientTransaction> runnable = new ConcurrentSkipListSet<SipClientTransaction>();
            for (TraceEndpoint traceEndpoint : endpoints.values()) {
                EmulatedEndpoint endpoint = traceEndpoint.getEmulatedEndpoint();
                logger.debug("endpoint " + endpoint.getSutUA().getIpAddress() + "/"
                        + endpoint.getSutUA().getPort());
                Iterator<SipClientTransaction> it = endpoint.getClientTransactions().iterator();
                while (it.hasNext()) {

                    SipClientTransaction ct = it.next();

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
                            if (dialogId != null) {

                                SipDialog dialog = getDialog(dialogId);

                                dialog.addSipServerTransaction(sst);
                            }
                        }

                    } else {
                        logger
                                .debug("could not find matching server transactions for client transaction");
                    }
                    ct.addMatchingServerTransactions(serverTransactions);

                    /*
                     * This transaction is runnable.
                     */
                    if (traceEndpoint.getBehavior().equals(Behavior.UA)) {
                        runnable.add(ct);
                    } else if (traceEndpoint.getBehavior().equals(Behavior.PROXY)) {
                        /*
                         * A proxy transaction is only emulated if we emulate both the client and
                         * server side of the transaction.
                         */
                        if (!ct.getMatchingServerTransactions().isEmpty()) {
                            logger.debug("adding client transaction to runnable");
                            runnable.add(ct);
                        } else {
                            logger.debug("no matching server transaction found");
                            logger.debug("ct frame = " + ct.getSipRequest().getFrameId());
                        }
                    }

                }

            }

            /*
             * Determine the happensBefore set of each tx that is runnable.
             */
            Iterator<SipClientTransaction> runIt = runnable.iterator();

            /*
             * For each element of the runnable set, record all the previous clientTransactions
             * that must run. This builds the dependency list of requests and responses that must
             * be sent or received before this transaction can run.
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
                while (it1.hasNext()) {
                    SipClientTransaction previousTx = it1.next();
                    for (SipServerTransaction sst : previousTx.getMatchingServerTransactions()) {
                        if (sst.getSipRequest().getFrameId() < currentTx.getSipRequest()
                                .getFrameId()) {
                            currentTx.addHappensBefore(sst.getSipRequest());
                            sst.getSipRequest().addPostCondition(currentTx);
                        }
                    }
                }
                it1 = runnable.iterator();
                while (it1.hasNext()) {
                    SipClientTransaction transaction = it1.next();
                    transaction.setPreconditionSem();

                }
            }

            for (TraceEndpoint traceEndpoint : endpoints.values()) {
                traceEndpoint.getEmulatedEndpoint().removeUnEmulatedClientTransactions(runnable);
            }

            if (runnable.isEmpty()) {
                System.out.println("Nothing to run!!");
                System.exit(0);
            }

            runIt = runnable.iterator();

            logger.debug("=============== DEPENDENCY MAP =================");

            while (runIt.hasNext()) {
                SipClientTransaction currentTx = runIt.next();
                currentTx.printTransaction();
            }
            getPrintWriter().println("<!--  SERVER TRANSACTIONS -->");
            runIt = runnable.iterator();
            while (runIt.hasNext()) {
                SipClientTransaction currentTx = runIt.next();
                for (SipServerTransaction sst : currentTx.getMatchingServerTransactions()) {
                    sst.printServerTransaction();
                }
            }
            getPrintWriter().println("<!--  SERVER TRANSACTIONS -->");

            schedule.flush();
            schedule.close();

            System.out.println("startTime " + startTime);

            // --- Mappings are completed at this point. Now we can start the emulation -----

            Thread.sleep(500);

            for (TraceEndpoint traceEndpoint : endpoints.values()) {
                traceEndpoint.getEmulatedEndpoint().runEmulatedCallFlow(startTime);
            }

            while (true) {
                Thread.sleep(1000);
                boolean doneFlag = true;
                for (TraceEndpoint traceEndpoint : endpoints.values()) {
                    if (!traceEndpoint.getEmulatedEndpoint().doneFlag) {
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

    public static EmulatedEndpoint getEndpoint(ListeningPointExt listeningPoint) {
        for (EmulatedEndpoint endpoint : SipTester.getEndpoints()) {
            if (endpoint.getListeningPoints().contains(listeningPoint))
                return endpoint;
        }
        return null;
    }

    public static boolean isExcluded(String method) {
        if (method.equals(Request.ACK))
            return true;
        else
            return false;
    }

    public static SipServerTransaction getSipServerTransaction(String transactionId) {
        return SipTester.serverTransactionMap.get(transactionId.toLowerCase());
    }

    public static PrintWriter getPrintWriter() {
        return schedule;
    }

    public static SipClientTransaction getSipClientTransaction(String transactionId) {
        return clientTransactionMap.get(transactionId.toLowerCase());
    }

    public static boolean checkProvisionalResponses() {
        return false;
    }

}
