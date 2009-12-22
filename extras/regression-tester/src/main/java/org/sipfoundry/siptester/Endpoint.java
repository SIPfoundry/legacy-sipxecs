package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipProviderExt;
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

public class Endpoint  {
    
    private static Logger logger = Logger.getLogger(Endpoint.class);
    
    private String ipAddress;
    
    private int port;
    
    private Hashtable<String,ListeningPoint>  listeningPoints = new Hashtable<String,ListeningPoint>(); 
    
    private Collection<SipClientTransaction> clientTransactions =  new ConcurrentSkipListSet<SipClientTransaction>();
    
    private Collection<SipServerTransaction> serverTransactions = new ConcurrentSkipListSet<SipServerTransaction>();
    
    private Map<String,SipClientTransaction> clientTransactionMap = new HashMap<String,SipClientTransaction>();
    
    private Map<String,SipServerTransaction> serverTransactionMap = new HashMap<String,SipServerTransaction>();

    private Map<String,SipProviderExt> sipProviders = new HashMap<String,SipProviderExt>();
     
    Hashtable<String,SipDialog> sipDialogs = new Hashtable<String,SipDialog>();
    
    boolean doneFlag;
    
    private SipStackBean sipStackBean;

    private SutUA sutUa;

    private long startTime;
    
    
    public Endpoint(String ipAddress, int port) {
        this.ipAddress = ipAddress;
        this.port = port;
    }
    
    
    
    public void setSutUA(SutUA sutUa) {
        this.sutUa = sutUa;
    }
    
   
    
    public SutUA getSutUA() {
        return this.sutUa;
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
    
    public void addOriginatingPacket(CapturedLogPacket logPacket) {
        if ( logPacket.getSipPacket() instanceof SIPRequest ) {
            SipRequest sipRequest = new SipRequest( (SIPRequest) logPacket.getSipPacket(),logPacket.getTimeStamp(),logPacket.getTraceFile());
            this.addOriginatingSipRequest(sipRequest);
        } else {
            SipResponse sipResponse = new SipResponse((SIPResponse) logPacket.getSipPacket(), logPacket.getTimeStamp(),logPacket.getTraceFile());
            this.addOriginatingSipResponse(sipResponse);
        }
        
    }
    
    public void addReceivedPacket(CapturedLogPacket logPacket) {
        if ( logPacket.getSipPacket() instanceof SIPRequest ) {
            SipRequest sipRequest = new SipRequest( (SIPRequest) logPacket.getSipPacket(),logPacket.getTimeStamp(),logPacket.getTraceFile());
            this.addReceivedSipRequest(sipRequest);
        } else {
            SipResponse sipResponse = new SipResponse((SIPResponse) logPacket.getSipPacket(), logPacket.getTimeStamp(),logPacket.getTraceFile());
            this.addReceivedSipResponse(sipResponse);
        }
    }
    
    private void addOriginatingSipRequest(SipRequest sipRequest) {
        String transactionid = ((SIPRequest)sipRequest.getSipRequest()).getTransactionId();
        SipClientTransaction clientTx = this.clientTransactionMap.get(transactionid);
        /*
         * We handle REGISTER requests independently. We do not emulate REGISTER requests.
         * Just keep track of who registered from where.
         */
        if ( sipRequest.getSipRequest().getMethod().equals(Request.REGISTER) ) {
            String fromUser = ((SipURI)sipRequest.getSipRequest().getFromHeader().getAddress().getURI()).getUser();
            logger.debug("REGISTER seen " + sipRequest.getSipRequest().getFirstLine() + " from " + fromUser);  
            this.getSutUA().addRegistration(fromUser);
            return;
        }
        if (!SipTester.isExcluded(sipRequest.getSipRequest().getMethod())) {
            if (clientTx == null) {
                clientTx = new SipClientTransaction(sipRequest);
                this.clientTransactionMap.put(transactionid, clientTx);
                this.clientTransactions.add(clientTx);
                clientTx.setEndpoint(this);
                logger.debug("created clientTransaction "
                        + sipRequest.getSipRequest().getFirstLine());
            } else {
                clientTx.addRequest(sipRequest);
            }
        }
        
    }

    private void addReceivedSipRequest(SipRequest sipRequest) {
        String transactionid = ((SIPRequest)sipRequest.getSipRequest()).getTransactionId();
        SipServerTransaction serverTx = this.serverTransactionMap.get(transactionid);
        if ( serverTx == null ) {
            serverTx = new SipServerTransaction(sipRequest);
            this.serverTransactionMap.put(transactionid, serverTx);
            this.serverTransactions.add(serverTx);
            serverTx.setEndpoint(this);
            logger.debug("created serverTransaction : " +sipRequest.getSipRequest().getFirstLine());
        } else {
            serverTx.addRequest(sipRequest);
        }
    }
    
    private void addReceivedSipResponse(SipResponse sipResponse) {
        String transactionid = ((SIPResponse)sipResponse.getSipResponse()).getTransactionId();
        SipClientTransaction clientTx = this.clientTransactionMap.get(transactionid);
        if ( clientTx == null ) {
            logger.debug("Cannot find clientTx for received response");
            return;
        }
        clientTx.addResponse(sipResponse);
    }

    public void addOriginatingSipResponse(SipResponse sipResponse) {
        String transactionid = ((SIPResponse)sipResponse.getSipResponse()).getTransactionId();
        SipServerTransaction serverTx = this.serverTransactionMap.get(transactionid);
        if ( serverTx == null  )  {
            logger.debug("Cannot find serverTx for transmitted response");
            return;
        }
        serverTx.addResponse(sipResponse);  
    }
    
    public SipServerTransaction getSipServerTransaction(String transactionId) {
        return this.serverTransactionMap.get(transactionId);
    }

    public SipClientTransaction getSipClientTransaction(String transactionId) {
         return this.clientTransactionMap.get(transactionId);
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
        String transport = this.sutUa.getDefaultTransport();
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
    
    public SipDialog getDialog(String dialogId) {
        System.out.println("dialogId " + dialogId);
        if ( dialogId == null ) return null;
        else {
             SipDialog sipDialog = this.sipDialogs.get(dialogId) ;
             if ( sipDialog != null ) {
                 return sipDialog;
             } else {
                 sipDialog = new SipDialog();
                 this.sipDialogs.put(dialogId, sipDialog);
                 return sipDialog;
             }
        }
    }



    public void runEmulatedCallFlow(long startTime) throws Exception {
        this.startTime = startTime;
        new Thread(new Runnable() {
            public void run() {
                
                if (clientTransactions.isEmpty() ) {
                    System.out.println("Nothing to run");
                    doneFlag = true;
                    return;
                }
                try {
                    long prevTime = Endpoint.this.startTime;
                    Iterator<SipClientTransaction> it = clientTransactions.iterator();
                     
                    while (it.hasNext()) {
                        SipClientTransaction ctx = it.next();
                        long timeToSleep = ctx.getDelay() - prevTime;
                        //if ( timeToSleep > 0 )
                       // Thread.sleep(timeToSleep);
                        prevTime = ctx.getDelay();
                        System.out.println("aboutToEmulate " + ctx.getSipRequest().getSipRequest().getFirstLine());
                        if ( ctx.checkPreconditions() ) {
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
    
    public SipServerTransaction findSipServerTransaction(Request request ) {
        logger.debug("find SipServerTransaction " + request.getMethod());
        this.printServerTransactions();
        
        Iterator<SipServerTransaction> it = this.serverTransactions.iterator();
        while(it.hasNext() ) {
            SipServerTransaction sst = it.next();
            if(request.getMethod().equals(sst.getSipRequest().getSipRequest().getMethod())) {
                it.remove();
                return sst;
            }
        }
        return null;
    }
    
    public void printServerTransactions() {
        if ( this.serverTransactionMap.isEmpty()) {
            logger.debug("empty server transaction map");
        }
        for ( SipServerTransaction stx : this.serverTransactionMap.values() ) {
            stx.printServerTransaction();
        }
    }

}
