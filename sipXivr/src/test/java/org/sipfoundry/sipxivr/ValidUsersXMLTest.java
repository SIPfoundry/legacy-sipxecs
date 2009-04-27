/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.IOException;
import java.io.StringReader;
import java.util.Properties;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import junit.framework.TestCase;

public class ValidUsersXMLTest extends TestCase {

    /*
     * public void testUpdate() { fail("Not yet implemented"); }
     * 
     * public void testImdb() { fail("Not yet implemented"); }
     * 
     * public void testLookupDTMF() { fail("Not yet implemented"); }
     * 
     * public void testIsValidUser() { fail("Not yet implemented"); }
     * 
     */
	public void testWalkXML() {
		String xml = 
			"<validusers xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/validusers-00-00\">" +
            "  <user>" +
            "   <aliases/>" +
            "   <identity>+9663@woof.us.nortel.com</identity>" +
            "   <userName>+9663</userName>" +
            "   <displayName>Woof Dogg</displayName>" +
            "   <contact>\"Woof Dogg\"&lt;sip:+9663@woof.us.nortel.com&gt;</contact>" +
            "   <pintoken>8ce33da2e5e332b81cc67c4f49077e7d</pintoken>" +
            "   <inDirectory>false</inDirectory>" +
            "  </user>" +
			"  <user>" +
			"  <aliases>" +
			"    <alias>42</alias>" +
			"    <alias>24</alias>" +
			"  </aliases>" +
		    "  <identity>lab@woof.us.nortel.com</identity>" +
		    "  <userName>lab</userName>" +
		    "  <displayName>Knight Dogg</displayName>" +
		    "  <contact>\"Knight Dogg\"&lt;sip:lab@woof.us.nortel.com&gt;</contact>" +
		    "  <pintoken>dae9763344ea0122eb7b10ab857e2704</pintoken>" +
		    "  <inDirectory>false</inDirectory>" +
		    "  </user>" +
		    "</validusers>";
		
		Document validUsers = null;
		DocumentBuilder builder;
		try {
			builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
	        validUsers = builder.parse(new InputSource(new StringReader(xml))) ;
	        
		} catch (ParserConfigurationException e) {
			fail("exception "+e);
		} catch (SAXException e) {
			fail("exception "+e);
		} catch (IOException e) {
			fail("exception "+e);
		}
        
		ValidUsersXML vu = ValidUsersXML.update(false) ;
		vu.walkXML(validUsers) ;
		User u = vu.isValidUser("+9663");  // primary user name
		assertNotNull(u);
		u = vu.isValidUser("lab");  // primary user name
		assertNotNull(u);
		assertEquals("lab", u.getUserName());
		u = vu.isValidUser("puppy") ; // no such user name
		assertNull(u);
		u = vu.isValidUser("42"); // Alias for lab
		assertNotNull(u);
		assertEquals("lab", u.getUserName());
		u = vu.isValidUser("24"); // Another alias for lab
		assertNotNull(u);
		assertEquals("lab", u.getUserName());
	}
    public void testCompress() {
        assertEquals("WOOF", ValidUsersXML.compress("Woof!")); 
        assertEquals("WOOFWASHERE", ValidUsersXML.compress("Woof! was here."));
        assertEquals("SPYDERTROBINSONRD", ValidUsersXML.compress("Spyder T. Robinson 3rd"));
        assertEquals("FRAUBRUCHER", ValidUsersXML.compress("Fräu BrÜcher"));
    }

    public void testMapDTMF() {
        assertEquals("9663", ValidUsersXML.mapDTMF("WOOF"));
        assertEquals("22233344455566677778889999", ValidUsersXML
                .mapDTMF("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        assertEquals("", ValidUsersXML.mapDTMF(null));
        assertEquals("22333", ValidUsersXML.mapDTMF("A B-D.E?F"));
    }

    public void testGetDisplayName() {
        assertNull(ValidUsersXML.getDisplayPart(null));
        assertNull(ValidUsersXML.getDisplayPart("woof"));
        assertEquals("", ValidUsersXML.getDisplayPart("sip:woof@somewhere"));
        assertEquals("woof", ValidUsersXML.getDisplayPart("woof <sip:woof@somewhere>"));
        assertEquals("Quebit T. Dogg", ValidUsersXML
                .getDisplayPart("\"Quebit T. Dogg\" <sip:woof@somewhere>"));
    }

    public void testBuildDialPatterns() {
        User u = new User();

        u.setDisplayName(null);
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("!@#$%^&*");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("woof");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(1, u.getDialPatterns().size());
        assertEquals("9663", u.getDialPatterns().firstElement());

        u.setDisplayName("Quebit T. Dogg");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(3, u.getDialPatterns().size());
        assertEquals("78324883644", u.getDialPatterns().get(0));
        assertEquals("83644783248", u.getDialPatterns().get(1));
        assertEquals("36447832488", u.getDialPatterns().get(2));

    }

    public void testPin() {
        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "debug, cons");
        props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
        props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.cons.layout.facility", "sipXivr");

        User u = new User();
        u.setIdentity("201@woof3.fuzzy");
        u.setUserName("201");
        u.setPintoken("3c46585d782a8b9444feca1c697e4d4f");
        
        assertTrue(u.isPinCorrect("9663", "woof3.fuzzy"));
    }
}
