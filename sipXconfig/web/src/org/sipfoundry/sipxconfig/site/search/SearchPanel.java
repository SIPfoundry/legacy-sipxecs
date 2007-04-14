/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.search;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SearchPanel extends BaseComponent {

    public abstract String getQuery();

    public IPage search(IRequestCycle cycle) {
        String query = getQuery();
        if (StringUtils.isEmpty(query)) {
            return null;
        }
        SearchPage page = (SearchPage) cycle.getPage(SearchPage.PAGE);
        page.setQuery(query);
        return page;
    }
}
