/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModelSource;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.valid.RenderString;

public final class LocalizedTableRendererSource implements ITableRendererSource {
    private static final String PREFIX_SEPARATOR = ".";

    private final Messages m_messages;
    private final String m_prefix;

    public LocalizedTableRendererSource(Messages messages) {
        this(messages, null);
    }

    public LocalizedTableRendererSource(Messages messages, String prefix) {
        m_messages = messages;
        m_prefix = (prefix == null) ? StringUtils.EMPTY : prefix + PREFIX_SEPARATOR;
    }

    public IRender getRenderer(IRequestCycle objCycle_, ITableModelSource objSource_,
            ITableColumn objColumn, Object objRow) {
        SimpleTableColumn objSimpleColumn = (SimpleTableColumn) objColumn;

        Object objValue = objSimpleColumn.getColumnValue(objRow);
        if (objValue == null) {
            return new RenderString("&nbsp;", true);
        }

        String key = m_prefix + objValue;
        String strValue = m_messages.getMessage(key);

        return new RenderString(strValue);
    }
}
