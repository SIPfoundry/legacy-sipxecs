/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Block;

/**
 * A special Block component that contains the left-hand navigation
 * links that exist on some pages.
 *
 * Once loaded, this component finds the Border component containing it (if any),
 * and sets itself to be inserted into the proper navigation area in the layout.
 */
@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class LeftNavigation extends Block {

    @Parameter(required = false, defaultValue = "true")
    public abstract void setRenderCondition(boolean renderCondition);

    public abstract boolean getRenderCondition();

    @Override
    public void renderBody(IMarkupWriter writer, IRequestCycle cycle) {
        if (getRenderCondition()) {
            super.renderBody(writer, cycle);
        }
    }
}
