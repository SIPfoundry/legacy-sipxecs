/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.util.List;
import java.util.Vector;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;

public class DistributionsReader extends XmlReaderImpl<Distributions> {
    
    private Node m_root;
    
    @SuppressWarnings("unchecked")
    @Override
    public Distributions readObject(Document doc) {
        Distributions ds = new Distributions();
        m_root = doc.getRootElement();
        List<Element> lists = m_root.selectNodes("list");
        for (Element list : lists) {
            String index = list.valueOf("index");
            List<Element> destinations = list.selectNodes("destination");
            Vector<String> dests = new Vector<String>();
            for (Element destination : destinations) {
                dests.add(destination.getTextTrim());
            }
            ds.addList(index, dests);
        }
        return ds;
    }
}
