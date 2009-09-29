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

import java.util.ArrayList;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.SetNestedPropertiesRule;

/**
 * Class for parsing IMDB xml
 */
public final class ImdbXmlHelper {
    public static final String PATTERN = "items/item";

    private ImdbXmlHelper() {
        // utility class - no instantiation
    }

    public static Digester configureDigester(Class itemClass) {
        Digester digester = new Digester();
        digester.setValidating(false);
        digester.setNamespaceAware(false);

        digester.addObjectCreate("items", ArrayList.class);

        digester.addObjectCreate(ImdbXmlHelper.PATTERN, itemClass);
        SetNestedPropertiesRule rule = new SetNestedPropertiesRule();
        // ignore all properties that we are not interested in
        rule.setAllowUnknownChildElements(true);
        digester.addRule(ImdbXmlHelper.PATTERN, rule);
        digester.addSetNext(ImdbXmlHelper.PATTERN, "add");

        return digester;
    }
}
