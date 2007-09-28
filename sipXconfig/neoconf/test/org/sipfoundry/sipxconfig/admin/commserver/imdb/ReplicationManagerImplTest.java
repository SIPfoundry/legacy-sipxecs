/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Base64;
import org.custommonkey.xmlunit.SimpleXpathEngine;
import org.custommonkey.xmlunit.XMLAssert;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.config.AttendantScheduleFile;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class ReplicationManagerImplTest extends TestCase {
    public ReplicationManagerImplTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testPostData() throws Exception {
        Document repDoc = XmlUnitHelper.loadDocument(getClass(), "replication.xml");
        final String data = XmlUnitHelper.asString(repDoc);

        ByteArrayOutputStream os = new ByteArrayOutputStream();
        InputStream is = new ByteArrayInputStream("replication was successful"
                .getBytes("US-ASCII"));

        IMocksControl control = EasyMock.createControl();
        final HttpURLConnection urlConnection = control.createMock(MockHttpURLConnection.class);
        urlConnection.setDoOutput(true);
        urlConnection.setRequestMethod("POST");
        urlConnection.setRequestProperty("Content-length", Integer.toString(data.length()));
        urlConnection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
        urlConnection.getOutputStream();
        control.andReturn(os);
        urlConnection.getInputStream();
        control.andReturn(is);
        urlConnection.connect();
        urlConnection.getResponseMessage();
        control.andReturn("");
        urlConnection.getHeaderField("ErrorInReplication");
        control.andReturn("");
        control.replay();

        ReplicationManagerImpl impl = new ReplicationManagerImpl() {
            protected HttpURLConnection getConnection(String url) {
                return urlConnection;
            }
        };

        assertTrue(impl.postData("http://bongo.com/replication.cgi", data.getBytes("UTF-8")));

        XMLAssert.assertXMLEqual(XmlUnitHelper.asString(repDoc), new String(os.toByteArray()));
        control.verify();
    }

    public void testGenerateXMLDataToPost() throws Exception {
        ReplicationManagerImpl impl = new ReplicationManagerImpl();
        byte[] data = new byte[] {
            15, 7, 123, -127, 126, 0
        };

        Document document = impl.generateXMLDataToPost(data, DataSet.EXTENSION.getName(),
                "database");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);

        XMLAssert.assertXpathEvaluatesTo(DataSet.EXTENSION.getName(),
                "/replicationdata/data/@target_data_name", domDoc);
        XMLAssert.assertXpathEvaluatesTo("database", "/replicationdata/data/@type", domDoc);
        XMLAssert.assertXpathEvaluatesTo("replace", "/replicationdata/data/@action", domDoc);
        XMLAssert.assertXpathEvaluatesTo("comm-server",
                "/replicationdata/data/@target_component_type", domDoc);
        XMLAssert.assertXpathEvaluatesTo("CommServer1",
                "/replicationdata/data/@target_component_id", domDoc);

        SimpleXpathEngine simpleXpathEngine = new SimpleXpathEngine();
        String payload = simpleXpathEngine.evaluate("/replicationdata/data/payload", domDoc);
        for (int i = 0; i < data.length; i++) {
            assertEquals(data[i], Base64.decodeBase64(payload.getBytes("US-ASCII"))[i]);
        }
    }

    public void testReplicateFile() throws Exception {
        final Document testDoc = XmlUnitHelper.loadDocument(getClass(), "replication.test.xml");

        // this may be slightly ugly. overriding all methods used internally by
        // replicateFile in order to focus testing on that method
        ReplicationManager out = new ReplicationManagerImpl() {
            boolean postData(String url, byte[] xmlData) throws IOException {
                return true;
            }

            Document generateXMLDataToPost(byte[] payload, String targetDataName, String dataType) {
                assertEquals(ConfigFileType.ATTENDANT_SCHEDULE.getName(), targetDataName);
                assertEquals("file", "file");
                try {
                    String testFile = new String(payload);
                    String controlFile = testDoc.asXML();
                    XMLAssert.assertXMLEqual(controlFile, testFile);
                } catch (Exception e) {
                    fail(e.getMessage());
                }
                return testDoc;
            }
        };

        Location location = new Location();

        XmlFile xmlFile = new AttendantScheduleFile() {
            public Document getDocument() {
                try {
                    return XmlUnitHelper.loadDocument(getClass(), "replication.test.xml");
                } catch (Exception e) {
                    fail(e.getMessage());
                    return null;
                }
            }
        };

        out.replicateFile(new Location[] {
            location
        }, xmlFile);
    }

    private static class MockHttpURLConnection extends HttpURLConnection {
        public MockHttpURLConnection() throws Exception {
            super(new URL("http://test"));
        }

        public void disconnect() {
        }

        public boolean usingProxy() {
            return false;
        }

        public void connect() {
        }
    }
}
