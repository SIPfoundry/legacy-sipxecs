/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.xmlrpc;

/**
 * Use by XmlRpcClientInterceptor to translate between Java methods and XML/RPC methods. Some of
 * the XML/RPC methods have names that are invalid in Java, or used parameter list that is not
 * natural in Java. We can still use client intercpetor for such methods as long as we provide a
 * mershaller.
 */
public interface XmlRpcMarshaller {
    String methodName(String name);

    Object[] parameters(String name, Object... args);
}
