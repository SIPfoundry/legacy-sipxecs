package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListSet;

import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class EmulatedEndpoint extends HostPort {

    private static Logger logger = Logger.getLogger(EmulatedEndpoint.class);

    private Hashtable<String, ListeningPoint> listeningPoints = new Hashtable<String, ListeningPoint>();

    private Collection<SipClientTransaction> clientTransactions = new ConcurrentSkipListSet<SipClientTransaction>();

    private Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();

    private Map<String, SipProviderExt> sipProviders = new HashMap<String, SipProviderExt>();

    Hashtable<String, SipDialog> sipDialogs = new Hashtable<String, SipDialog>();

    boolean doneFlag;

    private SipStackBean sipStackBean;

    private TraceEndpoint traceEndpoint;

    private long startTime;
    
    private HashMap<String,String> branchMap = new HashMap<String,String>();


    public EmulatedEndpoint(String ipAddress, int port) {
        super(ipAddress, port);
    }

    public void setTraceEndpoint(TraceEndpoint traceEndpoint) {
        this.traceEndpoint = traceEndpoint;
    }

    public TraceEndpoint getTraceEndpoint() {
        return this.traceEndpoint;
    }

    /**
     * @param ipAddress the ipAddress to set
     */
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }

    /**
     * @return the ipAddress
     */
    public String getIpAddress() {
        return ipAddress;
    }

    /**
     * @param port the port to set
     */
    public void setPort(int port) {
        this.port = port;
    }

    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }

    public void removeUnEmulatedClientTransactions(Collection<SipClientTransaction> emulatedSet) {
        Iterator<SipClientTransaction> it = this.clientTransactions.iterator();
        while (it.hasNext()) {
            SipClientTransaction ctx = it.next();
            if (!emulatedSet.contains(ctx)) {
                it.remove();
            }
        }

    }

    public void addEmulatedServerTransaction(SipServerTransaction serverTx) {
    	if (serverTx.isOfInterest()) {
			this.serverTransactions.add(serverTx);
			serverTx.setEndpoint(this);
			logger.debug("addEmulatedServerTransaction " + this.ipAddress + ":"
					+ this.port + " " + serverTx.getSipRequest().getFrameId());
		}

    }

    public void addEmulatedClientTransaction(SipClientTransaction clientTx) {
        this.clientTransactions.add(clientTx);
        clientTx.setEndpoint(this);
    }

    public void addOriginatingSipResponse(SipResponse sipResponse) {
        String transactionid = ((SIPResponse) sipResponse.getSipResponse()).getTransactionId();
        SipServerTransaction serverTx = SipTester.getSipServerTransaction(transactionid);
        if (serverTx == null) {
            logger.debug("Cannot find serverTx for transmitted response");
            return;
        }
        serverTx.addResponse(sipResponse);
    }

    public Collection<SipClientTransaction> getClientTransactions() {
        return this.clientTransactions;
    }

    public Collection<SipServerTransaction> getServerTransactions() {
        return this.serverTransactions;
    }

    public void addListeningPoint(ListeningPoint listeningPoint) {

        this.listeningPoints.put(listeningPoint.getTransport().toLowerCase(), listeningPoint);
    }

    public void setSipStack(SipStackBean sipStackBean) {
        this.sipStackBean = sipStackBean;
    }

    public SipStackBean getStackBean() {
        return this.sipStackBean;
    }

    public ListeningPointExt getDefaultListeningPoint() {
        String transport = this.traceEndpoint.getDefaultTransport();
        return (ListeningPointExt) this.getProvider(transport).getListeningPoint(transport);
    }

    public Collection<ListeningPoint> getListeningPoints() {
        return this.listeningPoints.values();
    }

    public ListeningPointExt getListeningPoint(String transport) {
        return (ListeningPointExt) this.listeningPoints.get(transport.toLowerCase());
    }

    public void setProvider(String transport, SipProvider sipProvider) {
        this.sipProviders.put(transport.toUpperCase(), (SipProviderExt) sipProvider);
    }

    public SipProviderExt getProvider(String transport) {
        return this.sipProviders.get(transport.toUpperCase());
    }

    public void runEmulatedCallFlow(long startTime) throws Exception {
        this.startTime = startTime;
        new Thread(new Runnable() {
            public void run() {

                if (clientTransactions.isEmpty() ) {
                    System.out.println(traceEndpoint.getTraceIpAddresses() +  " Nothing to run");
                    doneFlag = true;
                    return;
                }
                try {
                    long prevTime = EmulatedEndpoint.this.startTime;
                    Iterator<SipClientTransaction> it = clientTransactions.iterator();

                    while (it.hasNext()) {
                        SipClientTransaction ctx = it.next();
                        long timeToSleep = ctx.getDelay() - prevTime;
                        if (timeToSleep > 0) {
                            Thread.sleep(timeToSleep);
                        }
                        prevTime = ctx.getDelay();
                        logger.debug("aboutToEmulate "
                                + ctx.getSipRequest().getSipRequest().getMethod() + " tid = "
                                + ctx.getTransactionId() + " frameId "
                                + ctx.getSipRequest().getFrameId());
                        if (ctx.checkPreconditions() && !ctx.processed) {
                            ctx.createAndSend();
                        } else {
                            System.out.println("Could not emulate " + ctx.getSipRequest().getFrameId());
                        }
                    }
                    doneFlag = true;
                } catch (Exception ex) {
                    ex.printStackTrace();
                    System.exit(-1);
                }
            }
        }).start();
    }

    public Collection<SipServerTransaction> findSipServerTransaction(Request request) {
        Collection<SipServerTransaction> retval = new HashSet<SipServerTransaction>();
        Iterator<SipServerTransaction> it = this.serverTransactions.iterator();
        while (it.hasNext()) {
            SipServerTransaction sst = it.next();
            if (request.getMethod().equals(sst.getSipRequest().getSipRequest().getMethod())
                    && sst.getBranch() != null
                    && sst.getBranch().equalsIgnoreCase(SipUtilities.getBranchMatchId(request))) {
                it.remove();
                retval.add(sst);
                break;
            }
        }

        if (retval.isEmpty()) {
            logger.debug("Could not find transaction template. Looking for first matching transaction.");
            it = this.serverTransactions.iterator();
            while (it.hasNext()) {
                SipServerTransaction sst = it.next();
                  if ( request.getMethod().equalsIgnoreCase(sst.getSipRequest().getSipRequest().getMethod()) 
                		   /* && sst.getSipRequest().getFrameId() > SipTester.getMaxEmulatedFrame() */ ) {
                    it.remove();
                    retval.add(sst);
                    break;            
                }
            }
         }
        if (retval.isEmpty()) {
            logger.debug("Could not find a matching server transaction for the incoming request");
           
        }
        return retval;
    }
    
    public  boolean isBranchMapped(String method, String branch ) {
		return branchMap.containsKey(method + ":" + branch);
	}
	public void mapBranch(String method, String traceBranch, String emulatedBranch) {
			branchMap.put(method + ":" + traceBranch, emulatedBranch);
	}

    public boolean isSpiral(RequestExt sipRequest) {
		String branch = SipUtilities.getBottomViaHeader(sipRequest).getBranch();
		String method = sipRequest.getMethod();
		return isBranchMapped(method,branch);
	}
	

    public String getMappedBranch(String method, String branch) {
		 if ( branchMap.get(method + ":" + branch) != null ) {
			 return branchMap.get(method + ":" + branch);	 
		 } else return branch;
	}
  

}
