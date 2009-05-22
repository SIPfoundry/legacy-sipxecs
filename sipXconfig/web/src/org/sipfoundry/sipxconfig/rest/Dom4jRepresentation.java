/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.DOMWriter;
import org.restlet.data.MediaType;
import org.restlet.resource.DomRepresentation;

public class Dom4jRepresentation extends DomRepresentation {
    public Dom4jRepresentation(Document dom4jDocument) {
        super(MediaType.TEXT_XML, transformtoDOM(dom4jDocument));
    }

    public static org.w3c.dom.Document transformtoDOM(Document dom4jDocument) {
        try {
            DOMWriter writer = new DOMWriter();
            return writer.write(dom4jDocument);
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        }
    }
}
