/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;


import junit.framework.Test;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebForm;

public class ListBridgesTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListBridgesTestUi.class);
    }

    public ListBridgesTestUi() throws Exception {
        super("ListBridges", "resetConferenceBridgeContext", "bridge");
        setHasDuplicate(false);
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "host", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "bridge" + i, "host" + i + ".com", "Description" + i
        };
    }

    protected void setAddParams(String[] names, String[] values) {
        super.setAddParams(names, values);
        // make sure that all uploads are happy and set to something
        WebForm form = getDialog().getForm();
        SiteTestHelper.initUploadFields(form, "ListBridgesTestUi");
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        return ArrayUtils.add(paramValues, 1, "Disabled");
    }
    
    public void setUp() {
        super.setUp();
    }
}
