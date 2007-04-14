/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class CdrTable extends BaseComponent {
    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Parameter
    public abstract Object getSource();

    public abstract Cdr getRow();

    public ITableColumn getStartTimeColumn() {
        return TapestryUtils.createDateColumn("startTime", getMessages(),
                getExpressionEvaluator(), getPage().getLocale());
    }
}
