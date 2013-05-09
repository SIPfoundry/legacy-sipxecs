/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Any;

/**
 * Put a resource string on the page, but preserve the tag name
 *
 * Foo.html <span jwcid="@common/Message" key="quick.help">This text is not rendered.</span>
 *
 * Foo.properties quick.help=This text will render instead.
 *
 * Will render <span key="quick.help">This text will render instead.</span>
 *
 * Equivalent tag <span jwcid="@Insert" element="span" value="messages:fooString"/>
 */
public abstract class Message extends Any {

    @Parameter(required = true)
    public abstract String getKey();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        boolean rewinding = cycle.isRewinding();
        String key = getKey();
        if (!rewinding) {
            writer.begin(getTemplateTagName());
            writer.attribute("key", key);

            renderInformalParameters(writer, cycle);
        }
        String msg = getPage().getMessages().getMessage(key);
        writer.print(msg);

        if (!rewinding) {
            writer.end();
        }
    }
}
