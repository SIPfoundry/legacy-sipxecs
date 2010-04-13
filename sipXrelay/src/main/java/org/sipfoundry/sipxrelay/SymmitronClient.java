/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxrelay;

import java.net.URL;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Semaphore;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

/**
 * Wrapper for the client methods of the Symmitron.
 * 
 */
@SuppressWarnings("unchecked")
public class SymmitronClient {

	private static final Logger logger = Logger
			.getLogger("org.sipfoundry.sipxrelay");

	private String clientHandle;
	
	private SynchronizedXmlRpcClient client;

	private String serverHandle;

	private SymmitronResetHandler resetHandler;

	private String serverAddress;
	
	private String clientName;

	private static Timer timer = new Timer();

	private boolean checkForServerReboot(Map map) throws SymmitronException {

		String handle = (String) map.get(Symmitron.INSTANCE_HANDLE);

		if (!handle.equals(serverHandle)) {
			clientHandle = clientName + ":" + Math.abs(new Random().nextLong());
			this.signIn();
			return true;
		} else {
			return false;
		}

	}
	
	class SynchronizedXmlRpcClient {
	    private XmlRpcClient xmlRpcClient;
	    Semaphore semaphore;

        SynchronizedXmlRpcClient(XmlRpcClient xmlRpcClient) {
	        this.xmlRpcClient = xmlRpcClient;
	        this.semaphore = new Semaphore(1);
	    }
        
        public Object execute(String method, Object[] args) throws XmlRpcException {
            boolean acquired = false;
            try {
                semaphore.acquire();
                acquired = true;
                return this.xmlRpcClient.execute(method,args);
            } catch (XmlRpcException ex) {
                throw ex;
            } catch (InterruptedException ex) {
                throw new XmlRpcException("Interrupted!");
            } finally {
                if (acquired) {
                    semaphore.release();
                }
            }
            
        }
	}

	class ResetHandlerTimerTask extends TimerTask {

		@Override
		public void run() {

			Object[] args = new Object[1];
			args[0] = clientHandle;
			Map retval;
			String currentServerHandle = serverHandle;

			try {

				retval = (Map) client.execute("sipXrelay.ping", args);

			} catch (XmlRpcException e) {
				logger.error("XmlRpcException ", e);
				if (resetHandler != null) {
					resetHandler.reset(currentServerHandle);
				}
				return;
			}

			if (checkForServerReboot(retval)) {
				resetHandler.reset(currentServerHandle);
			}

		}

	}

	public String getServerHandle() {
		return this.serverHandle;
	}

	public SymmitronClient(String clientName, String serverAddress, int port,
			boolean isSipxRelaySecure, SymmitronResetHandler resetHandler)
			throws SymmitronException {
		try {
			XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
			String protocol = isSipxRelaySecure ? "https" : "http";
			try {

				logger.debug("Trying to sign in " + protocol + "://"
						+ serverAddress + ":" + port);

				config.setServerURL(new URL(protocol + "://" + serverAddress
						+ ":" + port));

				this.serverAddress = serverAddress;
				this.clientName = clientName;

				config.setEnabledForExceptions(true);
				config.setEnabledForExtensions(true);

			} catch (Exception e) {
				logger.error(e);
				throw new SymmitronException(e);
			}

			this.resetHandler = resetHandler;
			XmlRpcClient xmlRpcClient = new XmlRpcClient();

			xmlRpcClient.setConfig(config);
			xmlRpcClient.setMaxThreads(32);
			clientHandle = clientName + ":" + Math.abs(new Random().nextLong());
			client = new SynchronizedXmlRpcClient(xmlRpcClient);
			this.signIn();
			logger.debug("signedIn : " + protocol + "://" + serverAddress + ":"
					+ port);
			// ping the server continually.
			timer.schedule(new ResetHandlerTimerTask(), 0, 3000);
		} catch (SymmitronException ex) {
			logger.error("signing in failed", ex);
			throw ex;
		}

	}

	public SymImpl createEvenSym() throws SymmitronException {

		int count = 1;
		Object[] args = new Object[3];
		args[0] = clientHandle;
		args[1] = new Integer(count);
		args[2] = Symmitron.EVEN;

		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.createSyms", args);
		} catch (XmlRpcException e) {
			logger.error(e);

			throw new SymmitronException(e);
		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
		Object[] syms = (Object[]) retval.get(Symmitron.SYM_SESSION);
		Map sym = (Map) syms[0];
		String id = (String) sym.get("id");
		SymImpl symImpl = new SymImpl(id, this);

		Map receiverSession = (Map) sym.get("receiver");
		if (receiverSession != null && !receiverSession.isEmpty()) {
			String ipAddr = (String) receiverSession.get("ipAddress");
			int port = (Integer) receiverSession.get("port");
			String tid = (String) receiverSession.get("id");

			SymEndpointImpl receiverEndpoint = new SymEndpointImpl(this);
			receiverEndpoint.setIpAddress(ipAddr);
			receiverEndpoint.setPort(port);
			receiverEndpoint.setId(tid);
			symImpl.setReceiver(receiverEndpoint);
		}

		return symImpl;

	}

	public String createNewBridge() throws SymmitronException {
		try {
			Object[] args = new Object[1];
			args[0] = this.clientHandle;
			Map retval;
			try {
				retval = (Map) client.execute("sipXrelay.createBridge", args);
			} catch (XmlRpcException e) {
				logger.error("XmlRpcException ", e);

				throw new SymmitronException(e);
			}

			if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
				throw new SymmitronException("Error in processing request "
						+ retval.get(Symmitron.ERROR_INFO));
			}

			return (String) retval.get(Symmitron.BRIDGE_ID);
		} catch (SymmitronException ex) {
			logger.error(ex);
			if (resetHandler != null) {
				resetHandler.reset(serverHandle);
			}
			throw ex;
		}
	}

	public String getPublicAddress() throws SymmitronException {
		Object[] args = new Object[1];
		args[0] = this.clientHandle;
		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.getPublicAddress", args);
		} catch (XmlRpcException e) {
			logger.error("XmlRpcException ", e);
			throw new SymmitronException("ERROR Contacting SIPXRELAY");
		}

		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException((String)retval.get(Symmitron.ERROR_INFO));
		}
		
		String pubaddr  = (String) retval.get(Symmitron.PUBLIC_ADDRESS);
		if ( pubaddr == null ) {
			throw new SymmitronException("SIPXRELAY Public address not found");
		} else return pubaddr;
	}

	public void destroyBridge(String bridgeId) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = this.clientHandle;
		args[1] = bridgeId;
		Map retval;
		logger.debug(String.format("destroyBridge %s" , bridgeId));
		try {
			retval = (Map) client.execute("sipXrelay.destroyBridge", args);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}

		// Could not find the bridge -- silently log the error and dont
		// scream about it.
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			logger.error("destroyBridge " + bridgeId);
			logger.error("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
	}

	public void setRemoteEndpoint(SymImpl sym, String ipAddress,
			int destinationPort, int keepAliveInterval,
			KeepaliveMethod keepAliveMethod) throws SymmitronException {
		try {
			Object[] params = new Object[6];
			params[0] = clientHandle;
			params[1] = sym.getId();
			params[2] = ipAddress;
			params[3] = new Integer(destinationPort);
			params[4] = new Integer(keepAliveInterval);
			params[5] = keepAliveMethod.toString();

			logger.debug(String.format("setRemoteEndpoint " +
					" sym = %s " +
					" ipAddress = %s " +
					" destinationPort = %s " +
					" keepAliveInterval %s " +
					" keepAliveMethod = %s",
			        params[1].toString(),params[2].toString(),params[3].toString(),
			        params[4].toString(),params[5].toString()));
			Map retval = (Map) client.execute("sipXrelay.setDestination",
					params);
			
			if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
				throw new SymmitronException("Error in processing request "
						+ retval.get(Symmitron.ERROR_INFO));
			}

		} catch (XmlRpcException ex) {
			throw new SymmitronException("RPC error in executing method", ex);
		}

	}

	public void addSym(String bridge, SymInterface sym)
			throws SymmitronException {
		Object[] params = new Object[3];
		if (sym == null || sym.getId() == null) {
			throw new SymmitronException(new NullPointerException("null sym"));
		}
		if (bridge == null) {
			throw new SymmitronException(
					new NullPointerException("null bridge"));
		}
		params[0] = clientHandle;
		params[1] = bridge;
		params[2] = sym.getId();
		Map retval;
		logger.debug(String.format("addSym : %s %s ", bridge, sym.getId()));
		try {
			retval = (Map) client.execute("sipXrelay.addSym", params);
		} catch (XmlRpcException e) {
			logger.error("XmlRpcException ", e);
			throw new SymmitronException(e);

		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
	}

	public void removeSym(String bridge, SymInterface sym)
			throws SymmitronException {
		Object[] parms = new Object[3];
		parms[0] = clientHandle;
		parms[1] = bridge;
		parms[2] = sym.getId();
		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.removeSym", parms);
		} catch (XmlRpcException e) {
			logger.error("XmlRpcException ", e);
			throw new SymmitronException(e);

		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}

	}

	/**
	 * Get a Sym.
	 */
	public SymInterface getSym(String symId) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = symId;
		Map retval = null;
		try {
			retval = (Map) client.execute("sipXrelay.getSym", args);
		} catch (XmlRpcException ex) {
			throw new SymmitronException("Error in processing request ", ex);
		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
		SymImpl symImpl = new SymImpl(symId, this);

		Map symSession = (Map) retval.get(Symmitron.SYM_SESSION);
		Map receiverSession = (Map) symSession.get("receiver");
		if (receiverSession != null && !receiverSession.isEmpty()) {
			String ipAddr = (String) receiverSession.get("ipAddress");
			int port = (Integer) receiverSession.get("port");
			String id = (String) receiverSession.get("id");

			SymEndpointImpl receiverEndpoint = new SymEndpointImpl(this);
			receiverEndpoint.setIpAddress(ipAddr);
			receiverEndpoint.setPort(port);
			receiverEndpoint.setId(id);
			symImpl.setReceiver(receiverEndpoint);
		}

		Map transmitterSession = (Map) symSession.get("transmitter");
		if (transmitterSession != null && !transmitterSession.isEmpty()) {
			String ipAddr = (String) transmitterSession.get("ipAddress");

			int port = (Integer) transmitterSession.get("port");
			String id = (String) transmitterSession.get("id");

			SymEndpointImpl transmitterEndpoint = new SymEndpointImpl(this);
			transmitterEndpoint.setIpAddress(ipAddr);
			transmitterEndpoint.setPort(port);
			transmitterEndpoint.setId(id);
			symImpl.setTransmitter(transmitterEndpoint);
		}
		return symImpl;
	}

	public void pauseBridge(String bridge) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = bridge;
		Map retval;
		
		logger.debug("pauseBridge " + bridge);
		
		try {
			retval = (Map) client.execute("sipXrelay.pauseBridge", args);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
	}

	public void setOnHold(String symId, boolean holdFlag)
			throws SymmitronException {

		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = symId;
		
		logger.debug("setOnHold " + symId  + " holdFlag " + holdFlag);
		
		if (holdFlag) {
			Map retval;
			try {
				retval = (Map) client.execute("sipXrelay.pauseSym", args);
			} catch (XmlRpcException e) {
				logger.error(e);
				throw new SymmitronException(e);
			}
			if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
				throw new SymmitronException("Error in processing request "
						+ retval.get(Symmitron.ERROR_INFO));
			}
		} else {
			Map retval;
			try {
				retval = (Map) client.execute("sipXrelay.resumeSym", args);
			} catch (XmlRpcException e) {
				logger.error(e);
				throw new SymmitronException(e);
			}
			if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
				throw new SymmitronException("Error in processing request "
						+ retval.get(Symmitron.ERROR_INFO));
			}
		}
	}

	public String getReceiverState(String symId) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = symId;
		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.getReceiverState", args);
			if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
				throw new SymmitronException("Error in processing request "
						+ retval.get(Symmitron.ERROR_INFO));
			}
			return (String) retval.get(Symmitron.RECEIVER_STATE);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}

	}

	public void resumeBridge(String bridge) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = bridge;
		
		logger.debug("resumeBridge " + bridge);
		
		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.resumeBridge", args);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}
		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}
	}

	public void startBridge(String bridge) throws SymmitronException {
		Object[] args = new Object[2];
		args[0] = clientHandle;
		args[1] = bridge;
		
		logger.debug("startBridge " + bridge);
		
		Map retval;
		try {
			retval = (Map) client.execute("sipXrelay.startBridge", args);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}

		if (retval.get(Symmitron.STATUS_CODE).equals(Symmitron.ERROR)) {
			throw new SymmitronException("Error in processing request "
					+ retval.get(Symmitron.ERROR_INFO));
		}

	}
	public int ping() {
		Object[] args = new Object[1];
		args[0] = clientHandle;
		Map retval;
		
		try {
			retval = (Map) client.execute("sipXrelay.ping", args);
			if ( (String)retval.get(Symmitron.NBRIDGES) != null ) {
			    return Integer.parseInt((String)retval.get(Symmitron.NBRIDGES));
			} else {
			    return 0;
			}
		} catch (XmlRpcException e) {
			logger.error("XmlRpcException ", e);
			throw new SymmitronException("Error pinging " + this.serverAddress);
		}
		
	}
	
	public boolean pingAndTest(String host, int port) {
	    Object[] args = new Object[3];
        args[0] = clientHandle;
        args[1] = host;
        args[2] = new Integer(port);
        Map retval;
        
        logger.debug("pingAndTest " + host + ":" + port);
        
        
        try {
            retval = (Map) client.execute("sipXrelay.pingAndTest", args);
            if ( (String)retval.get(Symmitron.PROXY_LIVENESS) != null ) {
                return Boolean.parseBoolean((String)retval.get(Symmitron.PROXY_LIVENESS));
            } else {
                return false;
            }
        } catch (XmlRpcException e) {
            logger.error("XmlRpcException ", e);
            throw new SymmitronException("Error pinging " + this.serverAddress);
        }
	}
	
	
	public void signIn() throws SymmitronException {

		String[] myHandle = new String[1];
		myHandle[0] = clientHandle;
		logger.debug("signIn " + clientHandle);

		try {
			Map retval = (Map) client.execute("sipXrelay.signIn",
					(Object[]) myHandle);
			this.serverHandle = (String) retval.get(Symmitron.INSTANCE_HANDLE);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);

		}

	}

	public void signOut() throws SymmitronException {
		String[] myHandle = new String[1];
		myHandle[0] = clientHandle;
		logger.debug("signOut " + clientHandle);
		try {
			client.execute("sipXrelay.signOut", (Object[]) myHandle);
		} catch (XmlRpcException e) {
			logger.error(e);
			throw new SymmitronException(e);
		}

	}

	public void destroySym(SymImpl sym) throws SymmitronException {
	    logger.debug("destroySym " + sym.getId());
		try {
			Object[] args = new Object[2];
			args[0] = clientHandle;
			args[1] = sym.getId();
			client.execute("sipXrelay.destroySym", args);
		} catch (XmlRpcException ex) {
			logger.error(ex);
			throw new SymmitronException(ex);
		}

	}

	public BridgeInterface createBridge() throws SymmitronException {
		return new BridgeImpl(this);
	}

	public SymTransmitterEndpointImpl createSymTransmitter(SymImpl symImpl) {
		return new SymTransmitterEndpointImpl(this, symImpl);

	}

	public String getServerAddress() {
		return serverAddress;
	}

	

}
