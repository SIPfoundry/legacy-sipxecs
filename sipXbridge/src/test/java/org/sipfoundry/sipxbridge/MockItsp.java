/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.SipStackImpl;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Properties;

import javax.sdp.MediaDescription;
import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.ListeningPoint;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.ServerTransaction;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionState;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.ContactHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.RecordRouteHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.cafesip.sipunit.SipCall;
import org.cafesip.sipunit.SipPhone;
import org.cafesip.sipunit.SipRequest;
import org.cafesip.sipunit.SipTestCase;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.log4j.SipFoundryLogRecordFactory;

/**
 * Test ITSP. This is endpoint that mocks ITSP behavior.
 *
 * @author M. Ranganathan
 *
 */
public class MockItsp implements SipListener {

	private org.cafesip.sipunit.SipStack phoneStack;
	private HashMap<String, SipPhone> sipPhones = new HashMap<String, SipPhone>();
	private HashSet<PhoneResponder> phoneResponders = new HashSet<PhoneResponder>();

	private static SipStack sipStack;
	private SipProvider provider;
	private ListeningPoint listeningPoint;

	private static AccountManagerImpl accountManager;
	private BridgeConfiguration bridgeConfig;

	private String myIpAddress;
	private int myPort;

	// private Dialog dialog;
	boolean inviteSeen = false;

	int bytesRead;
	int readCount;
	int bytesToSend;
	private int writeCount;

	private SessionDescription remoteSessionDescripion;
	private AbstractSipSignalingTest abstractSipSignalingTest;

	private ItspAccountInfo accountInfo;

	private static Logger logger = Logger.getLogger(MockItsp.class);

	private static String sdpBodyFormat = "v=0\r\n"
			+ "o=alice 2890844526 2890844526 IN IP4 %s\r\n" + "s=\r\n"
			+ "c=IN IP4 %s\r\n" + "t=0 0\r\n" + "m=audio %d RTP/AVP 0 8 97\r\n"
			+ "a=rtpmap:0 PCMU/8000\r\n";

	static {
		try {
			Properties stackProperties = new Properties();
			stackProperties.setProperty("javax.sip.STACK_NAME", "MockITSP");
			stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL",
					"DEBUG");
			stackProperties.setProperty("gov.nist.javax.sip.DEBUG_LOG",
					"logs/itsp-debuglog.txt");
			stackProperties.setProperty(
					"gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");
			stackProperties.setProperty("javax.sip.AUTOMATIC_DIALOG_SUPPORT",
					"off");
			stackProperties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
					SipFoundryLogRecordFactory.class.getName());

			sipStack = ProtocolObjects.sipFactory
					.createSipStack(stackProperties);

			accountManager = Gateway.getAccountManager();
			SipFoundryAppender sfa = new SipFoundryAppender();
			sfa.setFile(Gateway.getLogFile());
			sfa.setLayout(new SipFoundryLayout());
			((SipStackImpl) sipStack).addLogAppender(sfa);
			logger.addAppender(sfa);

		} catch (Exception ex) {
			throw new RuntimeException("Error loading factories", ex);
		}

	}

	class SocketReader implements Runnable {

		private DatagramSocket socketToRead;

		public SocketReader(DatagramSocket socketToRead) {
			this.socketToRead = socketToRead;
		}

		public void run() {
			byte[] buffer = new byte[8192];
			DatagramPacket datagramPacket = new DatagramPacket(buffer,
					buffer.length);
			while (true) {
				try {
					socketToRead.receive(datagramPacket);
					bytesRead = bytesRead + datagramPacket.getLength();
					readCount++;
				} catch (IOException e) {
					logger
							.debug("SocketReader: Exiting read thread -- encountered an IOException");
					break;
				}
			}

		}

	}

	class SocketWriter implements Runnable {
		private DatagramSocket socketToWrite;

		public SocketWriter(DatagramSocket socketToWrite) {
			this.socketToWrite = socketToWrite;

		}

		public void run() {

			byte[] buffer = new byte[8192];
			DatagramPacket datagramPacket = new DatagramPacket(buffer,
					buffer.length);

			for (int i = 0; i < writeCount; i++) {
				try {
					socketToWrite.send(datagramPacket);
				} catch (IOException e) {
					e.printStackTrace();
					TestCase.fail("Unexpected exception sending");
				}
			}

		}
	}

	class PhoneResponder implements Runnable {
		SipPhone sipPhone;
		SocketReader socketReader;
		SessionDescription remoteSessionDescription;
		int callTime;
		boolean callerSendsBye;

		public PhoneResponder(SipPhone sipPhone, int callTime,
				boolean callerSendsBye) {
			this.sipPhone = sipPhone;
			this.callTime = callTime;
			this.callerSendsBye = callerSendsBye;
		}

		public void run() {
			try {
				sipPhone.listenRequestMessage();
				SipCall sipCall = sipPhone.createSipCall();

				sipCall.waitForIncomingCall(3000);

				SipRequest sipRequest = sipCall.getLastReceivedRequest();
				Request request = (Request) sipRequest.getMessage();
				remoteSessionDescription = SipUtilities
						.getSessionDescription(request);
				SessionDescription sd = createSessionDescription();

				sipCall.sendIncomingCallResponse(Response.OK, "OK", 3000, sd
						.toString(), "application", "sdp", null, null);
				sipCall.listenForAck();
				sipCall.waitForAck(3000);

				if (callerSendsBye) {
					sipCall.listenForDisconnect();

					sipCall.waitForDisconnect(3000);
					sipCall.respondToDisconnect();
				} else {
					Thread.sleep(callTime);
					sipCall.disconnect();
					sipCall.waitForAnswer(1000);
				}

			} catch (Exception ex) {
				ex.printStackTrace();
				AbstractSipSignalingTest.fail("Unexpected exception ");
			}

		}

	}

	class PhoneCaller {
		SipPhone phone;
		String inboundNumber;

		public PhoneCaller(String inboundNumber) throws Exception {
			String userName = accountInfo.getUserName();
			String domain = accountInfo.getProxyDomain();
			phone = phoneStack.createSipPhone(
					abstractSipSignalingTest.sipxProxyAddress, "udp", myPort,
					String.format("sip:%s@%s", userName, domain));

			this.inboundNumber = inboundNumber;

		}

		public void makeCall() {
			String user = inboundNumber;
			String to = "sip:" + user + "@" + bridgeConfig.getSipxProxyDomain();
			String sdpBody = String.format(sdpBodyFormat, myIpAddress,
					myIpAddress, abstractSipSignalingTest.getMediaPort());
			SipCall sipCall = phone.makeCall(to, Response.OK, 2000, null,
					sdpBody, "application", "sdp", null, null);

			abstractSipSignalingTest.assertLastOperationSuccess("Expect an OK"
					+ sipCall.format(), sipCall);
			boolean res = sipCall.sendInviteOkAck();
			SipTestCase.assertTrue("Successful ack sent "
					+ sipCall.getErrorMessage(), res);
			SipTestCase.assertNotNull("Null sipCall ", sipCall);

			SipTestCase.assertNotNull("Null sipCall ", sipCall);
			abstractSipSignalingTest.assertLastOperationSuccess("Expect an OK"
					+ sipCall.format(), sipCall);
			sipCall.disconnect();
			SipTestCase.assertTrue("Successful bye sent "
					+ sipCall.getErrorMessage(), res);
		}
	}

	public void makePhoneInboundCall() throws Exception {
		String userName = this.accountInfo.getUserName();

		PhoneCaller phoneCaller = new PhoneCaller(userName);
		phoneCaller.makeCall();
	}

	public void createPhones(int responderCount, int callerCount,
			int timeOfCall, boolean callerSendsBye) throws Exception {
		/*
		 * Set up phones.
		 */
		for (int i = 0; i < responderCount; i++) {
			String user = "111222333" + i;
			String phoneId = String.format("sip:%s@itsp.example.com", user);
			SipPhone sipPhone = this.phoneStack.createSipPhone(
					this.myIpAddress, "udp", this.myPort, phoneId);
			this.sipPhones.put(user, sipPhone);
			PhoneResponder phoneResponder = new PhoneResponder(sipPhone,
					timeOfCall, callerSendsBye);
			new Thread(phoneResponder).start();
			this.phoneResponders.add(phoneResponder);
		}
	}

	public MockItsp(AbstractSipSignalingTest abstractSipSignalingTest)
			throws Exception {
		System.out.println("Creating MockITSP");
		this.abstractSipSignalingTest = abstractSipSignalingTest;
		this.accountInfo = abstractSipSignalingTest.accountInfo;
		this.bridgeConfig = abstractSipSignalingTest.accountManager
				.getBridgeConfiguration();
		this.myPort = this.accountInfo.getOutboundProxyPort();
		Properties properties = new Properties();
		properties.setProperty("javax.sip.STACK_NAME", "itsp-emulator");
		properties.setProperty("javax.sip.IP_ADDRESS", "127.0.0.1");

		this.phoneStack = new org.cafesip.sipunit.SipStack("udp",
				this.myPort + 1, properties);

	}

	private SessionDescription createSessionDescription() {
		try {
			int mediaPort = this.abstractSipSignalingTest.getMediaPort();

			String sdpBody = String.format(sdpBodyFormat, myIpAddress,
					myIpAddress, mediaPort);
			SessionDescription sd = SdpFactory.getInstance()
					.createSessionDescription(sdpBody);

			return sd;

		} catch (Exception ex) {
			TestCase.fail("Cannot create session description");
			return null;
		}

	}

	public void processDialogTerminated(DialogTerminatedEvent dte) {
		// TODO Auto-generated method stub

	}

	public void processIOException(IOExceptionEvent ioex) {
		AbstractSipSignalingTest.fail("Unexpected exception ");

	}

	public void processRequest(RequestEvent requestEvent) {
		System.out.println("ProcessRequest "
				+ requestEvent.getRequest().getMethod());
		try {
			Request request = requestEvent.getRequest();
			ServerTransaction st = requestEvent.getServerTransaction();
			if (st == null && !request.getMethod().equals(Request.ACK)) {
				try {
					st = provider.getNewServerTransaction(request);
				} catch (TransactionAlreadyExistsException ex) {
					return;
				}
			}
			String user = ((SipURI) request.getRequestURI()).getUser();
			if (request.getMethod().equals(Request.REGISTER)) {
				Response response = SipFactories.messageFactory.createResponse(
						Response.OK, request);
				ContactHeader contact = (ContactHeader) request
						.getHeader(ContactHeader.NAME);
				contact.setExpires(3600);
				response.setHeader(contact);

				st.sendResponse(response);

			} else if (request.getMethod().equals(Request.INVITE)) {

				Request newRequest = (Request) request.clone();
				ViaHeader inboundViaHeader = (ViaHeader) newRequest
						.getHeader(ViaHeader.NAME);
				int viaPort = inboundViaHeader.getPort();
				System.out.println("viaPort = " + viaPort);
				RouteHeader rh = null;
				if (viaPort != this.myPort + 1) {

					SipPhone sipPhone = this.sipPhones.get(user);
					if (sipPhone == null) {
						Response response = SipFactories.messageFactory
								.createResponse(Response.NOT_FOUND, request);
						response.setReasonPhrase("Cannot find phone " + user);
						st.sendResponse(response);
						return;

					}

					SipURI phoneUri = SipFactories.addressFactory.createSipURI(
							((SipURI) request.getRequestURI()).getUser(),
							this.myIpAddress);
					phoneUri.setPort(this.myPort + 1);
					newRequest.setRequestURI(phoneUri);
					SipURI routeUri = SipFactories.addressFactory.createSipURI(
							null, this.myIpAddress);
					routeUri.setPort(this.myPort + 1);
					routeUri.setLrParam();
					Address routeAddress = SipFactories.addressFactory
							.createAddress(routeUri);
					rh = SipFactories.headerFactory
							.createRouteHeader(routeAddress);
				} else {
					SipURI phoneUri = SipFactories.addressFactory.createSipURI(
							((SipURI) request.getRequestURI()).getUser(),
							this.myIpAddress);
					phoneUri.setPort(accountManager.getBridgeConfiguration()
							.getExternalPort());
					newRequest.setRequestURI(phoneUri);
					SipURI routeUri = SipFactories.addressFactory.createSipURI(
							null, this.myIpAddress);
					routeUri.setPort(accountManager.getBridgeConfiguration()
							.getExternalPort());
					routeUri.setLrParam();
					Address routeAddress = SipFactories.addressFactory
							.createAddress(routeUri);
					rh = SipFactories.headerFactory
							.createRouteHeader(routeAddress);
				}
				ViaHeader viaHeader = SipFactories.headerFactory
						.createViaHeader(this.myIpAddress, this.myPort, "udp",
								null);
				newRequest.addFirst(viaHeader);

				SipURI recordRouteURI = SipFactories.addressFactory
						.createSipURI(null, this.myIpAddress);
				recordRouteURI.setPort(this.myPort);

				Address address = SipFactories.addressFactory
						.createAddress(recordRouteURI);
				RecordRouteHeader rrh = SipFactories.headerFactory
						.createRecordRouteHeader(address);
				newRequest.addLast(rrh);

				ContactHeader contactHeader = ((ListeningPointExt) this.listeningPoint)
						.createContactHeader();
				newRequest.setHeader(contactHeader);

				newRequest.setHeader(rh);

				System.out.println("newRequest -- " + newRequest);
				ClientTransaction ctx = provider
						.getNewClientTransaction(newRequest);
				ctx.setApplicationData(st);
				st.setApplicationData(ctx);
				ctx.sendRequest();

			} else if (request.getMethod().equals(Request.ACK)) {
				ViaHeader viaHeader = SipFactories.headerFactory
						.createViaHeader(this.myIpAddress, this.myPort, "udp",
								null);
				request.addFirst(viaHeader);
				this.provider.sendRequest(request);
			} else if (request.getMethod().equals(Request.CANCEL)) {
				ServerTransaction stx = requestEvent.getServerTransaction();
				Response cancelResponse = SipFactories.messageFactory
						.createResponse(Response.OK, stx.getRequest());
				stx.sendResponse(cancelResponse);
				ServerTransaction canceledInviteStx = ((ServerTransactionExt) stx)
						.getCanceledInviteTransaction();

				ClientTransaction ctx = (ClientTransaction) canceledInviteStx
						.getApplicationData();
				if (ctx.getState() == TransactionState.PROCEEDING) {
					Request cancel = ctx.createCancel();
					ClientTransaction cancelCtx = provider
							.getNewClientTransaction(cancel);
					ctx.sendRequest();
				}

			} else if (request.getMethod().equals(Request.BYE)) {

				Request newRequest = (Request) request.clone();
				ViaHeader inboundViaHeader = (ViaHeader) newRequest
						.getHeader(ViaHeader.NAME);

				int viaPort = inboundViaHeader.getPort();
				System.out.println("viaPort = " + viaPort);
				RouteHeader rh = null;

				if (viaPort != this.myPort + 1) {
					SipURI routeUri = SipFactories.addressFactory.createSipURI(
							null, this.myIpAddress);
					routeUri.setPort(this.myPort + 1);
					routeUri.setLrParam();
					Address routeAddress = SipFactories.addressFactory
							.createAddress(routeUri);
					rh = SipFactories.headerFactory
							.createRouteHeader(routeAddress);
					newRequest.setHeader(rh);

					ViaHeader viaHeader = SipFactories.headerFactory
							.createViaHeader(this.myIpAddress, this.myPort,
									"udp", null);
					newRequest.addFirst(viaHeader);
					ClientTransaction newTx = provider
							.getNewClientTransaction(newRequest);
					st.setApplicationData(newTx);
					newTx.setApplicationData(st);
					newTx.sendRequest();
				} else {
					SipURI routeUri = SipFactories.addressFactory.createSipURI(
							null, this.myIpAddress);
					routeUri.setPort(accountManager.getBridgeConfiguration()
							.getExternalPort());
					routeUri.setLrParam();
					Address routeAddress = SipFactories.addressFactory
							.createAddress(routeUri);
					rh = SipFactories.headerFactory
							.createRouteHeader(routeAddress);
					newRequest.setHeader(rh);

					ViaHeader viaHeader = SipFactories.headerFactory
							.createViaHeader(this.myIpAddress, this.myPort,
									"udp", null);
					newRequest.addFirst(viaHeader);
					ClientTransaction newTx = provider
							.getNewClientTransaction(newRequest);
					st.setApplicationData(newTx);
					newTx.setApplicationData(st);
					newTx.sendRequest();
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			System.out.println("Error while Processing request "
					+ requestEvent.getServerTransaction().getRequest());
			AbstractSipSignalingTest.fail("Unexpected exception");
		}
	}

	public void processResponse(ResponseEvent responseEvent) {
		Response response = responseEvent.getResponse();

		try {

			ClientTransaction ctx = responseEvent.getClientTransaction();
			if (ctx == null) {
				logger
						.debug("MockItsp: Null client transaction  dropping response");
				return;
			}
			ServerTransaction stx = (ServerTransaction) ctx
					.getApplicationData();
			Response newResponse = (Response) response.clone();
			newResponse.removeFirst(ViaHeader.NAME);
			stx.sendResponse(newResponse);
		} catch (Exception ex) {
			logger.error("Exception occured processing " + response);
			ex.printStackTrace();
			TestCase.fail("Exception processing response");
		}

	}

	public void processTimeout(TimeoutEvent arg0) {
		// TODO Auto-generated method stub

	}

	public void processTransactionTerminated(TransactionTerminatedEvent arg0) {
		// TODO Auto-generated method stub

	}

	public void init(int bytesToSend) throws Exception {

		myIpAddress = accountInfo.getOutboundProxy();
		myPort = accountInfo.getOutboundProxyPort();

		System.out
				.println("myIpAddress = " + myIpAddress + " myPort " + myPort);
		listeningPoint = sipStack.createListeningPoint(myIpAddress, myPort,
				"udp");
		provider = sipStack.createSipProvider(listeningPoint);

		provider.addSipListener(this);
		this.writeCount = bytesToSend;

		this.bytesToSend = bytesToSend;

	}

	public void stop() {
		this.sipStack.stop();
		this.phoneStack.dispose();
		for (SipPhone sipPhone : this.sipPhones.values()) {
			sipPhone.dispose();
		}

	}

}
