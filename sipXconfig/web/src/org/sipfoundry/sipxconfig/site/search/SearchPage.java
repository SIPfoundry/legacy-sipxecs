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

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IExternalPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.search.BeanAdaptor;
import org.sipfoundry.sipxconfig.search.IndexManager;
import org.sipfoundry.sipxconfig.search.SearchManager;

public abstract class SearchPage extends SipxBasePage implements IExternalPage,
        PageBeginRenderListener {
    public static final String PAGE = "search/SearchPage";

    public abstract SearchManager getSearchManager();

    public abstract String getQuery();

    public abstract void setQuery(String query);

    public abstract void setResults(Collection results);

    public abstract Collection getResults();

    public abstract EditPageProvider getEditPageProvider();

    public abstract BeanAdaptor.Identity getResultItem();

    @InjectObject("spring:indexManager")
    public abstract IndexManager getIndexManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void activateExternalPage(Object[] parameters, IRequestCycle cycle_) {
        String query = (String) TapestryUtils.assertParameter(String.class, parameters, 0);
        setQuery(query);
    }

    public void pageBeginRender(PageEvent event) {
        String query = getQuery();
        if (StringUtils.isEmpty(query)) {
            return;
        }
        if (!event.getRequestCycle().isRewinding()) {
            // do not search when rewinding
            search(query);
        }
    }

    private void search(String query) {
        List results = getSearchManager().search(query, null);
        setResults(results);
    }

    public String getFoundMsg() {
        Collection results = getResults();
        return getMessages().format("msg.found", new Integer(getFoundCount()));
    }

    public IPage activateEditPage(IRequestCycle cycle, String klass, Object id) {
        try {
            IPage page = getEditPageProvider().getPage(cycle, Class.forName(klass), id);
            return page;
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Iterate klass hierarchy untule it finds the name for the class.
     *
     * Names should be defined in SearchPage.properties file.
     *
     * @return name for the class of found item or label.default.type
     */
    public String getResultItemType() {
        Messages messages = getMessages();
        BeanAdaptor.Identity item = getResultItem();
        for (Class k = item.getBeanClass(); k != Object.class; k = k.getSuperclass()) {
            String typeName = LocalizationUtils.getMessage(messages, k.getName(), null);
            if (typeName != null) {
                return typeName;
            }
        }
        return getMessages().getMessage("label.default.type");
    }

    public int getFoundCount() {
        Collection results = getResults();
        return results != null ? results.size() : 0;
    }

    public void rebuildSearchIndex() {

        try {
            getIndexManager().indexAll();
        } catch (RuntimeException e) {
            // Just a precaution, in case we hit any bug from Lucene...
            getValidator().record(new UserException("&msg.rebuildSearchIndex.error"), getMessages());
            return;
        }

        getValidator().recordSuccess(getMessages().getMessage("msg.rebuildSearchIndex.success"));
    }
}
