/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.Schema;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class LdapServer extends BaseComponent implements PageBeginRenderListener {

    public abstract LdapConnectionParams getConnectionParams();

    public abstract void setConnectionParams(LdapConnectionParams setConnectionParams);

    public abstract AttrMap getAttrMap();

    public abstract void setAttrMap(AttrMap attrMap);

    public abstract LdapImportManager getLdapImportManager();

    public abstract LdapManager getLdapManager();

    public abstract String getStage();

    public abstract void setStage(String stage);

    public abstract Schema getSchema();

    public abstract void setSchema(Schema schema);

    public abstract String[] getSelectedAttributes();

    public abstract void setSelectedAttributes(String[] selectedAttributes);

    public void pageBeginRender(PageEvent event_) {
        LdapManager ldapManager = getLdapManager();
        if (getConnectionParams() == null) {
            setConnectionParams(ldapManager.getConnectionParams());
        }

        if (getAttrMap() == null) {
            setAttrMap(ldapManager.getAttrMap());
        }

        if (getStage() == null) {
            setStage("connection");
        }
    }

    public void applyConnectionParams() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        LdapConnectionParams connectionParams = getConnectionParams();
        AttrMap attrMap = getAttrMap();
        LdapManager ldapManager = getLdapManager();
        // check if we can connect to LDAP - throws user exception if there are any problems
        ldapManager.verify(connectionParams, attrMap);

        // save new connection params
        ldapManager.setConnectionParams(connectionParams);
        ldapManager.setAttrMap(attrMap);

        Schema schema = ldapManager.getSchema(attrMap.getSubschemaSubentry());
        setSchema(schema);

        setStage("objectClasses");
    }

    public void applyObjectClassesSelection() {
        Schema schema = getSchema();
        AttrMap attrMap = getAttrMap();
        String[] attributesPool = schema.getAttributesPool(attrMap.getSelectedObjectClasses());
        setSelectedAttributes(attributesPool);

        setStage("attrs");
    }

    public IPage applyAttrMap(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        getLdapManager().setAttrMap(getAttrMap());
        // send us to import preview
        LdapImportPreview ldapImportPreview = (LdapImportPreview) cycle.getPage(LdapImportPreview.PAGE);
        ldapImportPreview.setExample(null);
        return ldapImportPreview;
    }

    public IPropertySelectionModel getObjectClassesSelectionModel() {
        Collection<String> objectClasses = getAttrMap().getSelectedObjectClasses();
        return new StringPropertySelectionModel(objectClasses.toArray(new String[objectClasses
                                                                                 .size()]));
    }

    public IPropertySelectionModel getAttributesSelectionModel() {
        return new StringPropertySelectionModel(getSelectedAttributes());
    }
}
