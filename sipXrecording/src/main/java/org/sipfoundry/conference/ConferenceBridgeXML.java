/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.conference;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Vector;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.servlet.ServletException;
import javax.servlet.ServletInputStream;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Trigger the transfer of a conference recording using a simple HTTP request.
 */
public class ConferenceBridgeXML {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");
    private static ConferenceBridgeXML s_current;
    private static File s_conferenceBridgeFile;
    private static long s_lastModified;

    private static Vector<ConferenceBridgeItem> m_confs;
    private static HashMap<String, ConferenceBridgeItem> m_confNameMap;
    private static String s_conferenceBridgeFileName = System.getProperty("conf.dir", "/etc/sipxpbx")+"/conferencebridge.xml";

    /**
     * Private constructor for updatable singleton
     */
    private ConferenceBridgeXML() {
        m_confNameMap = new HashMap<String, ConferenceBridgeItem>();
        m_confs = new Vector<ConferenceBridgeItem>();
    }

    public static ConferenceBridgeXML update(boolean load) throws Exception {
        if (s_current == null || s_conferenceBridgeFile == null ||
                s_conferenceBridgeFile.lastModified() != s_lastModified) {
            s_current = new ConferenceBridgeXML();
            if (load) {
                s_current.loadXML();
            }
        }
        return s_current;
    }

    /**
     * Load the conferencebridge.xml file
     */
    void loadXML() throws Exception {
        LOG.info("Loading conferencebridge.xml configuration");
        Document conferenceBridge = null;
        s_conferenceBridgeFile = new File(s_conferenceBridgeFileName);
        s_lastModified = s_conferenceBridgeFile.lastModified();
        DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        conferenceBridge = builder.parse(s_conferenceBridgeFile);
        walkXML(conferenceBridge);
    }

    private String walkXML(Document confBridgeDoc) {
        String nodeString = "";

            confBridgeDoc.getDocumentElement().normalize();
            NodeList nodeLst = confBridgeDoc.getElementsByTagName("item");
            for (int s = 0; s < nodeLst.getLength(); s++) {
                Node fstNode = nodeLst.item(s);
                if (fstNode.getNodeType() == Node.ELEMENT_NODE) {
                    String bridgeName = "";
                    String foundString = "";

                    ConferenceBridgeItem item = new ConferenceBridgeItem();

                    for(Node next = fstNode.getFirstChild(); next != null; next = next.getNextSibling()) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            String nodename = next.getNodeName();
                            String nodevalue = "";
                            Node child = next.getFirstChild();
                            if (child != null) {
                                nodevalue = child.getNodeValue();
                            }
                            if (nodename.equals("bridgename")) {
                                item.setBridgeName(nodevalue);
                            } else if (nodename.equals("bridgecontact")) {
                                item.setBridgeContact(nodevalue);
                            } else if (nodename.equals("extensionname")) {
                                item.setExtensionName(nodevalue);
                            } else if (nodename.equals("ownername")) {
                                item.setOwnerName(nodevalue);
                            } else if (nodename.equals("ownerid")) {
                                item.setOwnerId(nodevalue);
                            } else if (nodename.equals("mboxserver")) {
                                item.setMailboxServer(nodevalue);
                            }
                        }
                    }

                    LOG.debug("Adding bridge: "  + item.getBridgeName());
                    m_confNameMap.put(item.getBridgeName(), item);
                    m_confs.add(item);
                }
            }

        return nodeString;
    }

    public static ConferenceBridgeItem getConferenceBridgeItem(String confName) {
        try {
            update(true);
        } catch (Exception ex) {
            return null;
        }
        LOG.debug("Looking for bridge: "  + confName);
        ConferenceBridgeItem item = m_confNameMap.get(confName);
        if (item != null) {
            LOG.debug("Found bridge: "  + item.getBridgeName());
            LOG.debug("Found owner: "  + item.getOwnerName());
        }
        return item;
    }

}
