/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.SelectMap;

/**
 * A table component that contains the dialing rules that use a particular gateway.
 */
public abstract class DialRulesTable extends BaseComponent implements PageBeginRenderListener {

    /**
     * Gets the dialing rules to be displayed in this table.
     */
    @Parameter(required = true)
    public abstract List<DialingRule> getDialingRules();

    /**
     * Gets the table selections.
     */
    public abstract SelectMap getSelections();

    /**
     * Sets the table selections.
     * @param selected the table selections
     */
    public abstract void setSelections(SelectMap selected);

    /**
     * Initializes the table selection model before the page begins rendering.
     */
    public void pageBeginRender(PageEvent event) {
        SelectMap selections = getSelections();
        if (selections == null) {
            setSelections(new SelectMap());
        }
    }

}
