/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;

import java.util.HashMap;
import java.util.Map;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.io.IOUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public final class ContactSynchronizer {
    private static final String SIP_ADDRESS_ATTR_NAME = "sip_address";
    private static final String ENTRY_NAME = "entry";
    private static Map<String, HashMap<String, ContactSynchronizer>> s_phoneBook =
        new HashMap<String, HashMap<String, ContactSynchronizer>>();
    private String m_userContactListFileName;
    private String m_phoneBookFileName;


    private ContactSynchronizer(String phoneBookFileName, String userContactListFileName) {
        super();
        this.m_userContactListFileName = userContactListFileName;
        this.m_phoneBookFileName = phoneBookFileName;
    }

    public static synchronized ContactSynchronizer getInstance(String phonebookName, String contactListName) {
        ContactSynchronizer instance = null;
        HashMap<String, ContactSynchronizer> cmap = s_phoneBook.get(phonebookName);
        if (cmap != null) {
            instance = (ContactSynchronizer) cmap.get(contactListName);
            if (instance != null) {
                return instance;
            } else {
                instance = new ContactSynchronizer(phonebookName, contactListName);
                cmap.put(contactListName, instance);

                return instance;
            }

        } else {
            Map<String, ContactSynchronizer> contactLists = new HashMap<String, ContactSynchronizer>();
            instance = new ContactSynchronizer(phonebookName, contactListName);

            contactLists.put(contactListName, instance);
            s_phoneBook.put(phonebookName, (HashMap<String, ContactSynchronizer>) contactLists);

            return instance;
        }

    }


    /**
     * @param m_phoneBookFileName
     * @param m_userContactListFileName
     */
    public synchronized void synChronize() {

        try {
            File phoneBookFile = new File(m_phoneBookFileName);
            File userContactListFile = new File(m_userContactListFileName);

            if (!phoneBookFile.exists()) {
                return;
            }

            if (!userContactListFile.exists()) {
                duplicatePhoneBook();
                return;
            }

            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document phoneBookDocument = dBuilder.parse(phoneBookFile);
            Document contactListDocument = dBuilder.parse(userContactListFile);

            phoneBookDocument.getDocumentElement().normalize();
            contactListDocument.getDocumentElement().normalize();

            NodeList contactlist = contactListDocument.getElementsByTagName(ENTRY_NAME);

            mergeAddressEntries(phoneBookDocument, contactlist);

            TransformerFactory transformerFactory = TransformerFactory.newInstance();
            Transformer transformer = transformerFactory.newTransformer();
            DOMSource source = new DOMSource(phoneBookDocument);

            userContactListFile.delete();

            PrintStream outstream = new PrintStream(new FileOutputStream(m_userContactListFileName));
            StreamResult result = new StreamResult(outstream);
            transformer.transform(source, result);

            outstream.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void duplicatePhoneBook() throws IOException {
        InputStream in = new FileInputStream(m_phoneBookFileName);
        OutputStream out = new FileOutputStream(m_userContactListFileName);

        IOUtils.copy(in, out);
    }

    /**
     * @param phoneBookDocument
     * @param contactList
     */
    private void mergeAddressEntries(Document phoneBookDocument, NodeList contactList) {
        Element root = phoneBookDocument.getDocumentElement();
        Element listElement = (Element) root.getElementsByTagName("list").item(0);
        NodeList theList = listElement.getElementsByTagName(ENTRY_NAME);

        for (int i = 0; i < contactList.getLength(); i++) {
            Node nNode = contactList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element eElement = (Element) nNode;
                if (!isDuplicated(eElement, theList)) {
                    Node a = phoneBookDocument.importNode(nNode, true);
                    listElement.insertBefore(a, theList.item(contactList.getLength() - 1));
                }
            }

        }

    }

    /**
     * @param element
     * @param nlist
     * @return
     */
    private boolean isDuplicated(Element element, NodeList nlist) {

        String theURI = getSipAddress(element);

        if (theURI == null) {
            return false;
        }

        for (int i = 0; i < nlist.getLength(); i++) {
            Node nNode = nlist.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element eElement = (Element) nNode;
                String uri = getSipAddress(eElement);
                if (uri == null) {
                    return false;
                }

                if (theURI.equals(uri)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @param element
     * @return
     */
    private String getSipAddress(Element element) {

        if (element.getTagName().equals(ENTRY_NAME)) {
            NodeList nList = element.getChildNodes();

            for (int i = 0; i < nList.getLength(); i++) {
                Node nNode = nList.item(i);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element propElement = (Element) nList.item(i);
                    String name = propElement.getAttribute("name");
                    String value = propElement.getAttribute("value");

                    if ((name != null) && (name.equals(SIP_ADDRESS_ATTR_NAME))) {
                        return value;
                    }
                }

            }
        }

        return null;
    }
}
