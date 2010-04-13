/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.siprouter;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.ParseException;
import java.util.HashMap;
import java.util.Properties;

import javax.sip.PeerUnavailableException;
import javax.sip.SipFactory;
import javax.sip.address.AddressFactory;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import junit.framework.TestCase;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.junit.Test;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.xbill.DNS.NAPTRRecord;
import org.xbill.DNS.Name;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;


/**
 * Test the FindSipServer class 
 */
public class FindSipServerTest extends TestCase {
	
	static final Logger LOG = Logger.getRootLogger();

	/**
	 * Create a debug version of FindSipServer that doesn't use live DNS records, and instead
	 * returns canned records filled in by each test.
	 */
	private class FindSipServerDebug extends FindSipServer {

		FindSipServerDebug(Logger log) {
			super(log);
		}

		HashMap<String, InetAddress> fakeA = null ;
		HashMap<String, Record[]> fakeNaptr = null ;
		HashMap<String, Record[]> fakeSrv = null ;
		@Override
		public InetAddress getByName(String name) {
			if (fakeA == null)
				return null ;
			return fakeA.get(name.toLowerCase());
		}

		@Override
		public Record[] getNaptrRecords(String name) {
			if (fakeNaptr == null)
				return null ;
			return fakeNaptr.get(name.toLowerCase()) ;
		}

		@Override
		public Record[] getSrvRecords(String name) {
			if (fakeSrv == null)
				return null ;
			return fakeSrv.get(name.toLowerCase()) ;
		}
	}
	
	public void setUp() {
		Properties props = new Properties();
		props.setProperty("log4j.rootLogger", "debug, cons");
		props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
		props.setProperty("log4j.appender.cons.layout", "org.apache.log4j.SimpleLayout");
		PropertyConfigurator.configure(props);
	}
	
	@Test
	public void testPickTransport() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServer f = new FindSipServerDebug(LOG);
		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@47.16.90.233:5160;transport=udp");
		String transport = f.pickTransport((SipURI)addr.getURI()).toLowerCase();
		assertEquals("udp", transport) ;

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:1@47.16.90.233:5160;transport=tcp");
		transport = f.pickTransport((SipURI)addr.getURI()).toLowerCase();
		assertEquals("tcp", transport) ;

		// Unspecified transport, should be UDP
		addr = addressFactory.createAddress("sip:1@47.16.90.233:5160");
		transport = f.pickTransport((SipURI)addr.getURI()).toLowerCase();
		assertEquals("udp", transport) ;

		// Unspecified transport on SIPS, should be TCP
		addr = addressFactory.createAddress("sips:1@47.16.90.233");
		transport = f.pickTransport((SipURI)addr.getURI()).toLowerCase();
		assertEquals("tcp", transport) ;
	}

	@Test
	public void testNumericIP() throws ParseException, PeerUnavailableException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServer f = new FindSipServer(LOG);
		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@47.16.90.233:5160;transport=udp");
		Hop h = f.numericIP((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("47.16.90.233", h.getHost());
		assertEquals(5160, h.getPort());
		
		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@5.4.3.2;transport=tcp");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5060, h.getPort());

		// Unspecified transport, should be UDP
		addr = addressFactory.createAddress("sip:3@5.4.3.2");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5060, h.getPort());

		// SIPS Unspecified transport, should be TCP
		addr = addressFactory.createAddress("sips:4@5.4.3.2");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5061, h.getPort());
	}

	@Test
	public void testHasPort() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServerDebug f = new FindSipServerDebug(LOG);
		
		try {
			f.fakeA = new HashMap<String, InetAddress>(); 
			f.fakeA.put("puppy", InetAddress.getByName("1.2.3.4"));
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@puppy:5160;transport=udp");
		Hop h = f.hasPort((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(5160, h.getPort());

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@puppy:6666;transport=tcp");
		h = f.hasPort((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(6666, h.getPort());

		// Unspecified transport should be UDP
		addr = addressFactory.createAddress("sip:3@puppy:7777");
		h = f.hasPort((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(7777, h.getPort());
		
		// SIPS Unspecified transport should be TCP
		addr = addressFactory.createAddress("sips:4@puppy:8888");
		h = f.hasPort((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(8888, h.getPort());
	}

	@Test
	public void testFindServerNumericIP() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServer f = new FindSipServer(LOG);
		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@47.16.90.233:5160;transport=udp");
		Hop h = f.numericIP((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("47.16.90.233", h.getHost());
		assertEquals(5160, h.getPort());
		
		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@5.4.3.2;transport=tcp");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5060, h.getPort());


		// Unspecified transport, should be UDP
		addr = addressFactory.createAddress("sip:3@5.4.3.2");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5060, h.getPort());

		// SIPS Unspecified transport, should be TCP
		addr = addressFactory.createAddress("sips:4@5.4.3.2");
		h = f.numericIP((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("5.4.3.2", h.getHost());
		assertEquals(5061, h.getPort());
	}

	@Test
	public void testFindServerHasPort() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServerDebug f = new FindSipServerDebug(LOG);
		
		try {
			f.fakeA = new HashMap<String, InetAddress>(); 
			f.fakeA.put("puppy", InetAddress.getByName("1.2.3.4"));
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@puppy:5160;transport=udp");
		Hop h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(5160, h.getPort());

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@puppy:6666;transport=tcp");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(6666, h.getPort());

		// Unspecified transport should be UDP
		addr = addressFactory.createAddress("sip:3@puppy:7777");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(7777, h.getPort());
		
		// SIPS Unspecified transport should be TCP
		addr = addressFactory.createAddress("sips:4@puppy:8888");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(8888, h.getPort());
		
		// Unknown host
		f.fakeA = null;
		addr = addressFactory.createAddress("sips:5@puppy:9999");
		h = f.findServer((SipURI)addr.getURI());
		assertNull(h);
	}

	@Test
	public void testFindServerNoNAPTRorSRV() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServerDebug f = new FindSipServerDebug(LOG);
		
		try {
			f.fakeA = new HashMap<String, InetAddress>(); 
			f.fakeA.put("puppy", InetAddress.getByName("1.2.3.4"));
			f.fakeA.put("dog", InetAddress.getByName("1.2.3.5"));
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@puppy;transport=udp");
		Hop h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(5060, h.getPort());

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@dog;transport=tcp");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.5", h.getHost());
		assertEquals(5060, h.getPort());

		// Unspecified transport should be UDP
		addr = addressFactory.createAddress("sip:3@puppy");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.4", h.getHost());
		assertEquals(5060, h.getPort());
		
		// SIPS Unspecified transport should be TCP
		addr = addressFactory.createAddress("sips:4@dog");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("1.2.3.5", h.getHost());
		assertEquals(5061, h.getPort());
		
		// Unknown A record
		addr = addressFactory.createAddress("sip:5@knight");
		h = f.findServer((SipURI)addr.getURI());
		assertNull(h);
	}

	@Test
	public void testFindServerNoNAPTR() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServerDebug f = new FindSipServerDebug(LOG);
		
		try {
			f.fakeA = new HashMap<String, InetAddress>(); 
			f.fakeA.put("puppy", InetAddress.getByName("1.2.3.4"));
			f.fakeA.put("dog", InetAddress.getByName("1.2.3.5"));
			f.fakeA.put("target2.", InetAddress.getByName("2.2.2.2"));
			f.fakeA.put("target4.", InetAddress.getByName("3.3.3.3"));

			f.fakeSrv = new HashMap<String, Record[]>();
			Record[] recsPuppy = new Record[2] ;
			recsPuppy[0] = new SRVRecord(new Name("name1."),0, 0, 0, 0, 5555, new Name("target1."));
			recsPuppy[1] = new SRVRecord(new Name("name2."),0, 0, 0, 0, 6666, new Name("target2."));
			f.fakeSrv.put("_sip._udp.puppy", recsPuppy);
			f.fakeSrv.put("_sip._tcp.puppy", recsPuppy);
			f.fakeSrv.put("_sips._tcp.puppy", recsPuppy);
			Record[] recsDog = new Record[2] ;
			recsDog[0] = new SRVRecord(new Name("name3."),0, 0, 0, 0, 5555, new Name("target3."));
			recsDog[1] = new SRVRecord(new Name("name4."),0, 0, 0, 0, 6666, new Name("target4."));
			f.fakeSrv.put("_sip._udp.dog", recsDog);
			f.fakeSrv.put("_sip._tcp.dog", recsDog);
			f.fakeSrv.put("_sips._tcp.dog", recsDog);
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@puppy;transport=udp");
		Hop h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("2.2.2.2", h.getHost());
		assertEquals(6666, h.getPort());

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@dog;transport=tcp");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("3.3.3.3", h.getHost());
		assertEquals(6666, h.getPort());

		// Unspecified transport should be UDP
		addr = addressFactory.createAddress("sip:3@dog");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("3.3.3.3", h.getHost());
		assertEquals(6666, h.getPort());
		
		// SIPS Unspecified transport should be TCP
		addr = addressFactory.createAddress("sips:4@dog");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("3.3.3.3", h.getHost());
		assertEquals(6666, h.getPort());
		
		// No matching A record
		f.fakeA = null ;
		addr = addressFactory.createAddress("sip:5@fred");
		h = f.findServer((SipURI)addr.getURI());
		assertNull(h);
	}

	@Test
	public void testFindServer() throws PeerUnavailableException, ParseException {
		SipFactory sipFactory = SipFactory.getInstance();
		AddressFactory addressFactory = sipFactory.createAddressFactory();
		FindSipServerDebug f = new FindSipServerDebug(LOG);
		
		try {
			f.fakeA = new HashMap<String, InetAddress>(); 
			f.fakeA.put("puppy", InetAddress.getByName("1.2.3.4"));
			f.fakeA.put("dog", InetAddress.getByName("1.2.3.5"));
			f.fakeA.put("target2.", InetAddress.getByName("2.2.2.2"));
			f.fakeA.put("target4.", InetAddress.getByName("3.3.3.3"));
			f.fakeA.put("target6.", InetAddress.getByName("4.4.4.4"));

			f.fakeNaptr = new HashMap<String, Record[]>();
			Record[] recsNaptr = new Record[3];
			recsNaptr[0] = new NAPTRRecord(new Name("naptr1."),0,0,0,0,"string5","SIP+D2U","string7",new Name("_sip._udp.naptr.")); 
			recsNaptr[1] = new NAPTRRecord(new Name("naptr2."),0,0,0,0,"string5","SIP+D2T","string7",new Name("_sip._tcp.naptr."));
			recsNaptr[2] = new NAPTRRecord(new Name("naptr3."),0,0,0,0,"string5","SIPS+D2T","string7",new Name("_sips._tcp.naptr."));
			f.fakeNaptr.put("poopus", recsNaptr);
			
			f.fakeSrv = new HashMap<String, Record[]>();
			Record[] recsPuppy = new Record[2] ;
			recsPuppy[0] = new SRVRecord(new Name("name1."),0, 0, 0, 0, 5555, new Name("target1."));
			recsPuppy[1] = new SRVRecord(new Name("name2."),0, 0, 0, 0, 6666, new Name("target2."));
			f.fakeSrv.put("_sip._udp.puppy", recsPuppy);
			f.fakeSrv.put("_sip._tcp.puppy", recsPuppy);
			f.fakeSrv.put("_sips._tcp.puppy", recsPuppy);
			Record[] recsDog = new Record[2] ;
			recsDog[0] = new SRVRecord(new Name("name3."),0, 0, 0, 0, 5555, new Name("target3."));
			recsDog[1] = new SRVRecord(new Name("name4."),0, 0, 0, 0, 6666, new Name("target4."));
			f.fakeSrv.put("_sip._udp.dog", recsDog);
			f.fakeSrv.put("_sip._tcp.dog", recsDog);
			f.fakeSrv.put("_sips._tcp.dog", recsDog);
			Record[] recsNap = new Record[2] ;
			recsNap[0] = new SRVRecord(new Name("name5."),0, 0, 0, 0, 7777, new Name("target5."));
			recsNap[1] = new SRVRecord(new Name("name6."),0, 0, 0, 0, 8888, new Name("target6."));
			f.fakeSrv.put("_sip._udp.naptr.", recsNap);
			f.fakeSrv.put("_sip._tcp.naptr.", recsNap);
			f.fakeSrv.put("_sips._tcp.naptr.", recsNap);
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// Specific transport UDP
		javax.sip.address.Address addr = addressFactory.createAddress("sip:1@poopus;transport=udp");
		Hop h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("4.4.4.4", h.getHost());
		assertEquals(8888, h.getPort());

		// Specific transport TCP
		addr = addressFactory.createAddress("sip:2@poopus;transport=tcp");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("4.4.4.4", h.getHost());
		assertEquals(8888, h.getPort());

		// Unspecified transport should be UDP
		addr = addressFactory.createAddress("sip:3@poopus");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("udp", h.getTransport().toLowerCase()) ;
		assertEquals("4.4.4.4", h.getHost());
		assertEquals(8888, h.getPort());
		
		// SIPS Unspecified transport should be TCP
		addr = addressFactory.createAddress("sips:4@poopus");
		h = f.findServer((SipURI)addr.getURI());
		assertEquals("tcp", h.getTransport().toLowerCase()) ;
		assertEquals("4.4.4.4", h.getHost());
		assertEquals(8888, h.getPort());
		
		// No matching A record
		f.fakeA = null ;
		addr = addressFactory.createAddress("sip:5@fred");
		h = f.findServer((SipURI)addr.getURI());
		assertNull(h);
	}

}
