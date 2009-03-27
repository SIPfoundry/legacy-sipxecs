/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sdp.MediaDescriptionImpl;
import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.header.ims.PAssertedIdentityHeader;
import gov.nist.javax.sip.header.ims.PrivacyHeader;

import java.io.InputStream;
import java.text.ParseException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Properties;
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
import javax.sip.Dialog;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.InReplyToHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.OrganizationHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.ReplyToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ServerHeader;
import javax.sip.header.SubjectHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.UserAgentHeader;
import javax.sip.header.ViaHeader;
import javax.sip.header.WarningHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * 
 * @author mranga
 * 
 */
@SuppressWarnings("unchecked")
class SipUtilities {

	private static Logger logger = Logger.getLogger(SipUtilities.class);
	static UserAgentHeader userAgent;
	static ServerHeader serverHeader;
	private static final String CONFIG_PROPERTIES = "config.properties";

	/**
	 * Create the UA header.
	 * 
	 * @return
	 */
	static UserAgentHeader createUserAgentHeader() {
		if (userAgent != null) {
			return userAgent;
		} else {
			try {
				InputStream cfg = SipUtilities.class.getClassLoader()
						.getResourceAsStream(CONFIG_PROPERTIES);
				if (cfg != null) {
					Properties configProperties = new Properties();

					configProperties.load(cfg);
					userAgent = (UserAgentHeader) ProtocolObjects.headerFactory
							.createHeader(UserAgentHeader.NAME, String.format(
									"sipXecs/%s sipXecs/sipxbridge (Linux)",
									configProperties.get("version")));
					cfg.close();
					return userAgent;
				} else {
					userAgent = (UserAgentHeader) ProtocolObjects.headerFactory
							.createHeader(UserAgentHeader.NAME, String.format(
									"sipXecs/%s sipXecs/sipxbridge (Linux)",
									"xxxx.yyyy"));
					logger.warn("Creating Default User Agent Header");
					return userAgent;

				}
			} catch (Exception e) {
				throw new SipXbridgeException(
						"unexpected exception creating UserAgent header", e);

			}
		}
	}

	/**
	 * Possibly create and return the default Server header.
	 * 
	 * @return
	 */
	static ServerHeader createServerHeader() {
		if (serverHeader != null) {
			return serverHeader;
		} else {
			try {
				InputStream cfg = SipUtilities.class.getClassLoader()
						.getResourceAsStream(CONFIG_PROPERTIES);
				if (cfg != null) {
					Properties configProperties = new Properties();
					configProperties.load(cfg);
					serverHeader = (ServerHeader) ProtocolObjects.headerFactory
							.createHeader(ServerHeader.NAME, String.format(
									"sipXecs/%s sipXecs/sipxbridge (Linux)",
									configProperties.get("version")));
					cfg.close();
					return serverHeader;
				} else {
					serverHeader = (ServerHeader) ProtocolObjects.headerFactory
							.createHeader(ServerHeader.NAME, String.format(
									"sipXecs/%s sipXecs/sipxbridge (Linux)",
									"xxxx.yyyy"));
					logger.warn("Creating Default Server Header");
					return serverHeader;
				}
			} catch (Exception e) {
				throw new SipXbridgeException(
						"unexpected exception creating UserAgent header", e);

			}
		}
	}

	static MediaDescription getMediaDescription(
			SessionDescription sessionDescription) {
		try {
			Vector sdVector = sessionDescription.getMediaDescriptions(true);
			MediaDescription mediaDescription = null;
			for (Object md : sdVector) {
				MediaDescription media = (MediaDescription) md;
				String mediaType = media.getMedia().getMediaType();
				if (mediaType.equals("audio") || mediaType.equals("image")
						|| mediaType.equals("video")) {
					mediaDescription = media;
					break;
				}

			}
			return mediaDescription;
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException("Unexpected exception ", ex);
		}
	}

	/**
	 * Create a Via header for a given provider and transport.
	 */
	static ViaHeader createViaHeader(SipProvider sipProvider, String transport) {
		try {
			if (!transport.equalsIgnoreCase("tcp")
					&& !transport.equalsIgnoreCase("udp"))
				throw new IllegalArgumentException("Bad transport");

			ListeningPoint listeningPoint = sipProvider
					.getListeningPoint(transport);
			String host = listeningPoint.getIPAddress();
			int port = listeningPoint.getPort();
			return ProtocolObjects.headerFactory.createViaHeader(host, port,
					listeningPoint.getTransport(), null);
		} catch (Exception ex) {
			throw new SipXbridgeException("Unexpected exception ", ex);
		}

	}

	/**
	 * Get the Via header to assign for this message processor. The topmost via
	 * header of the outoging messages use this.
	 * 
	 * @return the ViaHeader to be used by the messages sent via this message
	 *         processor.
	 */
	static ViaHeader createViaHeader(SipProvider sipProvider,
			ItspAccountInfo itspAccount) {
		try {
			if (itspAccount != null && !itspAccount.isGlobalAddressingUsed()) {
				ListeningPoint listeningPoint = sipProvider
						.getListeningPoint(itspAccount.getOutboundTransport());
				String host = listeningPoint.getIPAddress();
				int port = listeningPoint.getPort();
				ViaHeader viaHeader = ProtocolObjects.headerFactory
						.createViaHeader(host, port, listeningPoint
								.getTransport(), null);

				return viaHeader;

			} else {
				// Check -- what other parameters need to be set for NAT
				// traversal here?
				String transport = itspAccount != null ? itspAccount
						.getOutboundTransport()
						: Gateway.DEFAULT_ITSP_TRANSPORT;
				return ProtocolObjects.headerFactory.createViaHeader(Gateway
						.getGlobalAddress(), Gateway.getGlobalPort(),
						transport, null);

			}

		} catch (Exception ex) {
			logger.fatal("Unexpected exception creating via header", ex);
			throw new SipXbridgeException("Could not create via header", ex);
		}
	}

	/**
	 * Create a contact header for the given provider.
	 */
	static ContactHeader createContactHeader(SipProvider provider,
			ItspAccountInfo itspAccount) {
		try {
			/*
			 * If we are creating a contact header for a provider that is facing
			 * the WAN and if global addressing is used, then place the bridge
			 * public address in the contact header.
			 */
			if (provider != Gateway.getLanProvider()
					&& (itspAccount != null && !itspAccount
							.isGlobalAddressingUsed())
					|| Gateway.getGlobalAddress() == null) {
				String transport = itspAccount != null ? itspAccount
						.getOutboundTransport()
						: Gateway.DEFAULT_ITSP_TRANSPORT;
				String userName = itspAccount != null ? itspAccount
						.getUserName() : null;
				if (userName == null) {
					userName = Gateway.SIPXBRIDGE_USER;
				}
				ListeningPoint lp = provider.getListeningPoint(transport);
				String ipAddress = lp.getIPAddress();
				int port = lp.getPort();
				SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(
						userName, ipAddress);
				sipUri.setPort(port);
				sipUri.setTransportParam(transport);
				Address address = ProtocolObjects.addressFactory
						.createAddress(sipUri);
				ContactHeader ch = ProtocolObjects.headerFactory
						.createContactHeader(address);
				ch.removeParameter("expires");
				return ch;

			} else if (provider == Gateway.getLanProvider()) {
				/*
				 * Creating contact header for LAN bound request.
				 */
				return SipUtilities.createContactHeader(
						Gateway.SIPXBRIDGE_USER, provider);
			} else {
				/*
				 * Creating contact header for WAN bound request. Nothing is
				 * known about this ITSP. We just use global addressing.
				 */

				ContactHeader contactHeader = ProtocolObjects.headerFactory
						.createContactHeader();
				String userName = itspAccount != null ? itspAccount
						.getUserName() : null;
				if (userName == null) {
					userName = Gateway.SIPXBRIDGE_USER;
				}
				SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(
						userName, Gateway.getGlobalAddress());
				sipUri.setPort(Gateway.getGlobalPort());
				String transport = itspAccount != null ? itspAccount
						.getOutboundTransport() : "udp";
				sipUri.setTransportParam(transport);
				Address address = ProtocolObjects.addressFactory
						.createAddress(sipUri);
				contactHeader.setAddress(address);
				contactHeader.removeParameter("expires");
				return contactHeader;
			}
		} catch (Exception ex) {
			logger.fatal("Unexpected exception creating contact header", ex);
			throw new SipXbridgeException(
					"Unexpected error creating contact header", ex);
		}

	}

	/**
	 * Create a contact header for the given provider.
	 */
	static ContactHeader createContactHeader(String user, SipProvider provider) {
		try {

			/*
			 * The preferred transport of the sipx proxy server ( defaults to
			 * tcp ).
			 */
			String transport = Gateway.getSipxProxyTransport();

			ListeningPoint lp = provider.getListeningPoint(transport);
			String ipAddress = lp.getIPAddress();
			int port = lp.getPort();
			SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(user,
					ipAddress);
			sipUri.setPort(port);
			if (transport.equals("tls")) {
				sipUri.setTransportParam(transport);
			}
			Address address = ProtocolObjects.addressFactory
					.createAddress(sipUri);
			ContactHeader ch = ProtocolObjects.headerFactory
					.createContactHeader(address);
			return ch;
		} catch (Exception ex) {
			throw new SipXbridgeException(
					"Unexpected error creating contact header", ex);
		}
	}

	/**
	 * Create a basic registration request.
	 */

	static Request createRegistrationRequestTemplate(
			ItspAccountInfo itspAccount, SipProvider sipProvider)
			throws ParseException, InvalidArgumentException {

		String registrar = itspAccount.getProxyDomain();

		SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(null,
				registrar);
		int port = itspAccount.getInboundProxyPort();
		if (port != 5060) {
			requestUri.setPort(port);
		}
		if (itspAccount.getOutboundTransport().equalsIgnoreCase("tcp")) {
			requestUri.setTransportParam("tcp");
		}

		/*
		 * We register with From and To headers set to the proxy domain.
		 */
		String proxyDomain = itspAccount.getProxyDomain();
		SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(
				itspAccount.getUserName(), proxyDomain);

		SipURI toUri = ProtocolObjects.addressFactory.createSipURI(itspAccount
				.getUserName(), proxyDomain);

		Address fromAddress = ProtocolObjects.addressFactory
				.createAddress(fromUri);

		FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(
				fromAddress, new Long(Math.abs(new java.util.Random()
						.nextLong())).toString());

		Address toAddress = ProtocolObjects.addressFactory.createAddress(toUri);

		ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
				toAddress, null);

		CallIdHeader callid = sipProvider.getNewCallId();

		CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(
				1L, Request.REGISTER);

		MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
				.createMaxForwardsHeader(70);

		ViaHeader viaHeader = null;

		viaHeader = createViaHeader(sipProvider, itspAccount);

		List<ViaHeader> list = new LinkedList<ViaHeader>();
		list.add(viaHeader);

		Request request = ProtocolObjects.messageFactory.createRequest(
				requestUri, Request.REGISTER, callid, cseqHeader, fromHeader,
				toHeader, list, maxForwards);

		SipUtilities.addWanAllowHeaders(request);

		if (itspAccount.getOutboundRegistrar() != null
				&& !itspAccount.getProxyDomain().equals(
						itspAccount.getOutboundRegistrar())) {
			String outboundRegistrarRoute = itspAccount.getOutboundRegistrar();
			SipURI routeUri = ProtocolObjects.addressFactory.createSipURI(null,
					outboundRegistrarRoute);
			routeUri.setPort(itspAccount.getInboundProxyPort());
			routeUri.setLrParam();
			Address routeAddress = ProtocolObjects.addressFactory
					.createAddress(routeUri);
			RouteHeader routeHeader = ProtocolObjects.headerFactory
					.createRouteHeader(routeAddress);
			request.addHeader(routeHeader);
		}

		return request;
	}

	/**
	 * Creates a deregistration request and sends it out to deregister ourselves
	 * from the proxy server.
	 * 
	 * @param sipProvider
	 * @param itspAccount
	 * @return
	 * @throws SipXbridgeException
	 */
	static Request createDeregistrationRequest(SipProvider sipProvider,
			ItspAccountInfo itspAccount) throws SipXbridgeException {
		try {

			Request request = createRegistrationRequestTemplate(itspAccount,
					sipProvider);

			ContactHeader contactHeader = createContactHeader(sipProvider,
					itspAccount);

			request.addHeader(contactHeader);
			ExpiresHeader expiresHeader = ProtocolObjects.headerFactory
					.createExpiresHeader(0);
			request.addHeader(expiresHeader);
			return request;
		} catch (Exception ex) {
			String s = "Unexpected error creating register";
			logger.error(s, ex);
			throw new SipXbridgeException(s, ex);

		}

	}

	/**
	 * Create an OPTIONS Request
	 */

	static Request createOptionsRequest(SipProvider sipProvider,
			ItspAccountInfo itspAccount) throws SipXbridgeException {
		try {
			SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(
					null, itspAccount.getSipDomain());

			SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(
					itspAccount.getUserName(), itspAccount.getSipDomain());

			SipURI toUri = ProtocolObjects.addressFactory.createSipURI(
					itspAccount.getUserName(), itspAccount.getSipDomain());

			Address fromAddress = ProtocolObjects.addressFactory
					.createAddress(fromUri);

			FromHeader fromHeader = ProtocolObjects.headerFactory
					.createFromHeader(fromAddress, new Long(Math
							.abs(new java.util.Random().nextLong())).toString());

			Address toAddress = ProtocolObjects.addressFactory
					.createAddress(toUri);

			ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
					toAddress, null);

			CallIdHeader callid = sipProvider.getNewCallId();

			CSeqHeader cseqHeader = ProtocolObjects.headerFactory
					.createCSeqHeader(1L, Request.OPTIONS);

			MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
					.createMaxForwardsHeader(70);

			ViaHeader viaHeader = null;

			viaHeader = createViaHeader(sipProvider, itspAccount);

			List<ViaHeader> list = new LinkedList<ViaHeader>();
			list.add(viaHeader);

			Request request = ProtocolObjects.messageFactory.createRequest(
					requestUri, Request.OPTIONS, callid, cseqHeader,
					fromHeader, toHeader, list, maxForwards);

			return request;
		} catch (Exception ex) {
			throw new SipXbridgeException("Error creating OPTIONS request", ex);
		}
	}

	/**
	 * Create a Registration request for the given ITSP account.
	 * 
	 * @param sipProvider
	 * @param itspAccount
	 * @return
	 * @throws SipXbridgeException
	 */
	static Request createRegistrationRequest(SipProvider sipProvider,
			ItspAccountInfo itspAccount) throws SipXbridgeException {

		try {
			Request request = createRegistrationRequestTemplate(itspAccount,
					sipProvider);

			ContactHeader contactHeader = createContactHeader(sipProvider,
					itspAccount);
			contactHeader.removeParameter("expires");

			request.addHeader(contactHeader);
			int registrationTimer = itspAccount.getSipKeepaliveMethod().equals(
					Request.REGISTER) ? Gateway.getSipKeepaliveSeconds()
					: itspAccount.getRegistrationInterval();
			ExpiresHeader expiresHeader = ProtocolObjects.headerFactory
					.createExpiresHeader(registrationTimer);
			request.addHeader(expiresHeader);
			return request;
		} catch (ParseException ex) {
			String s = "Unexpected error creating register -- check proxy configuration ";
			logger.error(s, ex);
			throw new SipXbridgeException(s, ex);

		} catch (InvalidArgumentException ex) {
			logger.error("An unexpected exception occured", ex);
			throw new SipXbridgeException("Internal error", ex);
		}

	}

	/**
	 * Create a registration query.
	 * 
	 * @param sipProvider
	 * @param itspAccount
	 * @return
	 * @throws SipXbridgeException
	 */
	static Request createRegisterQuery(SipProvider sipProvider,
			ItspAccountInfo itspAccount) throws SipXbridgeException {
		try {
			Request request = createRegistrationRequestTemplate(itspAccount,
					sipProvider);

			return request;
		} catch (Exception ex) {
			String s = "Unexpected error creating register ";
			logger.error(s, ex);
			throw new SipXbridgeException(s, ex);

		}

	}

	static Request createInviteRequest(SipURI requestUri,
			SipProvider sipProvider, ItspAccountInfo itspAccount,
			FromHeader from, String callId) throws SipXbridgeException {
		try {

			String toUser = requestUri.getUser();
			String toDomain = itspAccount.getProxyDomain();
			String fromUser = ((SipURI) from.getAddress().getURI()).getUser();
			String fromDomain = ((SipURI) from.getAddress().getURI()).getHost();
			String fromDisplayName = from.getAddress().getDisplayName();
			if (fromDisplayName == null) {
				fromDisplayName = "sipxbridge";
			}

			Address address = itspAccount.getCallerAlias();

			PAssertedIdentityHeader passertedIdentityHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
					.createPAssertedIdentityHeader(address);

			PrivacyHeader privacyHeader = null;

			FromHeader fromHeader = from;

			/*
			 * Handle the case of anonymous calling. If the from header has
			 * anonymous@invalid then attach a P-Preferred-Identity Header to
			 * reflect the actual value of the caller Id.
			 */
			if (fromUser.equalsIgnoreCase("anonymous")
					&& fromDomain.equalsIgnoreCase("invalid")) {

				String domain = "anonymous.invalid";
				SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(
						fromUser, domain);
				fromHeader = ProtocolObjects.headerFactory.createFromHeader(
						ProtocolObjects.addressFactory.createAddress(fromUri),
						new Long(Math.abs(new java.util.Random().nextLong()))
								.toString());
				privacyHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
						.createPrivacyHeader("id");
			} else {
				fromHeader = (FromHeader) from.clone();

				fromHeader.setTag(new Long(Math.abs(new java.util.Random()
						.nextLong())).toString());
				if (itspAccount.stripPrivateHeaders()) {
					privacyHeader = ((HeaderFactoryExt) ProtocolObjects.headerFactory)
							.createPrivacyHeader("id");
				}
			}

			/*
			 * Remove stuff from the request that can have an effect on the
			 * routing of the request and add stuff that we want to add.
			 */

			/*
			 * If the target is a phone number we set user=phone. Otherwise we
			 * remove it.
			 */
			if (itspAccount.isUserPhone()) {
				/*
				 * If call routed to phone then the RURI MUST have user=phone
				 * set ( sipconnect requirement ).
				 */
				requestUri.setParameter("user", "phone");
			} else {
				requestUri.removeParameter("user");
			}

			if (itspAccount.getOutboundTransport() != null) {
				requestUri
						.setTransportParam(itspAccount.getOutboundTransport());
			} else {
				requestUri.removeParameter("transport");
			}

			requestUri.removePort();

			requestUri.removeParameter("maddr");

			fromHeader.setTag(new Long(Math.abs(new java.util.Random()
					.nextLong())).toString());

			SipURI toUri = ProtocolObjects.addressFactory.createSipURI(toUser,
					toDomain);

			if (itspAccount.isUserPhone()) {
				toUri.setParameter("user", "phone");
			}

			ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
					ProtocolObjects.addressFactory.createAddress(toUri), null);

			CSeqHeader cseqHeader = ProtocolObjects.headerFactory
					.createCSeqHeader(1L, Request.INVITE);

			MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
					.createMaxForwardsHeader(70);

			ViaHeader viaHeader = createViaHeader(sipProvider, itspAccount);

			List<ViaHeader> list = new LinkedList<ViaHeader>();
			list.add(viaHeader);

			CallIdHeader callid = ProtocolObjects.headerFactory
					.createCallIdHeader(callId);
			Request request = ProtocolObjects.messageFactory.createRequest(
					requestUri, Request.INVITE, callid, cseqHeader, fromHeader,
					toHeader, list, maxForwards);

			/*
			 * Attach the PreferredIdentity header if we generated one.
			 */
			if (passertedIdentityHeader != null) {
				request.setHeader(passertedIdentityHeader);
			}

			/*
			 * Attach the privacy header if we generated one.
			 */
			if (privacyHeader != null) {
				request.setHeader(privacyHeader);
			}

			/*
			 * Strip any identifying information if needed.
			 */
			if (itspAccount.stripPrivateHeaders()) {
				/* Remove the display name */
				SipUtilities.stripPrivateHeaders(request);
			}

			Gateway.getAuthenticationHelper().setAuthenticationHeaders(request);

			ContactHeader contactHeader = createContactHeader(sipProvider,
					itspAccount);
			request.addHeader(contactHeader);

			String outboundProxy = itspAccount.getOutboundProxy();
			SipURI routeUri = ProtocolObjects.addressFactory.createSipURI(null,
					outboundProxy);
			if (itspAccount.getOutboundProxyPort() != 0) {
				routeUri.setPort(itspAccount.getOutboundProxyPort());
			}
			routeUri.setLrParam();
			Address routeAddress = ProtocolObjects.addressFactory
					.createAddress(routeUri);
			RouteHeader routeHeader = ProtocolObjects.headerFactory
					.createRouteHeader(routeAddress);
			request.addHeader(routeHeader);
			if (itspAccount.stripPrivateHeaders()) {
				SipUtilities.stripPrivateHeaders(request);
			}
			return request;

		} catch (Exception ex) {
			String s = "Unexpected error creating INVITE ";
			logger.error(s, ex);
			throw new SipXbridgeException(s, ex);

		}
	}

	static SessionDescription getSessionDescription(Message message)
			throws SdpParseException {
		if (message.getRawContent() == null)
			throw new SdpParseException(0, 0, "Missing sdp body");
		String messageString = new String(message.getRawContent());
		SessionDescription sd = SdpFactory.getInstance()
				.createSessionDescription(messageString);
		return sd;

	}

	/**
	 * Extract and return the media formats from the session description.
	 * 
	 * @param sessionDescription
	 *            -- the SessionDescription to examine.
	 * @return a Set of media formats. Each element is a codec number.
	 */
	static Set<Integer> getMediaFormats(SessionDescription sessionDescription) {
		try {
			Vector mediaDescriptions = sessionDescription
					.getMediaDescriptions(true);

			HashSet<Integer> retval = new HashSet<Integer>();
			for (Iterator it = mediaDescriptions.iterator(); it.hasNext();) {
				MediaDescription mediaDescription = (MediaDescription) it
						.next();
				Vector formats = mediaDescription.getMedia().getMediaFormats(
						true);
				for (Iterator it1 = formats.iterator(); it1.hasNext();) {
					Object format = it1.next();
					int fmt = new Integer(format.toString());
					retval.add(fmt);
				}
			}
			return retval;
		} catch (Exception ex) {
			logger.fatal("Unexpected exception!", ex);
			throw new SipXbridgeException(
					"Unexpected exception getting media formats", ex);
		}
	}

	/**
	 * Cleans the Session description to include only the specified codec set.
	 * It removes all the SRTP related fields as well.
	 * 
	 * @param sessionDescription
	 *            -- the session description to clean ( modify )
	 * @param codecs
	 *            -- the codec set to restrict to.
	 * @return
	 */
	static SessionDescription cleanSessionDescription(
			SessionDescription sessionDescription, Set<Integer> codecs) {
		try {

			if (codecs.isEmpty()) {
				return sessionDescription;
			}

			logger.debug("Codecs = " + codecs);
			Vector mediaDescriptions = sessionDescription
					.getMediaDescriptions(true);

			for (Iterator it = mediaDescriptions.iterator(); it.hasNext();) {

				MediaDescription mediaDescription = (MediaDescription) it
						.next();
				Vector formats = mediaDescription.getMedia().getMediaFormats(
						true);

				for (Iterator it1 = formats.iterator(); it1.hasNext();) {
					Object format = it1.next();
					Integer fmt = new Integer(format.toString());
					if (!codecs.contains(fmt)) {
						it1.remove();
					}
				}

				Vector attributes = mediaDescription.getAttributes(true);
				for (Iterator it1 = attributes.iterator(); it1.hasNext();) {
					Attribute attr = (Attribute) it1.next();
					if (logger.isDebugEnabled()) {
						logger.debug("attrName = " + attr.getName());
					}

					if (attr.getName().equalsIgnoreCase("rtpmap")) {
						String attribute = attr.getValue();
						String[] attrs = attribute.split(" ");
						int rtpMapCodec = Integer.parseInt(attrs[0]);

						if (!codecs.contains(rtpMapCodec)) {
							it1.remove();
						}
					} else if (attr.getName().equalsIgnoreCase("crypto")) {
						it1.remove();
						logger.debug("Not adding crypto");
					} else if (attr.getName().equalsIgnoreCase("encryption")) {
						it1.remove();
						logger.debug("Not adding encryption");
					}
				}

			}

			return sessionDescription;

		} catch (Exception ex) {
			logger.fatal("Unexpected exception!", ex);
			throw new SipXbridgeException("Unexpected exception cleaning SDP",
					ex);
		}
	}

	static String getSessionDescriptionMediaIpAddress(
			SessionDescription sessionDescription) {
		try {
			String ipAddress = null;
			if (sessionDescription.getConnection() != null)
				ipAddress = sessionDescription.getConnection().getAddress();
			MediaDescription mediaDescription = getMediaDescription(sessionDescription);
			if (mediaDescription == null) {
				return null;
			}

			if (mediaDescription.getConnection() != null) {
				ipAddress = mediaDescription.getConnection().getAddress();
			}
			return ipAddress;
		} catch (Exception ex) {
			throw new SipXbridgeException("Unexpected parse exception ", ex);
		}
	}

	static String getSessionDescriptionMediaAttributeDuplexity(
			SessionDescription sessionDescription) {
		try {

			MediaDescription md = getMediaDescription(sessionDescription);
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
			throw new SipXbridgeException("Malformatted sdp", ex);
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
			throw new SipXbridgeException(
					"Unexpected exeption retrieving a field", ex);
		}
	}

	static void setDuplexity(SessionDescription sessionDescription,
			String attributeValue) {

		try {

			MediaDescriptionImpl md = (MediaDescriptionImpl) getMediaDescription(sessionDescription);
			md.setDuplexity(attributeValue);

		} catch (Exception ex) {
			logger.error("Error while processing the following SDP : "
					+ sessionDescription);
			logger.error("attributeValue = " + attributeValue);
			throw new SipXbridgeException("Malformatted sdp", ex);
		}

	}

	static int getSessionDescriptionMediaPort(
			SessionDescription sessionDescription) {
		try {
			MediaDescription mediaDescription = getMediaDescription(sessionDescription);

			return mediaDescription.getMedia().getMediaPort();
		} catch (Exception ex) {
			throw new SipXbridgeException("Malformatted sdp", ex);
		}

	}

	static long getSeqNumber(Message message) {
		return ((CSeqHeader) message.getHeader(CSeqHeader.NAME)).getSeqNumber();
	}

	static String getCallId(Message message) {
		String callId = ((CallIdHeader) message.getHeader(CallIdHeader.NAME))
				.getCallId();

		return callId;
	}

	private static Response createResponse(SipProvider provider,
			Request request, int statusCode) throws ParseException {
		Response response = ProtocolObjects.messageFactory.createResponse(
				statusCode, request);
		ContactHeader contactHeader = createContactHeader(
				Gateway.SIPXBRIDGE_USER, provider);
		response.addHeader(contactHeader);

		if (provider == Gateway.getLanProvider()) {
			SupportedHeader sh = ProtocolObjects.headerFactory
					.createSupportedHeader("replaces");
			response.setHeader(sh);
			response.addHeader(ProtocolObjects.headerFactory
					.createSupportedHeader("100rel"));
		} else {
			SupportedHeader sh = ProtocolObjects.headerFactory
					.createSupportedHeader("replaces");
			response.setHeader(sh);
		}
		return response;
	}

	static Response createResponse(Transaction transaction, int statusCode) {
		try {
			Request request = transaction.getRequest();

			SipProvider provider = ((TransactionExt) transaction)
					.getSipProvider();
			Response response = createResponse(provider, request, statusCode);
			Dialog dialog = transaction.getDialog();
			/*
			 * Privacy enabled ? if so, remove private headers while sending
			 * response.
			 */
			if (dialog.getApplicationData() != null
					&& DialogContext.get(dialog).getItspInfo() != null
					&& DialogContext.get(dialog).getItspInfo()
							.stripPrivateHeaders()
					&& provider != Gateway.getLanProvider()) {
				SipUtilities.stripPrivateHeaders(response);
			}
			return response;

		} catch (ParseException ex) {
			logger.fatal("Unexpected parse exception", ex);
			throw new SipXbridgeException("Unexpected parse exceptione", ex);
		}
	}

	static SipProvider getPeerProvider(SipProvider provider, String transport) {
		if (transport == null) {
			transport = Gateway.DEFAULT_ITSP_TRANSPORT;
		}
		if (provider == Gateway.getLanProvider()) {
			return Gateway.getWanProvider(transport);
		} else {
			return Gateway.getLanProvider();
		}
	}

	/**
	 * Fix up the Origin field, ip address and port of the given session
	 * description.
	 * 
	 * @param sessionDescription
	 *            -- session description field to fix up.
	 * 
	 * @param address
	 *            -- new address to assign to media
	 * 
	 * @param port
	 *            -- new port to assign.
	 */
	static void fixupSdpMediaAddresses(SessionDescription sessionDescription,
			String address, int port) {
		try {
			Connection connection = sessionDescription.getConnection();

			if (connection != null) {
				connection.setAddress(address);
			}

			Origin origin = sessionDescription.getOrigin();
			origin.setAddress(address);
			MediaDescription mediaDescription = getMediaDescription(sessionDescription);
			if (mediaDescription.getConnection() != null) {
				mediaDescription.getConnection().setAddress(address);
			}

			mediaDescription.getMedia().setMediaPort(port);

		} catch (Exception ex) {
			logger.error("Unepxected exception fixing up sdp addresses", ex);
			throw new SipXbridgeException(
					"Unepxected exception fixing up sdp addresses", ex);
		}

	}

	static void setSessionDescriptionAttribute(String attribute,
			SessionDescription sessionDescription) {
		try {
			sessionDescription.setAttribute("a", attribute);
		} catch (SdpException ex) {
			logger.error("Unexpected exception ", ex);
			throw new SipXbridgeException("Unexpected exception", ex);
		}

	}

	/**
	 * Increment the session version.
	 * 
	 * @param sessionDescription
	 */
	static void incrementSessionVersion(SessionDescription sessionDescription) {
		try {
			long version = sessionDescription.getOrigin().getSessionVersion();
			sessionDescription.getOrigin().setSessionVersion(++version);
		} catch (SdpException ex) {
			logger.error("Unexpected exception ", ex);
			throw new SipXbridgeException("Unexepcted exception", ex);
		}

	}

	/**
	 * Fix up request to use global addressing.
	 * 
	 * @param request
	 */
	static void setGlobalAddresses(Request request) {
		try {
			SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null,
					Gateway.getGlobalAddress());
			sipUri.setPort(Gateway.getGlobalPort());

			ContactHeader contactHeader = (ContactHeader) request
					.getHeader(ContactHeader.NAME);
			contactHeader.getAddress().setURI(sipUri);
			ViaHeader viaHeader = (ViaHeader) request.getHeader(ViaHeader.NAME);
			viaHeader.setHost(Gateway.getGlobalAddress());
			viaHeader.setPort(Gateway.getGlobalPort());
		} catch (Exception ex) {
			logger.error("Unexpected exception ", ex);
			throw new SipXbridgeException("Unexepcted exception", ex);
		}

	}

	static void setGlobalAddress(Response response) {
		try {
			SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null,
					Gateway.getGlobalAddress());
			sipUri.setPort(Gateway.getGlobalPort());

			ContactHeader contactHeader = (ContactHeader) response
					.getHeader(ContactHeader.NAME);
			contactHeader.getAddress().setURI(sipUri);

		} catch (Exception ex) {
			logger.error("Unexpected exception ", ex);
			throw new SipXbridgeException("Unexepcted exception", ex);
		}
	}

	static String getFromAddress(Message message) {
		FromHeader fromHeader = (FromHeader) message.getHeader(FromHeader.NAME);
		return fromHeader.getAddress().toString();
	}

	static String getToAddress(Message message) {
		ToHeader toHeader = (ToHeader) message.getHeader(ToHeader.NAME);
		return toHeader.getAddress().toString();
	}

	static boolean isSdpOfferSolicitation(Request request) {
		return request.getMethod().equals(Request.INVITE)
				&& request.getContentLength().getContentLength() == 0;

	}

	static String getToUser(Message message) {
		return ((SipURI) ((ToHeader) message.getHeader(ToHeader.NAME))
				.getAddress().getURI()).getUser();

	}

	static HashSet<Integer> getCommonCodec(SessionDescription sd1,
			SessionDescription sd2) {

		Set<Integer> codecSet1 = getMediaFormats(sd1);
		Set<Integer> codecSet2 = getMediaFormats(sd2);
		HashSet<Integer> union = new HashSet<Integer>();
		union.addAll(codecSet1);
		union.addAll(codecSet2);
		HashSet<Integer> retval = new HashSet<Integer>();
		for (int codec : union) {
			if (codecSet1.contains(codec) && codecSet2.contains(codec)) {
				retval.add(codec);
			}
		}
		return retval;

	}

	static void setSessionDescription(Message message,
			SessionDescription sessionDescription) {
		try {
			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("application", "sdp");
			message.setContent(sessionDescription.toString(), cth);
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException(ex);
		}

	}

	/**
	 * Return true if the session description contains at least one codec of the
	 * specified set.
	 * 
	 * @param sd
	 * @param codecSet
	 * @return
	 */
	static boolean isCodecSupported(SessionDescription sd, Set<Integer> codecSet) {

		Set<Integer> codecs = SipUtilities.getMediaFormats(sd);
		for (int codecNumber : codecSet) {
			if (codecs.contains(codecNumber))
				return true;
		}
		return false;
	}

	static boolean isPrackAllowed(Message message) {
		ListIterator li = message.getHeaders(AllowHeader.NAME);

		while (li != null && li.hasNext()) {
			AllowHeader ah = (AllowHeader) li.next();
			if (ah.getMethod().equals(Request.PRACK)) {
				return true;
			}
		}
		return false;
	}

	static WarningHeader createWarningHeader(String agent, int code, String text) {
		try {
			return ProtocolObjects.headerFactory.createWarningHeader(agent,
					code, text);
		} catch (Exception ex) {
			logger.fatal(String.format(
					"Unexpected Error creating warning header %s %d %s", agent,
					code, text));
			return null;
		}
	}

	static String getSessionDescriptionMediaType(
			SessionDescription sessionDescription) {
		try {
			MediaDescription mediaDescription = (MediaDescription) sessionDescription
					.getMediaDescriptions(true).get(0);
			String mediaType = mediaDescription.getMedia().getMediaType();
			logger.debug("media type " + mediaType);
			return mediaType;

		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException(ex);
		}

	}

	static void addLanAllowHeaders(Message message) {
		message.removeHeader(AllowHeader.NAME); // Remove existing Allow
		try {
			for (String method : new String[] { Request.INVITE, Request.BYE,
					Request.ACK, Request.CANCEL, Request.REFER,
					Request.OPTIONS, Request.PRACK }) {
				AllowHeader allow = ProtocolObjects.headerFactory
						.createAllowHeader(method);
				message.addHeader(allow);
			}
			if (message instanceof Request) {
				SupportedHeader supportedHeader = ProtocolObjects.headerFactory
						.createSupportedHeader("replaces");
				message.setHeader(supportedHeader);
				supportedHeader = ProtocolObjects.headerFactory
						.createSupportedHeader("100rel");
				message.addHeader(supportedHeader);
			}
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException(ex);
		}

	}

	static void addWanAllowHeaders(Message message) {

		message.removeHeader(AllowHeader.NAME); // Remove existing Allow
		try {
			for (String method : new String[] { Request.INVITE, Request.BYE,
					Request.ACK, Request.CANCEL, Request.OPTIONS, Request.PRACK }) {
				AllowHeader allow = ProtocolObjects.headerFactory
						.createAllowHeader(method);
				message.addHeader(allow);
			}
			if (message instanceof Request) {
				SupportedHeader supportedHeader = ProtocolObjects.headerFactory
						.createSupportedHeader("100rel");
				message.setHeader(supportedHeader);
			}
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException(ex);
		}
	}

	static boolean isCodecDifferent(SessionDescription sd1,
			SessionDescription sd2) {

		try {
			MediaDescription md1 = getMediaDescription(sd1);
			MediaDescription md2 = getMediaDescription(sd2);
			HashSet<String> fmt1 = new HashSet<String>(md1.getMedia()
					.getMediaFormats(true));
			HashSet<String> fmt2 = new HashSet<String>(md2.getMedia()
					.getMediaFormats(true));
			logger.debug("Comparing " + fmt1 + " with " + fmt2 + " returning "
					+ !fmt1.equals(fmt2));
			return !fmt1.equals(fmt2);
		} catch (Exception ex) {
			logger.error("Unexpected exception", ex);
			throw new SipXbridgeException(ex);
		}
	}

	static boolean isReplacesHeaderPresent(Request referRequest) {
		ReferToHeader referToHeader = (ReferToHeader) referRequest
				.getHeader(ReferToHeader.NAME);

		if (referToHeader == null) {
			return false;
		}

		if (referRequest.getHeader(ReplacesHeader.NAME) != null) {
			return true;
		}

		SipURI uri = (SipURI) referToHeader.getAddress().getURI();

		/* Does the refer to header contain a Replaces? ( attended transfer ) */
		String replacesParam = uri.getHeader(ReplacesHeader.NAME);

		return replacesParam != null;
	}

	static SipURI createInboundRequestUri(ItspAccountInfo itspInfo) {

		try {
			String address;

			address = itspInfo == null || !itspInfo.isGlobalAddressingUsed() ? Gateway
					.getLocalAddress()
					: Gateway.getGlobalAddress();

			SipURI retval = (SipURI) ProtocolObjects.addressFactory
					.createURI("sip:" + address);

			int port = itspInfo == null || !itspInfo.isGlobalAddressingUsed() ? Gateway
					.getBridgeConfiguration().getExternalPort()
					: Gateway.getGlobalPort();

			retval.setPort(port);

			retval.setUser(itspInfo.getUserName());

			return retval;
		} catch (Exception ex) {
			logger.error("unexpected error creating inbound Request URI ", ex);
			throw new SipXbridgeException("Unexpected error creating RURI ", ex);
		}
	}

	static SipURI createInboundReferredByUri(ItspAccountInfo itspInfo) {
		try {
			String address;

			address = itspInfo.getCallerId();

			SipURI retval = (SipURI) ProtocolObjects.addressFactory
					.createURI("sip:" + address);

			return retval;
		} catch (Exception ex) {
			logger.error("unexpected error creating inbound Request URI ", ex);
			throw new SipXbridgeException("Unexpected error creating RURI ", ex);
		}
	}

	static SessionDescription cloneSessionDescription(SessionDescription sd) {
		try {
			return SdpFactory.getInstance().createSessionDescription(
					sd.toString());
		} catch (Exception ex) {
			logger.error("unexpected exception cloning sd", ex);
			throw new SipXbridgeException("unexpected exception cloning sd", ex);
		}
	}

	static long getSessionDescriptionVersion(
			SessionDescription sessionDescription) {
		try {
			long version = sessionDescription.getOrigin().getSessionVersion();
			return version;
		} catch (SdpException ex) {
			logger.error("Unexpected exception ", ex);
			throw new SipXbridgeException("Unexepcted exception", ex);
		}
	}

	static void addAllowHeaders(Message message, SipProvider inboundProvider) {
		if (inboundProvider == Gateway.getLanProvider()) {
			addLanAllowHeaders(message);
		} else {
			addWanAllowHeaders(message);
		}
	}

	static void fixupOutboundRequest(Dialog dialog, Request request) {

		SipProvider provider = ((DialogExt) dialog).getSipProvider();
		try {
			if (provider != Gateway.getLanProvider()) {
				/*
				 * Request is bound to the WAN.
				 */
				if (DialogContext.get(dialog).getItspInfo() == null
						|| DialogContext.get(dialog).getItspInfo()
								.isGlobalAddressingUsed()) {
					SipUtilities.setGlobalAddresses(request);
					SipUtilities.addWanAllowHeaders(request);
					request.removeHeader(SupportedHeader.NAME);

				}
			} else {
				SipUtilities.addLanAllowHeaders(request);
				request.setHeader(ProtocolObjects.headerFactory
						.createSupportedHeader("replaces"));
			}
		} catch (Exception ex) {
			String s = "Unexpected exception";
			logger.fatal(s, ex);
			throw new SipXbridgeException(s, ex);
		}

	}

	static void stripPrivateHeaders(Message message) {
		try {
			message.removeHeader(SubjectHeader.NAME);
			message.removeHeader(OrganizationHeader.NAME);
			message.removeHeader(ReplyToHeader.NAME);
			message.removeHeader(InReplyToHeader.NAME);
			message.removeHeader(UserAgentHeader.NAME);
			if (message instanceof Request) {
				Address fromAddress = ((FromHeader) message
						.getHeader(FromHeader.NAME)).getAddress();
				fromAddress.setDisplayName(null);
			}
		} catch (Exception ex) {
			String s = "Unexpected exception";
			logger.fatal(s, ex);
			throw new SipXbridgeException(s, ex);
		}

	}

	static boolean isOriginatorSipXbridge(Message incomingMessage) {
		ListIterator headerIterator = incomingMessage
				.getHeaders(ViaHeader.NAME);
		boolean spiral = false;
		while (headerIterator.hasNext()) {
			ViaHeader via = (ViaHeader) headerIterator.next();
			String originator = via
					.getParameter(BackToBackUserAgent.ORIGINATOR);
			if (originator != null
					&& originator.equals(Gateway.SIPXBRIDGE_USER)) {
				spiral = true;
			}
		}
		return spiral;
	}

	public static void addSipFrag(Response response, String message) {
		try {
			ContentTypeHeader cth = ProtocolObjects.headerFactory
					.createContentTypeHeader("message", "sipfrag");
			response.setContent(message, cth);
		} catch (Exception ex) {
			String s = "Unexpected exception";
			logger.fatal(s, ex);
			throw new SipXbridgeException(s, ex);
		}

	}

}
