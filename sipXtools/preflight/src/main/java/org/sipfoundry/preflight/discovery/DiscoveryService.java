package org.sipfoundry.preflight.discovery;

import javax.sip.*;
import javax.sip.address.*;
import javax.sip.header.*;
import javax.sip.message.*;

import org.sipfoundry.ao.ActiveObjectGroupImpl;

import java.util.*;
import java.text.*;

/**
 * @author Mardy Marshall
 */

public class DiscoveryService extends ActiveObjectGroupImpl<String> implements SipListener {
    private String localHostAddress;
    private SipFactory sipFactory;
    private SipProvider sipProvider;
    private AddressFactory addressFactory;
    private MessageFactory messageFactory;
    private HeaderFactory headerFactory;
    private SipStack sipStack;
    private ContactHeader contactHeader;
    private ListeningPoint listeningPoint;
    private CallIdHeader callIdHeader;
    private CSeqHeader cSeqHeader;
    private FromHeader fromHeader;
    private ArrayList<ViaHeader> viaHeaders;
    private MaxForwardsHeader maxForwards;
    private final String localHostTransport = "udp";
    private LinkedList<Device> devices;
    
    private class UAVendor {
        public final String name;
        public final String oui;
        
        public UAVendor(String name, String oui) {
            this.name = name;
            this.oui = oui;
        }
    }
    
    private final UAVendor[] UAVendorList = {
            new UAVendor("Aastra", "00:08:5D"),
            new UAVendor("Aastra", "00:10:BC"),
            new UAVendor("Grandstream", "00:0B:82"),
            new UAVendor("LG-Nortel", "00:40:5A"),
            new UAVendor("LG-Nortel", "00:1A:7E"),
            new UAVendor("Linksys", "00:0E:08"),
            new UAVendor("MITEL", "08:00:0F"),
            new UAVendor("Pingtel Xpressa", "00:D0:1E"),
            new UAVendor("Polycom", "00:04:F2"),
            new UAVendor("SNOM", "00:04:13"),
    };

    public DiscoveryService(String localHostAddress, int localHostPort) {
        super("PreflightDiscovery", Thread.NORM_PRIORITY, 10, 60);

        this.localHostAddress = localHostAddress;
        sipFactory = SipFactory.getInstance();
        sipFactory.setPathName("gov.nist");

        devices = new LinkedList<Device>();
        
        Properties properties = new Properties();
        properties.setProperty("javax.sip.STACK_NAME", "Preflight");
        properties.setProperty("gov.nist.javax.sip.MAX_MESSAGE_SIZE", "1048576");
        properties.setProperty("gov.nist.javax.sip.DEBUG_LOG", "PreflightDebug.txt");
        properties.setProperty("gov.nist.javax.sip.SERVER_LOG", "PreflightLog.txt");
        properties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", "0");
        // Drop the client connection after we are done with the transaction.
        properties.setProperty("gov.nist.javax.sip.CACHE_CLIENT_CONNECTIONS", "false");

        try {
            // Create SipStack object
            sipStack = sipFactory.createSipStack(properties);
        } catch (PeerUnavailableException e) {
            // could not find gov.nist.jain.protocol.ip.sip.SipStackImpl in the classpath.
            e.printStackTrace();
            System.err.println(e.getMessage());
            System.exit(0);
        }
        try {
            headerFactory = sipFactory.createHeaderFactory();
            addressFactory = sipFactory.createAddressFactory();
            messageFactory = sipFactory.createMessageFactory();
            listeningPoint = sipStack.createListeningPoint(localHostAddress, localHostPort, localHostTransport);
            sipProvider = sipStack.createSipProvider(listeningPoint);
            DiscoveryService listener = this;
            sipProvider.addSipListener(listener);

        } catch (PeerUnavailableException e) {
            e.printStackTrace();
            System.err.println(e.getMessage());
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
        }

        String fromName = "preflight";
        String fromSipAddress = "pingtel.com";
        String fromDisplayName = "Preflight";

        try {
            // create From Header
            SipURI fromAddress;
            fromAddress = addressFactory.createSipURI(fromName, fromSipAddress);

            Address fromNameAddress = addressFactory.createAddress(fromAddress);
            fromNameAddress.setDisplayName(fromDisplayName);
            fromHeader = headerFactory.createFromHeader(fromNameAddress, Long.toHexString(System.currentTimeMillis()));

            // Create ViaHeaders
            viaHeaders = new ArrayList<ViaHeader>();

            // Create a new CallId header
            callIdHeader = sipProvider.getNewCallId();

            // Create a new Cseq header
            cSeqHeader = headerFactory.createCSeqHeader(1L, Request.OPTIONS);

            // Create contact headers
            SipURI contactUrl = addressFactory.createSipURI(fromName, localHostAddress);
            contactUrl.setPort(localHostPort);

            // Create the contact name address.
            SipURI contactURI = addressFactory.createSipURI(fromName, localHostAddress);
            contactURI.setPort(localHostPort);

            Address contactAddress = addressFactory.createAddress(contactURI);

            // Add the contact address.
            contactAddress.setDisplayName(fromName);

            contactHeader = headerFactory.createContactHeader(contactAddress);

            // Create a new MaxForwardsHeader
            maxForwards = headerFactory.createMaxForwardsHeader(70);
        } catch (ParseException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvalidArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

    public SipProvider getSipProvider() {
        return sipProvider;
    }

    public void processRequest(RequestEvent requestEvent) {
    }

    public void processResponse(ResponseEvent responseEvent) {
        ClientTransaction tid = responseEvent.getClientTransaction();
        if (tid == null) {
            return;
        }

        DiscoveryAgent discoveryAgent = (DiscoveryAgent) tid.getApplicationData();
        discoveryAgent.processResponse(responseEvent);

    }

    public void processTimeout(javax.sip.TimeoutEvent timeoutEvent) {
        ClientTransaction tid = timeoutEvent.getClientTransaction();
        if (tid != null) {
            DiscoveryAgent discoveryAgent = (DiscoveryAgent) tid.getApplicationData();
            discoveryAgent.processTimeout(timeoutEvent);
        }
    }

    public void processIOException(IOExceptionEvent exceptionEvent) {
        DiscoveryAgent discoveryAgent = (DiscoveryAgent) findInstance(exceptionEvent.getHost());
        discoveryAgent.terminate();
    }

    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
        ClientTransaction tid = transactionTerminatedEvent.getClientTransaction();
        if (tid != null) {
            DiscoveryAgent discoveryAgent = (DiscoveryAgent) tid.getApplicationData();
            discoveryAgent.processTransactionTerminated(transactionTerminatedEvent);
        }
    }

    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        System.err.println("dialogTerminatedEvent");

    }

    public synchronized Request createOptionsRequest(String destinationAddress, int destinationPort) {
        Request request = null;
        String destinationAddressPort = new String(destinationAddress + ":" + new Integer(destinationPort).toString());

        try {
            // Create To Header.
            Address toAddress = addressFactory.createAddress(destinationAddressPort);
            ToHeader toHeader = headerFactory.createToHeader(toAddress, null);

            // Create Request URI.
            SipURI requestURI = addressFactory.createSipURI("", destinationAddressPort);

            // Create the request.
            request = messageFactory.createRequest(requestURI, Request.OPTIONS, callIdHeader, cSeqHeader, fromHeader, toHeader,
                    viaHeaders, maxForwards);

            request.addHeader(contactHeader);
        } catch (ParseException e) {
            e.printStackTrace();
        }

        return request;
    }
    
    public synchronized void addDiscovered(String networkAddress, String userAgentInfo) {
        String hardwareAddress = ArpTable.lookup(networkAddress);
        if (hardwareAddress != null) {
            String vendor = null;
            for (int x = 0; x < UAVendorList.length; x++) {
                if (hardwareAddress.startsWith(UAVendorList[x].oui)) {
                    vendor = UAVendorList[x].name;
                }
            }
            if (vendor != null) {
                devices.add(new Device(hardwareAddress, networkAddress, vendor, userAgentInfo));
            }
        }
    }

    public LinkedList<Device> discover(String network, String networkMask) {
        int[] addr = new int[4];
        String[] fields = network.split("[.]");
        if (fields.length == 4) {
            for (int i = 0; i < 4 ; i++) {
                addr[i] = Integer.parseInt(fields[i]) & 0xFF;
            }
        }
        addr[3] = 1;
        for (int x = 0; x < 254; x++) {
            try {
                //InetAddress target = InetAddress.getByAddress(addr);
                String target = String.format("%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]++);
                if (target.compareTo(localHostAddress) == 0) {
                    // Skip ourself.
                    continue;
                }
                newActiveObject(new DiscoveryAgentImpl(target, this), target);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        
        try {
            while (size() > 0) {
                Thread.sleep(1000);
            }
        } catch (InterruptedException e) {
        }
        
        return devices;

    }

}
