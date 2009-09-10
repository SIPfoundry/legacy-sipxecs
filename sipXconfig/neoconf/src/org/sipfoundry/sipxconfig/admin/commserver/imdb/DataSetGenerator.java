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

import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;

public abstract class DataSetGenerator {
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected CoreContext getCoreContext() {
        return m_coreContext;
    }

    /**
     * @return SIP domain - if not set uses m_coreContext to retrieve domain
     */
    protected String getSipDomain() {
        return m_coreContext.getDomainName();
    }

    public List<Map<String, String>> generate() {
        List<Map<String, String>> items = new LinkedList<Map<String, String>>();
        addItems(items);
        return items;
    }

    /**
     * Creates empty item.
     *
     * XML/RPC client that we use insist on using Hashtable, but there is no reason to pollute the
     * code everywhere.
     *
     * @return newly created empty item
     */
    protected final Map<String, String> addItem(List<Map<String, String>> items) {
        Map<String, String> item = new Hashtable<String, String>();
        items.add(item);
        return item;
    }

    /**
     * This is for testing only.
     *
     * @return XML representation of dataset
     */
    @Deprecated
    public Document generateXml() {
        List<Map<String, String>> items = generate();

        DocumentFactory factory = DocumentFactory.getInstance();
        Document document = factory.createDocument();
        Element itemsEl = document.addElement("items");
        itemsEl.addAttribute("type", getType().getName());

        for (Map<String, String> item : items) {
            Element itemEl = itemsEl.addElement("item");
            for (Map.Entry<String, String> entry : item.entrySet()) {
                itemEl.addElement(entry.getKey()).setText(entry.getValue());
            }
        }
        return document;
    }

    protected abstract DataSet getType();

    protected abstract void addItems(List<Map<String, String>> items);
}
