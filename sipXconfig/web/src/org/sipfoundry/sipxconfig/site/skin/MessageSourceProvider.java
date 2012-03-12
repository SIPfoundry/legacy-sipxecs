/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import org.springframework.context.MessageSource;


/**
 * To supply resource strings to UI.
 */
public interface MessageSourceProvider {

    /**
     * Example implementation:
     *
     *  1.) Put  "foo.properties" in base path of of jar in classpath
     *
     *  2.) Write MessageSourceProvider implementation that returns
     *
     *        @Override
     *        public MessageSource getMessageSource() {
     *            ResourceBundleMessageSource rb = new ResourceBundleMessageSource();
     *            rb.setBasename("foo");
     *            return rb;
     *         }
     *
     */
    public MessageSource getMessageSource();

}
