/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcMarshaller;

/**
 * Hides details of low level XML/RPC that freeswitch provides.
 *
 * See: http://wiki.freeswitch.org/wiki/Freeswitch_XML-RPC
 *
 * There is really only one method and the first parameter is the name of the command. Other
 * parameters follow.
 */
public class FreeswitchApiMarshaller implements XmlRpcMarshaller {

    public String methodName(String name) {
        return "freeswitch.api";
    }

    public Object[] parameters(String name, Object... args) {
        // for now: CamelCase to lowercase
        Object[] operation = new Object[] {
            name.toLowerCase()
        };
        if (args.length == 0) {
            return ArrayUtils.add(operation, "");
        }
        return ArrayUtils.addAll(operation, args);
    }
}
