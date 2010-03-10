package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.address.SipURIExt;
import gov.nist.javax.sip.clientauthutils.DigestServerAuthenticationHelper;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferencesHeader;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.MessageFactoryExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;
import gov.nist.javax.sip.message.SIPMessage;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetAddress;
import java.net.URLDecoder;
import java.text.ParseException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Random;
import java.util.Set;
import java.util.Vector;

import javax.sdp.Attribute;
import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SdpException;
import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.InvalidArgumentException;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ProxyAuthenticateHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.RecordRouteHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.header.WWWAuthenticateHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class SipUtilities {
	private static final Logger logger = Logger.getLogger(SipUtilities.class);

	private static final String APPLICATION = "application";

	private static final String SDP = "sdp";

	private int m_maxForwards = 70;

	private static HeaderFactoryExt headerFactory = (HeaderFactoryExt) SipTester
			.getStackBean().getHeaderFactory();

	private static MessageFactoryExt messageFactory = (MessageFactoryExt) SipTester
			.getStackBean().getMessageFactory();

	private static AddressFactory addressFactory = (AddressFactory) SipTester
			.getStackBean().getAddressFactory();

	public final FromHeader createFromHeader(String fromDisplayName,
			SipURI fromAddress) throws ParseException {
		Address fromNameAddress = addressFactory.createAddress(fromDisplayName,
				fromAddress);
		return headerFactory.createFromHeader(fromNameAddress, Integer
				.toString(Math.abs(new Random().nextInt())));
	}

	final public static ContactHeader createContactHeader(
			ListeningPointExt listeningPoint) throws ParseException {
		ContactHeader contactHeader = listeningPoint.createContactHeader();
		Address address = contactHeader.getAddress();
		SipURI sipUri = (SipURI) address.getURI();

		sipUri.setUser("sipxtester");

		return contactHeader;
	}

	static final public long getSequenceNumber(Message sipMessage) {
		CSeqHeader cseqHeader = (CSeqHeader) sipMessage
				.getHeader(CSeqHeader.NAME);
		return cseqHeader.getSeqNumber();
	}

	final public ToHeader createToHeader(SipURI toURI) throws ParseException {
		Address toAddress = SipTester.getStackBean().getAddressFactory()
				.createAddress(toURI);
		return headerFactory.createToHeader(toAddress, null);
	}

	final public AcceptHeader createAcceptHeader(String type, String subType)
			throws ParseException {
		return headerFactory.createAcceptHeader(type, subType);
	}

	final public static ViaHeader createViaHeader(
			ListeningPointExt listeningPoint) throws ParseException,
			InvalidArgumentException {
		return listeningPoint.createViaHeader();
	}

	public final ReasonHeader createReasonHeader(String reason)
			throws ParseException, InvalidArgumentException {
		ReasonHeader reasonHeader = headerFactory.createReasonHeader(
				"sipxtester", 1024, reason);
		return reasonHeader;
	}

	final public void addContent(Request request, String contentType,
			byte[] payload) throws ParseException {
		if (contentType == null) {
			return;
		}
		String[] ct = contentType.split("/", 2);
		ContentTypeHeader contentTypeHeader = headerFactory
				.createContentTypeHeader(ct[0], ct[1]);
		if (contentTypeHeader != null) {
			request.setContent(payload, contentTypeHeader);
		}
	}

	final public void addEventHeader(Request request, String eventType)
			throws ParseException {
		EventHeader eventHeader = headerFactory.createEventHeader(eventType);
		request.addHeader(eventHeader);
	}

	final public void addHeader(Request request, String name, String value)
			throws ParseException {
		Header header = headerFactory.createHeader(name, value);
		request.addHeader(header);
	}

	final public void setContent(Message message,
			SessionDescription sessionDescription) {
		try {
			ContentTypeHeader cth = headerFactory.createContentTypeHeader(
					APPLICATION, SDP);
			String sd = sessionDescription.toString();
			message.setContent(sd, cth);
		} catch (ParseException ex) {
			throw new SipTesterException(ex);
		}

	}

	final public Response createResponse(Request request, int responseCode)
			throws ParseException {
		Response response = messageFactory
				.createResponse(responseCode, request);
		return response;
	}

	final public static String getCSeqMethod(Message response) {
		return ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod();
	}

	final public void setSdpContent(Message message, String sdpContent) {
		try {
			ContentTypeHeader cth = this.createContentTypeHeader();
			message.setContent(sdpContent, cth);
		} catch (Exception ex) {
			logger.error("Unexpected exception creating header", ex);
			throw new SipTesterException(ex);
		}
	}

	final public ReferToHeader createReferToHeader(String referToAddrSpec) {
		try {
			String referToUri = "sip:" + referToAddrSpec;
			SipURI sipUri = (SipURI) addressFactory.createURI(referToUri);
			Address address = addressFactory.createAddress(sipUri);
			ReferToHeader referToHeader = headerFactory
					.createReferToHeader(address);
			return referToHeader;
		} catch (ParseException ex) {
			logger.error("Unexpected exception creating header", ex);
			throw new SipTesterException(ex);
		}
	}

	final public ContentTypeHeader createContentTypeHeader() {
		try {
			return headerFactory.createContentTypeHeader(APPLICATION, SDP);
		} catch (Exception ex) {
			logger.error("Unexpected exception creating header", ex);
			throw new SipTesterException(ex);
		}
	}

	final public ReferredByHeader createReferredByHeader(String addrSpec)
			throws ParseException {
		String referredByUri = "sip:" + addrSpec;
		Address address = addressFactory.createAddress(addressFactory
				.createURI(referredByUri));
		return ((HeaderFactoryImpl) headerFactory)
				.createReferredByHeader(address);
	}

	final public String getSipDomain(String uri) throws Exception {
		SipURI sipUri = (SipURI) addressFactory.createURI(uri);
		return sipUri.getHost();
	}

	public static String getToTag(Message message) {
		return ((ToHeader) message.getHeader(ToHeader.NAME)).getTag();
	}

	public static String getToAddress(Message message) {
		return ((ToHeader) message.getHeader(ToHeader.NAME)).getAddress()
				.toString();
	}

	public static String getToAddrSpec(Message message) {
		String user = ((SipURI) ((ToHeader) message.getHeader(ToHeader.NAME))
				.getAddress().getURI()).getUser();
		String host = ((SipURI) ((ToHeader) message.getHeader(ToHeader.NAME))
				.getAddress().getURI()).getHost();
		return user + "@" + host;
	}

	public static String getCallId(Message message) {
		return ((CallIdHeader) message.getHeader(CallIdHeader.NAME))
				.getCallId();
	}

	public static String getFromTag(Message message) {
		return ((FromHeader) message.getHeader(FromHeader.NAME)).getTag();
	}

	public static String getFromAddress(Message message) {
		return ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress()
				.toString();
	}

	public static String getFromAddrSpec(Message message) {
		String user = ((SipURI) ((FromHeader) message
				.getHeader(FromHeader.NAME)).getAddress().getURI()).getUser();
		String host = ((SipURI) ((FromHeader) message
				.getHeader(FromHeader.NAME)).getAddress().getURI()).getHost();
		return user + "@" + host;
	}

	public static String getFromDisplayName(Message message) {
		return ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress()
				.getDisplayName();
	}

	public static String getFromUserName(Message message) {
		return ((SipURI) ((FromHeader) message.getHeader(FromHeader.NAME))
				.getAddress().getURI()).getUser();
	}

	public void addSdpContent(Message message, String sdpContent)
			throws SipTesterException {
		try {
			ContentTypeHeader cth = headerFactory.createContentTypeHeader(
					"application", "sdp");
			message.setContent(sdpContent, cth);
		} catch (Exception ex) {
			throw new SipTesterException(ex);
		}
	}

	public static SessionDescription getSessionDescription(Message message) {
		if (message.getRawContent() == null)
			throw new SipTesterException(new SdpParseException(0, 0,
					"Missing sdp body"));
		try {
			String messageString = new String(message.getRawContent());
			SessionDescription sd = SdpFactory.getInstance()
					.createSessionDescription(messageString);
			return sd;
		} catch (SdpParseException ex) {
			throw new SipTesterException(ex);
		}
	}

	public static SessionDescription incrementSessionDescriptionVersionNumber(
			Response response) {
		SessionDescription sd = getSessionDescription(response);
		try {
			long versionNumber = sd.getOrigin().getSessionVersion();
			SdpFactory sdpFactory = SdpFactory.getInstance();

			SessionDescription newSd = sdpFactory.createSessionDescription(sd
					.toString());
			Origin origin = newSd.getOrigin();
			origin.setSessionVersion(versionNumber + 1);
			return newSd;
		} catch (SdpParseException ex) {
			throw new SipTesterException(ex);
		} catch (SdpException ex) {
			throw new SipTesterException(ex);
		}

	}

	public static SessionDescription incrementSessionDescriptionVersionNumber(
			SessionDescription sd) {
		try {
			long versionNumber = sd.getOrigin().getSessionVersion();
			SdpFactory sdpFactory = SdpFactory.getInstance();

			SessionDescription newSd = sdpFactory.createSessionDescription(sd
					.toString());
			Origin origin = newSd.getOrigin();
			origin.setSessionVersion(versionNumber + 1);
			return newSd;
		} catch (SdpParseException ex) {
			throw new SipTesterException(ex);
		} catch (SdpException ex) {
			throw new SipTesterException(ex);
		}
	}

	public static SessionDescription decrementSessionDescriptionVersionNumber(
			Response response) {
		SessionDescription sd = getSessionDescription(response);
		try {
			long versionNumber = sd.getOrigin().getSessionVersion();
			SdpFactory sdpFactory = SdpFactory.getInstance();

			SessionDescription newSd = sdpFactory.createSessionDescription(sd
					.toString());
			Origin origin = newSd.getOrigin();
			origin.setSessionVersion(versionNumber - 1);
			return newSd;
		} catch (SdpParseException ex) {
			throw new SipTesterException(ex);
		} catch (SdpException ex) {
			throw new SipTesterException(ex);
		}
	}

	private static ContactHeader createContactHeader(EmulatedEndpoint endpoint) {
		try {
			String ipAddress = endpoint.getIpAddress();
			int port = endpoint.getPort();
			SipURI sipUri = SipTester.getAddressFactory().createSipURI(null,
					ipAddress);
			sipUri.setPort(port);
			Address address = SipTester.getAddressFactory().createAddress(
					sipUri);
			return SipTester.getHeaderFactory().createContactHeader(address);
		} catch (Exception ex) {
			logger.fatal("Unexpected exception", ex);
			throw new SipTesterException(ex);
		}
	}

	public static RouteHeader createRouteHeader(Hop hop) {
		try {
			SipURI routeUri = SipTester.getAddressFactory().createSipURI(null,
					InetAddress.getByName(hop.getHost()).getHostAddress());
			if (hop.getPort() != -1) {
				routeUri.setPort(hop.getPort());
			}
			routeUri.setTransportParam(hop.getTransport());
			routeUri.setLrParam();
			Address routeAddress = SipTester.getAddressFactory().createAddress(
					routeUri);
			RouteHeader routeHeader = SipTester.getHeaderFactory()
					.createRouteHeader(routeAddress);
			return routeHeader;
		} catch (Exception ex) {
			String s = "Unexpected exception";
			logger.fatal(s, ex);
			throw new SipTesterException(s, ex);
		}
	}

	public static Address remapAddress(Address oldAddress,
			TraceEndpoint traceEndpoint) {
		try {
			SipURI sipUri = (SipURI) oldAddress.getURI();
			SipURI newSipUri = SipUtilities.mapUri(sipUri, traceEndpoint);
			return SipTester.getAddressFactory().createAddress(newSipUri);
		} catch (Exception ex) {
			SipTester.fail("Unexpected exception", ex);
			throw new SipTesterException(ex);
		}
	}

	static MediaDescription getMediaDescription(String desiredMediaType,
			SessionDescription sessionDescription) {
		try {
			Vector sdVector = sessionDescription.getMediaDescriptions(true);
			MediaDescription mediaDescription = null;
			for (Object md : sdVector) {
				MediaDescription media = (MediaDescription) md;
				String mediaType = media.getMedia().getMediaType();
				// audi image or video
				if (mediaType.equals(desiredMediaType)) {
					mediaDescription = media;
					break;
				}

			}
			return mediaDescription;
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipTesterException("Unexpected exception ", ex);
		}
	}

	static Set<String> getMediaTypes(SessionDescription sessionDescription) {
		HashSet<String> retval = new HashSet<String>();
		try {
			Vector sdVector = sessionDescription.getMediaDescriptions(true);
			MediaDescription mediaDescription = null;
			for (Object md : sdVector) {
				MediaDescription media = (MediaDescription) md;
				String mediaType = media.getMedia().getMediaType();
				retval.add(mediaType);
			}
			return retval;
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipTesterException("Unexpected exception ", ex);
		}
	}

	static int getSessionDescriptionMediaPort(String mediaType,
			SessionDescription sessionDescription) {
		try {
			MediaDescription mediaDescription = getMediaDescription(mediaType,
					sessionDescription);

			return mediaDescription.getMedia().getMediaPort();
		} catch (Exception ex) {
			throw new SipTesterException("Malformatted sdp", ex);
		}

	}

	static String getSessionDescriptionMediaIpAddress(String mediaType,
			SessionDescription sessionDescription) {
		try {
			String ipAddress = null;
			if (sessionDescription.getConnection() != null)
				ipAddress = sessionDescription.getConnection().getAddress();
			MediaDescription mediaDescription = getMediaDescription(mediaType,
					sessionDescription);
			if (mediaDescription == null) {
				return null;
			}

			if (mediaDescription.getConnection() != null) {
				ipAddress = mediaDescription.getConnection().getAddress();
			}
			return ipAddress;
		} catch (Exception ex) {
			throw new SipTesterException("Unexpected parse exception ", ex);
		}
	}

	static void setMediaAddressPort(SessionDescription sessionDescription,
			String mediaType, String ipAddress, int port) {
		try {
			MediaDescription mediaDescription = getMediaDescription(mediaType,
					sessionDescription);
			Connection connection = mediaDescription.getConnection();
			if (connection != null) {
				connection.setAddress(ipAddress);
				mediaDescription.setConnection(connection);
			}
			mediaDescription.getMedia().setMediaPort(port);
		} catch (Exception ex) {
			throw new SipTesterException(ex);
		}

	}

	/**
	 * This routine copies headers from inbound to outbound responses.
	 * 
	 * @param message
	 *            -- the message to transform.
	 * @param triggerMessage
	 *            -- the message that triggered this request.
	 * @param newMessage
	 *            -- the new message to copy headers into.
	 * @param newMessage
	 */
	public static void copyHeaders(SipMessage traceMessage,
			SipMessage triggerMessage, Message newMessage,
			TraceEndpoint traceEndpoint) {
		try {
			Message message = traceMessage.getSipMessage();
			logger.debug("message = " + message);
			if (triggerMessage instanceof SipRequest) {
				SipRequest sipRequest = (SipRequest) triggerMessage;
				if ( sipRequest.getRequestEvent() != null ) {
					Request request = sipRequest.getRequestEvent().getRequest();
					ReferToHeader referTo = (ReferToHeader) request
					.getHeader(ReferToHeader.NAME);
					if (referTo != null) {
						SipURIExt uri = (SipURIExt) referTo.getAddress().getURI();
						Iterator<String> headerNames = uri.getHeaderNames();
						while (headerNames.hasNext()) {
							String headerName = headerNames.next();
							String headerValue = URLDecoder.decode(uri
									.getHeader(headerName), "UTF-8");
							newMessage.setHeader(SipTester.getHeaderFactory()
									.createHeader(headerName, headerValue));
						}
					}
				}
			}
			Iterator<String> headerNames = message.getHeaderNames();
			while (headerNames.hasNext()) {
				String headerName = headerNames.next();
				if (newMessage.getHeader(headerName) == null) {
					ListIterator<Header> headerIterator = message
							.getHeaders(headerName);
					while (headerIterator.hasNext()) {
						Header header = headerIterator.next();
						Header newHeader = header;
						if (newHeader.getName().equals(ReferToHeader.NAME)) {
							ReferToHeader referToHeader = (ReferToHeader) newHeader;
							Address address = referToHeader.getAddress();
							Address newAddress = remapAddress(address,
									traceEndpoint);
							newHeader = SipTester.getHeaderFactory()
									.createReferToHeader(newAddress);
						} else if (newHeader.getName().equals(
								ReferredByHeader.NAME)) {
							ReferredByHeader referToHeader = (ReferredByHeader) newHeader;
							Address address = referToHeader.getAddress();
							Address newAddress = remapAddress(address,
									traceEndpoint);
							newHeader = ((HeaderFactoryExt) SipTester
									.getHeaderFactory())
									.createReferredByHeader(newAddress);

						} else if (newHeader.getName().equals(RouteHeader.NAME)) {
							RouteHeader routeHeader = (RouteHeader) newHeader;
							Address address = routeHeader.getAddress();
							Address newAddress = remapAddress(address,
									traceEndpoint);
							newHeader = ((HeaderFactoryExt) SipTester
									.getHeaderFactory())
									.createRouteHeader(newAddress);

						} else if (newHeader.getName().equals(
								ReplacesHeader.NAME)) {
							ReplacesHeader replacesHeader = (ReplacesHeader) newHeader;
							String fromTag = SipTester
									.getMappedFromTag(replacesHeader
											.getFromTag());
							String toTag = SipTester
									.getMappedToTag(replacesHeader.getToTag());
							String callId = replacesHeader.getCallId();
							newHeader = ((HeaderFactoryExt) SipTester
									.getHeaderFactory()).createReplacesHeader(
									callId, toTag, fromTag);

						}
						newMessage.addHeader(newHeader);
					}
				}

			}

			if (newMessage.getHeader(ProxyAuthenticateHeader.NAME) != null) {
				ProxyAuthenticateHeader pah = (ProxyAuthenticateHeader) newMessage
						.getHeader(ProxyAuthenticateHeader.NAME);
				String realm = pah.getRealm();
				DigestServerAuthenticationHelper dah = new DigestServerAuthenticationHelper();
				if (traceEndpoint.getBehavior() == Behavior.ITSP) {
					ItspAccount itspAccount = SipTester.getItspAccounts()
							.getItspAccount(
									traceEndpoint.getEmulatedEndpoint()
											.getPort());
					if (itspAccount == null) {
						SipTester
								.fail("Cannot find an itsp account at this port "
										+ traceEndpoint.getEmulatedEndpoint()
												.getPort());
						dah.generateChallenge(SipTester.getHeaderFactory(),
								((Response) newMessage), realm);
					}
				}
			}

			/*
			 * For debugging purposes, track the original branch from which this
			 * request was obtained.
			 */
			if (logger.isDebugEnabled()) {
				Header newHeader = SipTester.getHeaderFactory().createHeader(
						"x-sipx-emulated-frame",
						Integer.toString(traceMessage.getFrameId()));
				newMessage.setHeader(newHeader);
			}
			newMessage.removeHeader(RecordRouteHeader.NAME);

			ViaHeader bottomVia = SipUtilities.getBottomViaHeader(traceMessage.getSipMessage());
			String method = SipUtilities.getCSeqMethod(newMessage);
			if (message.getContent() != null) {
				ContentTypeHeader cth = ((MessageExt) message)
						.getContentTypeHeader();
				
				
				boolean spiral =  traceEndpoint.getEmulatedEndpoint().isBranchMapped(method, bottomVia.getBranch());
			    
				// BUGBUG -- revisit this code.
				if (newMessage instanceof Response || !spiral) {
					if (cth.getContentType().equals("application")
							&& cth.getContentSubType().equals("sdp")) {
						SessionDescription sdp = SipUtilities
								.getSessionDescription(message);
						String mediaAddress = SipUtilities.getSessionDescriptionMediaIpAddress("audio", sdp);
						
						String mappedAddress = mediaAddress.equals("0.0.0.0") ? "0.0.0.0" : SipTester.getTesterConfig()
								.getTesterIpAddress();
						

						/*
						 * Edit the SDP -- get new ports and stick them in
						 * there.
						 */
						sdp.getConnection().setAddress(mappedAddress);

						for (String type : SipUtilities.getMediaTypes(sdp)) {
							int port = SipUtilities
									.getSessionDescriptionMediaPort(type, sdp);
							int mappedPort = SipTester.getTesterConfig()
									.getMediaPort(port);
							setMediaAddressPort(sdp, type, mappedAddress,
									mappedPort);
						}

						newMessage.setContent(sdp, cth);

					} else {
						byte[] contents = message.getRawContent();
						newMessage.setContent(contents, cth);

					}
				} else {
					byte[] contents = message.getRawContent();
					newMessage.setContent(contents, cth);
				}

			}

		} catch (Exception ex) {
			SipTester.fail("unexepcted exception", ex);
		}

	}

	public static ViaHeader getBottomViaHeader(Message newMessage) {
		ListIterator<ViaHeader> viaHeaders = newMessage
				.getHeaders(ViaHeader.NAME);
		ViaHeader viaHeader = null;
		while (viaHeaders.hasNext()) {
			viaHeader = viaHeaders.next();
		}
		return viaHeader;
	}

	public static SipURI mapUri(SipURI uri, TraceEndpoint traceEndpoint) {
		try {
			/*
			 * Find the UserAgent that corresponds to the destination of the
			 * INVITE.
			 */
			logger.debug("mapUri " + uri);
			String toDomain = uri.getHost();
			int toPort = uri.getPort();
			String targetUser = uri.getUser();

			String newToDomain = toPort == -1 ? SipTester
					.getMappedAddress(toDomain) : SipTester
					.getMappedAddress(toDomain + ":" + toPort);
			String[] hostPort = newToDomain.split(":");
			SipURIExt retval = (SipURIExt) SipTester.getAddressFactory()
					.createSipURI(targetUser, hostPort[0]);
			if (hostPort.length > 1) {
				int newPort = Integer.parseInt(hostPort[1]);
				retval.setPort(newPort);
			}
			Iterator<String> names = uri.getParameterNames();
			while (names.hasNext()) {
				String name = names.next();
				String value = uri.getParameter(name);
				if (value != null) {
					retval.setParameter(name, value);
				}
				if (name.equals("lr")) {
					retval.setLrParam();
				}

			}
			logger.debug("mappedUri = " + retval);
			return retval;

		} catch (Exception ex) {
			throw new SipTesterException(ex);
		}

	}

	static String getSessionDescriptionMediaAttributeDuplexity(
			SessionDescription sessionDescription) {
		try {

			MediaDescription md = getMediaDescription("audio",
					sessionDescription);
			for (Object obj : md.getAttributes(false)) {
				Attribute attr = (Attribute) obj;
				if (attr.getName().equals("sendrecv"))
					return "sendrecv";
				else if (attr.getName().equals("sendonly"))
					return "sendonly";
				else if (attr.getName().equals("recvonly"))
					return "recvonly";
				else if (attr.getName().equals("inactive"))
					return "inactive";

			}
			return null;
		} catch (Exception ex) {
			throw new SipTesterException("Malformatted sdp", ex);
		}

	}

	static String getSessionDescriptionAttribute(
			SessionDescription sessionDescription) {
		try {
			Vector sessionAttributes = sessionDescription.getAttributes(false);
			if (sessionAttributes == null)
				return null;
			for (Object attr : sessionAttributes) {
				Attribute attribute = (Attribute) attr;
				if (attribute.getName().equals("sendrecv")
						|| attribute.getName().equals("sendonly")
						|| attribute.getName().equals("recvonly")
						|| attribute.getName().equals("inactive")) {
					return attribute.getName();
				}
			}
			return null;
		} catch (SdpParseException ex) {
			throw new SipTesterException(
					"Unexpected exeption retrieving a field", ex);
		}
	}

	static boolean isHoldRequest(SessionDescription sessionDescription) {

		String newIpAddress = SipUtilities.getSessionDescriptionMediaIpAddress(
				"audio", sessionDescription);

		/*
		 * Get the a media attribute -- CAUTION - this only takes care of the
		 * first media. Question - what to do when only one media stream is put
		 * on hold?
		 */

		String mediaAttribute = SipUtilities
				.getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

		String sessionAttribute = SipUtilities
				.getSessionDescriptionAttribute(sessionDescription);

		if (logger.isDebugEnabled()) {
			logger.debug("mediaAttribute = " + mediaAttribute
					+ "sessionAttribute = " + sessionAttribute);
		}
		/*
		 * RFC2543 specified that placing a user on hold was accomplished by
		 * setting the connection address to 0.0.0.0. This has been deprecated,
		 * since it doesn't allow for RTCP to be used with held streams, and
		 * breaks with connection oriented media. However, a UA MUST be capable
		 * of receiving SDP with a connection address of 0.0.0.0, in which case
		 * it means that neither RTP nor RTCP should be sent to the peer.
		 * Whenever the phone puts an external call on hold, it sends a
		 * re-INVITE to the gateway with "a=sendonly". Normally, the gateway
		 * would respond with "a=recvonly".
		 */
		String attribute = sessionAttribute != null ? sessionAttribute
				: mediaAttribute;
		if (newIpAddress.equals("0.0.0.0")) {
			return true;
		} else if (attribute != null
				&& (attribute.equals("sendonly") || attribute
						.equals("inactive"))) {
			return true;
		}
		return false;
	}

	/**
	 * Creates an emulated request based upon the request to emulate and the
	 * mapping provided for the domains and user names.
	 * 
	 * @param sipRequest
	 *            - captured request.
	 * @param triggeringMessage
	 *            - message that triggered this action ( during the test run).
	 * @param endpoint
	 *            - the emulated endpoint where this belongs.
	 * 
	 * @return emulated INVITE or null
	 */
	public static RequestExt createRequest(SipRequest traceRequest,
			SipMessage triggeringMessage, EmulatedEndpoint endpoint)
			throws Exception {
		RequestExt sipRequest = traceRequest.getSipRequest();
		SipURI requestUri = (SipURI) sipRequest.getRequestURI();
		SipURI toSipUri = (SipURI) sipRequest.getToHeader().getAddress()
				.getURI();
		SipURI fromSipUri = (SipURI) sipRequest.getFromHeader().getAddress()
				.getURI();
		String method = sipRequest.getMethod();
		SipProvider provider = endpoint.getProvider("udp");
		SipURI newRequestUri = mapUri(requestUri, endpoint.getTraceEndpoint());
		SipURI newFromURI = mapUri(fromSipUri, endpoint.getTraceEndpoint());
		Address newFromAddress = SipTester.getAddressFactory().createAddress(
				newFromURI);
		String fromTag = sipRequest.getFromHeader().getTag();
		FromHeader fromHeader = SipTester.getHeaderFactory().createFromHeader(
				newFromAddress, fromTag);
		Address toAddress = SipTester.getAddressFactory().createAddress(
				mapUri(toSipUri, endpoint.getTraceEndpoint()));
		String toTag = sipRequest.getToHeader().getTag();
		ToHeader toHeader = SipTester.getHeaderFactory().createToHeader(
				toAddress, toTag);

		CallIdHeader callIdHeader = sipRequest.getCallIdHeader();
		// CallIdHeader callIdHeader = provider.getNewCallId();

		CSeqHeader cseqHeader = sipRequest.getCSeqHeader();

		LinkedList<ViaHeader> viaList = new LinkedList<ViaHeader>();

		Iterator<ViaHeader> viaHeaders = sipRequest.getHeaders(ViaHeader.NAME);
		boolean topmostVia = true;
		while (viaHeaders.hasNext()) {
			ViaHeader vh = viaHeaders.next();
			String mappedHostPort = null;
			if (topmostVia) {
				String mappedHost = SipTester.getMappedAddress(vh.getHost());
				mappedHostPort = mappedHost + ":" + endpoint.getPort();
			} else {
				mappedHostPort = SipTester.getMappedAddress(vh.getHost() + ":"
						+ (vh.getPort() == -1 ? 5060 : vh.getPort()));
			}
			String transport = vh.getTransport();
			ViaHeader viaHeader = (ViaHeader) SipTester.getHeaderFactory()
					.createHeader(
							"Via: SIP/2.0/" + transport.toUpperCase() + " "
									+ mappedHostPort);
			viaHeader.setBranch(endpoint.getMappedBranch(method,vh.getBranch()));
			Iterator<String> parameters = vh.getParameterNames();
			while (parameters.hasNext()) {
				String parameter = parameters.next();
				String value = vh.getParameter(parameter);
				String parameterValue = SipTester.getMappedViaParameter(
						parameter, value);
				if (!parameter.equals("rport") && !parameter.equals("branch")) {
					viaHeader.setParameter(parameter, parameterValue);
				}

			}
			viaList.add(viaHeader);

		}

		byte[] content = sipRequest.getRawContent();

		MaxForwardsHeader maxForwards = (MaxForwardsHeader) sipRequest
				.getHeader(MaxForwardsHeader.NAME);

		Request newRequest = SipTester.getMessageFactory().createRequest(
				newRequestUri, method, callIdHeader, cseqHeader, fromHeader,
				toHeader, viaList, maxForwards);

		if (content != null) {
			newRequest.setContent(content, sipRequest.getContentTypeHeader());
		}

		if (sipRequest.getHeader(ContactHeader.NAME) != null) {
			ContactHeader contactHeader = SipUtilities
					.createContactHeader(endpoint);
			newRequest.setHeader(contactHeader);
		}

		if (sipRequest.getHeader(ReferToHeader.NAME) != null) {
			ReferToHeader oldReferToHeader = (ReferToHeader) sipRequest
					.getHeader(ReferToHeader.NAME);
			SipURI oldUri = (SipURI) oldReferToHeader.getAddress().getURI();
			SipURI newUri = SipUtilities.mapUri(oldUri, endpoint
					.getTraceEndpoint());
			Address newAddress = SipTester.getAddressFactory().createAddress(
					newUri);
			ReferToHeader newReferToHeader = SipTester.getHeaderFactory()
					.createReferToHeader(newAddress);
			newRequest.setHeader(newReferToHeader);
		}

		if (sipRequest.getHeader(ReferredByHeader.NAME) != null) {
			ReferredByHeader oldReferByHeader = (ReferredByHeader) sipRequest
					.getHeader(ReferredByHeader.NAME);
			SipURI oldUri = (SipURI) oldReferByHeader.getAddress().getURI();
			SipURI newUri = SipUtilities.mapUri(oldUri, endpoint
					.getTraceEndpoint());
			Address newAddress = SipTester.getAddressFactory().createAddress(
					newUri);
			ReferredByHeader newReferByHeader = SipTester.getHeaderFactory()
					.createReferredByHeader(newAddress);
			newRequest.setHeader(newReferByHeader);
		}

		Iterator<Header> routeIterator = sipRequest
				.getHeaders(RouteHeader.NAME);

		while (routeIterator.hasNext()) {
			RouteHeader routeHeader = (RouteHeader) routeIterator.next();
			logger.debug("routeHeader = " + routeHeader);
			SipURI routeUri = (SipURI) routeHeader.getAddress().getURI();
			SipURI newSipUri = SipUtilities.mapUri(routeUri, endpoint
					.getTraceEndpoint());
			Address routeAddress = SipTester.getAddressFactory().createAddress(
					newSipUri);
			RouteHeader newRouteHeader = SipTester.getHeaderFactory()
					.createRouteHeader(routeAddress);
			logger.debug("newRouteHeader " + newRouteHeader);
			newRequest.addHeader(newRouteHeader);
		}

		SipUtilities.copyHeaders(traceRequest, triggeringMessage, newRequest,
				endpoint.getTraceEndpoint());

		if (endpoint.getTraceEndpoint().getBehavior() == Behavior.UA) {
			newRequest.removeHeader(RecordRouteHeader.NAME);
		}

		return (RequestExt) newRequest;

	}

	public static ResponseExt createResponse(EmulatedEndpoint endpoint,
			RequestExt request, SipResponse traceResponse) {
		try {
			Response response = traceResponse.getSipResponse();
			int statusCode = response.getStatusCode();
			ResponseExt newResponse = (ResponseExt) SipTester
					.getMessageFactory().createResponse(statusCode, request);

			String transport = request.getTopmostViaHeader().getTransport();

			ContactHeader contactHeader = SipUtilities
					.createContactHeader(endpoint.getListeningPoint(transport));
			newResponse.setHeader(contactHeader);
			String toTag = traceResponse.getSipResponse().getToHeader()
					.getTag();

			if (toTag != null) {
				ToHeader newTo = newResponse.getToHeader();
				newTo.setTag(toTag);
			}
			ContentTypeHeader contentTypeHeader = (ContentTypeHeader) response
					.getHeader(ContentTypeHeader.NAME);
			byte[] content = response.getRawContent();
			if (content != null) {
				newResponse.setContent(content, contentTypeHeader);
			}

			copyHeaders(traceResponse, null, newResponse, endpoint
					.getTraceEndpoint());
			
			newResponse.setHeader( SipTester.getHeaderFactory().createHeader("x-sipx-emulated-frame",
					new Integer(traceResponse.getFrameId()).toString() ));

			return newResponse;
		} catch (Exception ex) {
			SipTester.fail("unxpeceted exception", ex);
			return null;
		}
	}

	private static final char[] toHex = { '0', '1', '2', '3', '4', '5', '6',
			'7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	/**
	 * convert an array of bytes to an hexadecimal string
	 * 
	 * @return a string
	 * @param b
	 *            bytes array to convert to a hexadecimal string
	 */

	public static String toHexString(byte b[]) {
		int pos = 0;
		char[] c = new char[b.length * 2];
		for (int i = 0; i < b.length; i++) {
			c[pos++] = toHex[(b[i] >> 4) & 0x0F];
			c[pos++] = toHex[b[i] & 0x0f];
		}
		return new String(c);
	}

	public static String getCorrelator(MessageExt message) {

		StringBuffer branchCorrelator = new StringBuffer();

		String triggeredBy = message instanceof Request ? "request"
				: "response";
		String branch = message.getTopmostViaHeader().getBranch();
		String method = message.getCSeqHeader().getMethod();

		branchCorrelator.append(triggeredBy);
		branchCorrelator.append("-");
		branchCorrelator.append(method);
		branchCorrelator.append("-");
		branchCorrelator.append(branch);
		if (message instanceof Response) {
			int statusCode = ((Response) message).getStatusCode();
			branchCorrelator.append("-");
			branchCorrelator.append(statusCode);
		}
		return branchCorrelator.toString().toLowerCase();
		/*
		 * try { MessageDigest m = MessageDigest.getInstance("MD5"); byte[]
		 * digest =
		 * m.digest(branchCorrelator.toString().toLowerCase().getBytes());
		 * return toHexString(digest); } catch (NoSuchAlgorithmException e) {
		 * throw new SipTesterException(e); }
		 */

	}

	/**
	 * Returns the branch-id parameter from the References header or the bottom
	 * most via header if References does not exist.
	 */
	public static String getBranchMatchId(Request request) {
		/*
		 * See if we have a references header. This will give us the connected
		 * transaction.
		 */
		String bid = null;
		try {
			ReferencesHeader referencesHeader = (ReferencesHeader) request
					.getHeader(ReferencesHeader.NAME);
			if (referencesHeader != null
					&& referencesHeader.getParameter("sipxecs-tag") != null) {
				bid = referencesHeader.getParameter("sipxecs-tag");
			}

			if (bid == null) {
				Iterator headers = request.getHeaders(ViaHeader.NAME);

				while (headers.hasNext()) {
					ViaHeader viaHeader = (ViaHeader) headers.next();
					bid = viaHeader.getBranch();
				}
			}
			return bid;
		} finally {
			logger.debug("bid = " + bid);
		}

	}

	public static String computeBranchCorrelator(MessageExt lastMessage) {
		StringBuffer branchCorrelator = new StringBuffer();

		if (lastMessage instanceof RequestExt) {
			branchCorrelator.append("request");
			branchCorrelator.append("-");
			branchCorrelator.append(((RequestExt) lastMessage).getMethod());
			branchCorrelator.append("-");
			branchCorrelator.append(lastMessage.getTopmostViaHeader()
					.getBranch());
			return branchCorrelator.toString().toLowerCase();
		} else {
			branchCorrelator.append("response");
			branchCorrelator.append("-");
			String method = lastMessage.getCSeqHeader().getMethod();
			branchCorrelator.append(method);
			branchCorrelator.append("-");
			branchCorrelator.append(lastMessage.getTopmostViaHeader()
					.getBranch());
			int rc = ((ResponseExt) lastMessage).getStatusCode();
			branchCorrelator.append("-").append(Integer.toString(rc));
			return branchCorrelator.toString().toLowerCase();
		}

	}

	public static Request createAck(SipRequest traceRequest,
			SipMessage triggeringMessage, EmulatedEndpoint endpoint,
			Response response) throws Exception {
		Request ackRequest = SipUtilities.createRequest(traceRequest,
				triggeringMessage, endpoint);
		ContactHeader cth = (ContactHeader) response
				.getHeader(ContactHeader.NAME);
		RouteHeader routeHeader = SipTester.getHeaderFactory()
				.createRouteHeader(cth.getAddress());
		((SipURI) routeHeader.getAddress().getURI()).setLrParam();
		ackRequest.setHeader(routeHeader);
		ackRequest.setHeader(response.getHeader(FromHeader.NAME));
		ackRequest.setHeader(response.getHeader(ToHeader.NAME));
		return ackRequest;
	}

	static String getStackTrace() {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		StackTraceElement[] ste = new Exception().getStackTrace();
		// Skip the log writer frame and log all the other stack frames.
		for (int i = 0; i < ste.length; i++) {
			String callFrame = "[" + ste[i].getFileName() + ":"
					+ ste[i].getLineNumber() + "]";
			pw.print(callFrame);
		}
		pw.close();
		return sw.getBuffer().toString();

	}

	public static String getDialogId(MessageExt sipMessage, boolean isServer, Behavior behavior) {
		if (behavior == Behavior.PROXY) {
			String callId = sipMessage.getCallIdHeader().getCallId();
			if (isServer) {
				return callId + ":" + sipMessage.getFromHeader().getTag();
			} else {
				return callId + ":" + sipMessage.getToHeader().getTag();
			}
		} else {
			return ((SIPMessage)sipMessage).getDialogId(isServer);
		}
	}

	

}
