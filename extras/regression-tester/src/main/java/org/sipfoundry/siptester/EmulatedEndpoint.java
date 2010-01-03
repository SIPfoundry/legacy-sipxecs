package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.SIPRequest;
import gov.nist.javax.sip.message.SIPResponse;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentSkipListSet;

import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.address.SipURI;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class EmulatedEndpoint  {
    
    private static Logger logger = Logger.getLogger(EmulatedEndpoint.class);
    
    private String ipAddress;
    
    private int port;
    
    private Hashtable<String,ListeningPoint>  listeningPoints = new Hashtable<String,ListeningPoint>(); 
    
    private Collection<SipClientTransaction> clientTransactions =  new ConcurrentSkipListSet<SipClientTransaction>();
    
    private Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();
     
    private Map<String,SipProviderExt> sipProviders = new HashMap<String,SipProviderExt>();
     
    Hashtable<String,SipDialog> sipDialogs = new Hashtable<String,SipDialog>();
    
    boolean doneFlag;
    
    private SipStackBean sipStackBean;

    private TraceEndpoint traceEndpoint;

    private long startTime;

    private int emulatedPort;
    
    
    public EmulatedEndpoint(String ipAddress, int port) {
        this.ipAddress = ipAddress;
        this.port = port;
    }
    
    
    
    public void setSutUA(TraceEndpoint sutUa) {
        this.traceEndpoint = sutUa;
    }
    
   
    
    public TraceEndpoint getSutUA() {
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
    
    public void setEmulatedPort(int port) {
        this.emulatedPort = port;
    }

    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }
    
    
    public void addOriginatingPacket(CapturedLogPacket logPacket) {
        if ( logPacket.getSipPacket() instanceof SIPRequest ) {
            SipRequest sipRequest = new SipRequest( (SIPRequest) logPacket.getSipPacket(),logPacket.getTimeStamp(),logPacket.getFrameId());
            this.addOriginatingSipRequest(sipRequest);
        } else {
            SipResponse sipResponse = new SipResponse((SIPResponse) logPacket.getSipPacket(), logPacket.getTimeStamp(),logPacket.getFrameId());
            this.addOriginatingSipResponse(sipResponse);
        }
        
    }
    
    public void addReceivedPacket(CapturedLogPacket logPacket) {
        if ( logPacket.getSipPacket() instanceof SIPRequest ) {
            SipRequest sipRequest = new SipRequest( (SIPRequest) logPacket.getSipPacket(),logPacket.getTimeStamp(),logPacket.getFrameId());
            this.addReceivedSipRequest(sipRequest);
        } else {
            SipResponse sipResponse = new SipResponse((SIPResponse) logPacket.getSipPacket(), logPacket.getTimeStamp(),logPacket.getFrameId());
            this.addReceivedSipResponse(sipResponse);
        }
    }
    
    private void addOriginatingSipRequest(SipRequest sipRequest) {
        String transactionid = ((SIPRequest)sipRequest.getSipRequest()).getTransactionId();
        SipClientTransaction clientTx = SipTester.clientTransactionMap.get(transactionid);
        
        if (!SipTester.isExcluded(sipRequest.getSipRequest().getMethod())) {
            if (clientTx == null) {
                clientTx = new SipClientTransaction(sipRequest);
                SipTester.clientTransactionMap.put(transactionid, clientTx);
                this.clientTransactions.add(clientTx);
                clientTx.setEndpoint(this);
                logger.debug("created clientTransaction "
                        + transactionid);
            } else {
                clientTx.addRequest(sipRequest);
            }
        }
        
    }
    
    public void removeUnEmulatedClientTransactions(Collection<SipClientTransaction> emulatedSet) {
       Iterator<SipClientTransaction> it = this.clientTransactions.iterator();
       while(it.hasNext()) {
           SipClientTransaction ctx = it.next();
           if (! emulatedSet.contains(ctx)) {
               it.remove();
           }
       }
        
    }
    
  
    
    

    private void addReceivedSipRequest(SipRequest sipRequest) {
        String transactionid = ((SIPRequest)sipRequest.getSipRequest()).getTransactionId().toLowerCase();
        SipServerTransaction serverTx = SipTester.serverTransactionMap.get(transactionid);
        if ( serverTx == null ) {
            serverTx = new SipServerTransaction(sipRequest);
            SipTester.serverTransactionMap.put(transactionid, serverTx);
            this.serverTransactions.add(serverTx);
            serverTx.setEndpoint(this);
            logger.debug("created serverTransaction : " + transactionid);
        } else {
            serverTx.addRequest(sipRequest);
        }
    }
    
    private void addReceivedSipResponse(SipResponse sipResponse) {
        String transactionid = ((SIPResponse)sipResponse.getSipResponse()).getTransactionId();
        SipClientTransaction clientTx = SipTester.clientTransactionMap.get(transactionid);
        if ( clientTx == null ) {
            logger.debug("Cannot find clientTx for received response");
            return;
        }
        clientTx.addResponse(sipResponse);
    }

    public void addOriginatingSipResponse(SipResponse sipResponse) {
        String transactionid = ((SIPResponse)sipResponse.getSipResponse()).getTransactionId();
        SipServerTransaction serverTx = SipTester.getSipServerTransaction(transactionid);
        if ( serverTx == null  )  {
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
        
       this.listeningPoints.put(listeningPoint.getTransport().toLowerCase(),listeningPoint);
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
        this.sipProviders.put(transport.toUpperCase(),(SipProviderExt)sipProvider);
    }
    
    public SipProviderExt getProvider(String transport) {
        return this.sipProviders.get(transport.toUpperCase());       
    }
    
    



    public void runEmulatedCallFlow(long startTime) throws Exception {
        this.startTime = startTime;
        new Thread(new Runnable() {
            public void run() {
                
                if (clientTransactions.isEmpty() ) {
                    System.out.println(traceEndpoint.getIpAddress() + ":" + traceEndpoint.getPort() + " Nothing to run");
                    doneFlag = true;
                    return;
                }
                try {
                    long prevTime = EmulatedEndpoint.this.startTime;
                    Iterator<SipClientTransaction> it = clientTransactions.iterator();
                     
                    while (it.hasNext()) {
                        SipClientTransaction ctx = it.next();
                        long timeToSleep = ctx.getDelay() - prevTime;
                        //if ( timeToSleep > 0 )
                       // Thread.sleep(timeToSleep);
                        prevTime = ctx.getDelay();
                        System.out.println("aboutToEmulate " + 
                                ctx.getSipRequest().getSipRequest().getMethod() + " tid = " + ctx.getTransactionId());
                        if ( ctx.checkPreconditions() && ! ctx.processed  ) {
                            ctx.createAndSend();
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
    
    public Collection<SipServerTransaction> findSipServerTransaction(Request request ) {
        Collection<SipServerTransaction> retval = new HashSet<SipServerTransaction>();    
        Iterator<SipServerTransaction> it = this.serverTransactions.iterator();
        while(it.hasNext() ) {
            SipServerTransaction sst = it.next();
            logger.debug("sst.getBranch() " + sst.getBranch());
            if(request.getMethod().equals(sst.getSipRequest().getSipRequest().getMethod()) && 
                    sst.getBranch() != null &&
                    sst.getBranch().equalsIgnoreCase(SipUtilities.getBranchMatchId(request))) {
                it.remove();
                retval.add(sst);
            }
        }
        return retval;
    }
    
   

}
