/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
