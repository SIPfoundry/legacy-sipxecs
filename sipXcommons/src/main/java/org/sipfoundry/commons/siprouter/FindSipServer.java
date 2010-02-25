/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.siprouter;



import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.HashSet;
import java.util.PriorityQueue;
import java.util.Vector;

import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.xbill.DNS.Address;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.NAPTRRecord;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.TextParseException;
import org.xbill.DNS.Type;

/**
 * Finds SIP Servers using algorithms from RFC-3263
 * 
 * Specifically uses NAPTR records to find SRV records, And SRV records to find
 * A records.
 * 
 * Uses DNSJava to do DNS lookups.
 * 
 * Doesn't do any weighting or priority, possibly DNSJava already does that.
 * 
 */
public class FindSipServer {

	/**
	 * Helper class to hold name/transport pair
	 */
	class tupple {
		String name;
		String transport;

		tupple(String name, String transport) {
			this.name = name;
			this.transport = transport;
		}
	}

	Logger LOG;

	public FindSipServer(Logger log) {
		LOG = log;
	}

	/**
	 * Lookup A records for name in DNS using JavaDNS
	 * 
	 * @param name
	 * @return The first InetAddress that was found, or null it none can be
	 *         found.
	 */
	InetAddress getByName(String name) {
		try {
			InetAddress addr = Address.getByName(name);
			return addr;
		} catch (UnknownHostException e) {
			LOG.debug("FindSipServer::getByName Cannot resolve A record for "
					+ name);
			return null;
		}
	}

	/**
	 * Lookup NAPTR records for name in DNS using JavaDNS
	 * 
	 * @param name
	 * @return An array of NAPTR records, or null if none found or understood.
	 */
	Record[] getNaptrRecords(String name) {
		Record[] records = null;

		try {
			records = new Lookup(name, Type.NAPTR).run();
		} catch (TextParseException e) {
			LOG
					.warn("FindSipServer::getNaptrRecords Error parsing NAPTR record for "
							+ name);
			return null;
		}
		return records;
	}

	/**
	 * Lookup SRV records for name using JavaDNS
	 * 
	 * @param name
	 * @return An array of SRV records, or null if none found or understood.
	 */
	Record[] getSrvRecords(String name) {
		Record[] records = null;

		try {
			records = new Lookup(name, Type.SRV).run();
		} catch (TextParseException e) {
			LOG
					.warn("FindSipServer::getSrvRecords Error parsing SRV record for "
							+ name);
			return null;
		}
		return records;
	}

	/**
	 * Select a transport to use for a given SIP URI
	 * 
	 * @param uri
	 * @return The string "TCP" or "UDP"
	 */
	String pickTransport(SipURI uri) {
		String transport = uri.getTransportParam();
		if (transport == null) {
			if (uri.isSecure()) {
				transport = "TCP";
			} else {
				transport = "UDP";
			}
		}
		return transport;
	}

	/**
	 * Select the default port (5060 for udp/tcp, 5061 for TLS)
	 * 
	 * @param uri
	 * @return the port
	 */
	int pickPort(SipURI uri) {
		int port = uri.getPort();
		if (port == -1) {
			if (uri.isSecure() || 
			   (uri.getTransportParam() != null && uri.getTransportParam().equals("tls"))) {
				port = 5061;
			} else {
				port = 5060;
			}
		}
		return port;
	}

	/**
	 * Return the next hop if the URI's host is a numeric IP address
	 * 
	 * @param uri
	 * @return The Hop with IP addr, port, and transport
	 */
	Hop numericIP(SipURI uri) {
		/*
		 * RFC-3263
		 * 
		 * if no transport protocol is specified, but the TARGET is a numeric IP
		 * address, the client SHOULD use UDP for a SIP URI, and TCP for a SIPS
		 * URI.
		 */
		String transport = pickTransport(uri);
		int port = pickPort(uri);
		return new ProxyHop(uri.getHost(), port, transport);
	}

	/**
	 * Return the next hop if the URI's port is specified
	 * 
	 * @param uri
	 * @return The Hop with IP Addr, port and transport. Null if it cannot be
	 *         determined.
	 */
	Hop hasPort(SipURI uri) {
		/*
		 * RFC-3263
		 * 
		 * if no transport protocol is specified, and the TARGET is not numeric,
		 * but an explicit port is provided, the client SHOULD use UDP for a SIP
		 * URI, and TCP for a SIPS URI.
		 */
		String transport = pickTransport(uri);
		int port = pickPort(uri);

		// Lookup A record
		InetAddress addr = getByName(uri.getHost());
		LOG.debug("FindSipServer::hasPort Found by A " + uri.getHost() + "="
				+ addr);
		if (addr == null) {
			return null;
		}
		return new ProxyHop(addr.getHostAddress(), port, transport);
	}

	/**
	 * Return the next hop for a URI using the rules of RFC-3263
	 * 
	 * @param uri
	 * @return The Hop with IP addr, port and transport. Null if it cannot be
	 *         determined.
	 */
	public Hop findServer(SipURI uri) {
		Vector<tupple> srvs = new Vector<tupple>();
		InetAddress addr = null;
		String transport = null;
		int port = -1;

		LOG.debug("FindSipServer::findServer: uri " + uri.toString());

		// If the host part is numeric, use it as is
		if (Address.isDottedQuad(uri.getHost())) {
			LOG.debug("FindSipServer::findServer Host is already dotted quad "
					+ uri.getHost());
			return numericIP(uri);
		}

		// If there is a port specified, use the A record.
		if (uri.getPort() != -1) {
			LOG.debug("FindSipServer::findServer Port supplied");
			return hasPort(uri);
		}

		String bestSrvName = null;
		String bestTransport = null;
		String udpSrvName = null;
		String tcpSrvName = null;
		String tlsSrvName = null;

		/*
		 * RFC-3263
		 * 
		 * Otherwise, if no transport protocol or port is specified, and the
		 * target is not a numeric IP address, the client SHOULD perform a NAPTR
		 * query for the domain in the URI.
		 */
		// Look up NAPTR records to find the SRV Names
		try {
			LOG.debug("FindSipServer::findServer Looking up NAPTR "
					+ uri.getHost());
			Record[] records = getNaptrRecords(uri.getHost());
			if (records != null) {
				for (Record record : records) {
					String service = ((NAPTRRecord) record).getService();
					String replacement = ((NAPTRRecord) record)
							.getReplacement().toString();
					LOG.debug("FindSipServer::findServer Found NAPTR "
							+ service + "=" + replacement);
					if (bestSrvName == null) {
						bestSrvName = replacement;
					}
					if (service.equalsIgnoreCase("SIP+D2U")) {
						udpSrvName = replacement;
						if (bestTransport == null) {
							bestTransport = "UDP";
						}
					} else if (service.equalsIgnoreCase("SIP+D2T")) {
						tcpSrvName = replacement;
						if (bestTransport == null) {
							bestTransport = "TCP";
						}
					} else if (service.equalsIgnoreCase("SIPS+D2T")) {
						tlsSrvName = replacement;
						if (uri.isSecure()) {
							// Pick TLS over anything else
							bestSrvName = tlsSrvName;
							bestTransport = "TCP";
						}
						if (bestTransport == null) {
							bestTransport = "TCP";
						}

					}
				}
			}
		} catch (Throwable t) {
			LOG.error("FindServer::unexpected exception", t);
		}

		LOG.debug("FindSipServer::findServer Best NAPTR choice is "
				+ bestSrvName);
		// If NAPTR was bust, build up SRV names
		if (bestSrvName == null) {
			if (uri.isSecure()) {
				tlsSrvName = "_sips._TCP." + uri.getHost();
				bestSrvName = tlsSrvName;
				bestTransport = "TCP";
				// Don't do udp and tcp, as we must use TLS
				udpSrvName = null;
				tcpSrvName = null;
			} else {
				udpSrvName = "_sip._UDP." + uri.getHost();
				bestSrvName = udpSrvName;
				bestTransport = "UDP";
				tcpSrvName = "_sip._TCP." + uri.getHost();
			}
		}

		// If a transport is specified, use the specific SRV record for that
		// transport
		if (uri.getTransportParam() != null) {
			LOG.debug("FindSipServer::findServer Transport is specified as "
					+ uri.getTransportParam());
			if (uri.getTransportParam().equalsIgnoreCase("UDP")) {
				srvs.add(new tupple(udpSrvName, "UDP"));
			} else if (uri.getTransportParam().equalsIgnoreCase("TCP")) {
				if (uri.isSecure() && tlsSrvName != null) {
					srvs.add(new tupple(tlsSrvName, "TCP"));
				} else {
					srvs.add(new tupple(tcpSrvName, "TCP"));
				}
			}
		} else {
			// Use the best one first, then the other
			srvs.add(new tupple(bestSrvName, bestTransport));
			if (udpSrvName != null && !bestSrvName.equals(udpSrvName)) {
				srvs.add(new tupple(udpSrvName, "UDP"));
			}
			if (tcpSrvName != null && !bestSrvName.equals(tcpSrvName)) {
				srvs.add(new tupple(tcpSrvName, "TCP"));
			}
		}

		// Lookup the various SRV records until one returns a valid entry.
		for (tupple tup : srvs) {
			String srv = tup.name;
			if (srv != null) {
				Record[] records;
				LOG.debug("FindSipServer::findServer Looking up SRV " + srv);
				records = getSrvRecords(srv);
				if (records != null) {
					for (Record record : records) {
						SRVRecord srvRecord = (SRVRecord) record;
						LOG.debug("FindSipServer::findServer Found SRV "
								+ srvRecord.getTarget() + ":"
								+ srvRecord.getPort());
						// Lookup the A record for the target of the SRV
						LOG.debug("FindSipServer::findServer Looking up A "
								+ srvRecord.getTarget());
						addr = getByName(srvRecord.getTarget().toString());
						if (addr != null) {
							port = srvRecord.getPort();
							transport = tup.transport;
							LOG.debug("FindSipServer::findServer Found by SRV "
									+ srvRecord.getTarget().toString() + "="
									+ addr + ":" + port);
							// Once we find one, stop.
							break;
						}
					}
				}
			}
			if (addr != null) {
				break;
			}
		}

		if (addr == null) {
			// None found via SRV, try A record
			LOG
					.debug("FindSipServer::findServer Looking up A "
							+ uri.getHost());
			addr = getByName(uri.getHost());
			if (addr == null) {
				LOG.warn("FindSipServer::findServer Unable to resolve by A "
						+ uri.getHost());
				return null;
			}

			if (transport == null) {
				transport = pickTransport(uri);
			}
			if (port == -1) {
				port = pickPort(uri);
			}
			LOG.debug("FindSipServer::findServer Found by A " + addr);
		}

		return new ProxyHop(addr.getHostAddress(), port, transport);

	}

	/**
	 * Find the collection of SIP servers corresponding to the r-URI. A client
	 * that wishes to do its own lookup can use this method and set the maddr
	 * parameter in the R-URI.
	 * 
	 * @param uri
	 * @return
	 */
	public  Collection<Hop> findSipServers(SipURI uri) {
		Vector<tupple> srvs = new Vector<tupple>();
		InetAddress addr = null;
		String transport = null;
		int port = -1;
		HashSet<Hop> retval = new HashSet<Hop>();

		LOG.debug("FindSipServers::findServer: uri " + uri.toString());

		// If the host part is numeric, use it as is
		if (Address.isDottedQuad(uri.getHost())) {
			LOG.debug("FindSipServer::findServer Host is already dotted quad "
					+ uri.getHost());
			retval.add(numericIP(uri));
			return retval;
		}

		// If there is a port specified, use the A record.
		if (uri.getPort() != -1) {
			LOG.debug("FindSipServer::findServer Port supplied");
			Hop hop  = hasPort(uri);
			if ( hop != null ) {
				retval.add(hasPort(uri));
			}
			return retval;
		}

		String bestSrvName = null;
		String bestTransport = null;
		String udpSrvName = null;
		String tcpSrvName = null;
		String tlsSrvName = null;

		/*
		 * RFC-3263
		 * 
		 * Otherwise, if no transport protocol or port is specified, and the
		 * target is not a numeric IP address, the client SHOULD perform a NAPTR
		 * query for the domain in the URI.
		 */
		// Look up NAPTR records to find the SRV Names
		try {
			LOG.debug("FindSipServer::findServer Looking up NAPTR "
					+ uri.getHost());
			Record[] records = getNaptrRecords(uri.getHost());
			if (records != null) {
				for (Record record : records) {
					String service = ((NAPTRRecord) record).getService();
					String replacement = ((NAPTRRecord) record)
							.getReplacement().toString();
					LOG.debug("FindSipServer::findServer Found NAPTR "
							+ service + "=" + replacement);
					if (bestSrvName == null) {
						bestSrvName = replacement;
					}
					if (service.equalsIgnoreCase("SIP+D2U")) {
						udpSrvName = replacement;
						if (bestTransport == null) {
							bestTransport = "UDP";
						}
					} else if (service.equalsIgnoreCase("SIP+D2T")) {
						tcpSrvName = replacement;
						if (bestTransport == null) {
							bestTransport = "TCP";
						}
					} else if (service.equalsIgnoreCase("SIPS+D2T")) {
						tlsSrvName = replacement;
						if (uri.isSecure()) {
							// Pick TLS over anything else
							bestSrvName = tlsSrvName;
							bestTransport = "TCP";
						}
						if (bestTransport == null) {
							bestTransport = "TCP";
						}

					}
				}
			}
		} catch (Throwable t) {
			LOG.error("FindServer::unexpected exception", t);
		}

		LOG.debug("FindSipServer::findServer Best NAPTR choice is "
				+ bestSrvName);
		// If NAPTR was bust, build up SRV names
		if (bestSrvName == null) {
			if (uri.isSecure()) {
				tlsSrvName = "_sips._TCP." + uri.getHost();
				bestSrvName = tlsSrvName;
				bestTransport = "TCP";
				// Don't do udp and tcp, as we must use TLS
				udpSrvName = null;
				tcpSrvName = null;
			} else {
				udpSrvName = "_sip._UDP." + uri.getHost();
				bestSrvName = udpSrvName;
				bestTransport = "UDP";
				tcpSrvName = "_sip._TCP." + uri.getHost();
			}
		}

		// If a transport is specified, use the specific SRV record for that
		// transport
		if (uri.getTransportParam() != null) {
			LOG.debug("FindSipServer::findServer Transport is specified as "
					+ uri.getTransportParam());
			if (uri.getTransportParam().equalsIgnoreCase("UDP")) {
				srvs.add(new tupple(udpSrvName, "UDP"));
			} else if (uri.getTransportParam().equalsIgnoreCase("TCP")) {
				if (uri.isSecure() && tlsSrvName != null) {
					srvs.add(new tupple(tlsSrvName, "TCP"));
				} else {
					srvs.add(new tupple(tcpSrvName, "TCP"));
				}
			}
		} else {
			// Use the best one first, then the other
			srvs.add(new tupple(bestSrvName, bestTransport));
			if (udpSrvName != null && !bestSrvName.equals(udpSrvName)) {
				srvs.add(new tupple(udpSrvName, "UDP"));
			}
			if (tcpSrvName != null && !bestSrvName.equals(tcpSrvName)) {
				srvs.add(new tupple(tcpSrvName, "TCP"));
			}
		}

		// Lookup the various SRV records until one returns a valid entry.
		for (tupple tup : srvs) {
			String srv = tup.name;
			if (srv != null) {
				Record[] records;
				LOG.debug("FindSipServer::findServer Looking up SRV " + srv);
				records = getSrvRecords(srv);
				if (records != null) {
					for (Record record : records) {
						SRVRecord srvRecord = (SRVRecord) record;
						LOG.debug("FindSipServer::findServer Found SRV "
								+ srvRecord.getTarget() + ":"
								+ srvRecord.getPort());
						// Lookup the A record for the target of the SRV
						LOG.debug("FindSipServer::findServer Looking up A "
								+ srvRecord.getTarget());
						addr = getByName(srvRecord.getTarget().toString());
						
						if (addr != null) {
							port = srvRecord.getPort();
							transport = tup.transport;
							LOG.debug("FindSipServer::findServer Found by SRV "
									+ srvRecord.getTarget().toString() + "="
									+ addr + ":" + port);
							// Once we find one, stop.
							ProxyHop hop = new ProxyHop(addr.getHostAddress(), port,
									transport);
							hop.setPriority(srvRecord.getPriority());
							hop.setWeight(srvRecord.getWeight());
							retval.add(hop);
						}
					}
				}
			}
			if (retval.size() != 0) {
				return retval;
			}
		}

		// None found via SRV, try A record
		LOG.debug("FindSipServer::findServer Looking up A " + uri.getHost());
		addr = getByName(uri.getHost());
		if (addr == null) {
			LOG.warn("FindSipServer::findServer Unable to resolve by A "
					+ uri.getHost());
			return null;
		}

		if (transport == null) {
			transport = pickTransport(uri);
		}
		if (port == -1) {
			port = pickPort(uri);
		}
		LOG.debug("FindSipServer::findServer Found by A " + addr);
		Hop hop = new ProxyHop(addr.getHostAddress(), port, transport);
		retval.add(hop);
		return retval;

	}
	
	/**
     * Get the proxy addresses.
     */

    public PriorityQueue<Hop> getSipxProxyAddresses(SipURI proxyUri)
            throws SipRouterException {
        try {
            Collection<Hop> hops = this.findSipServers(proxyUri);
            PriorityQueue<Hop> proxyAddressTable = new PriorityQueue<Hop>();
            proxyAddressTable.addAll(hops);
            LOG.debug("proxy address table = " + proxyAddressTable);
            return proxyAddressTable;
        } catch (Exception ex) {
            LOG.error("Cannot do address lookup ", ex);
            throw new SipRouterException("Could not do dns lookup for "
                    + proxyUri, ex);
        }
    }
    
    


	/*
	 * public static void main(String[] args) { FindSipServer f = new
	 * FindSipServer(); try { SipFactory sipFactory = SipFactory.getInstance();
	 * AddressFactory addressFactory = sipFactory.createAddressFactory();
	 * javax.sip.address.Address addr =addressFactory.createAddress(
	 * "Hello<sip:woof@interop.pingtel.com:5000;transport=udp>"); Hop h =
	 * f.findServer((SipURI)addr.getURI());
	 * System.out.println("Hop is "+h.toString()); System.out.println();
	 * 
	 * addr =
	 * addressFactory.createAddress("sip:1@47.16.90.233:5160;Alert-info=sipXpage"
	 * ); h = f.findServer((SipURI)addr.getURI());
	 * System.out.println("Hop is "+h.toString()); System.out.println();
	 * 
	 * addr = addressFactory.createAddress("sip:woof@interop.pingtel.com"); h =
	 * f.findServer((SipURI)addr.getURI());
	 * System.out.println("Hop is "+h.toString()); System.out.println();
	 * 
	 * addr = addressFactory.createAddress("sip:woof@nortel.com"); h =
	 * f.findServer((SipURI)addr.getURI());
	 * System.out.println("Hop is "+h.toString()); System.out.println();
	 * 
	 * } catch (ParseException e) { // TODO Auto-generated catch block
	 * e.printStackTrace(); } catch (PeerUnavailableException e) { // TODO
	 * Auto-generated catch block e.printStackTrace(); }
	 * 
	 * }
	 */

}
