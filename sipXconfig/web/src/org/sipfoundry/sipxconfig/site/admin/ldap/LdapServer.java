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
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.bulk.ldap.Schema;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class LdapServer extends BaseComponent implements PageBeginRenderListener {
    public static final String CONNECTION_STAGE = "connection";
    public static final String OBJECT_CLASSES_STAGE = "objectClasses";
    public static final String ATTRS_STAGE = "attrs";

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

    @Parameter(required = true)
    public abstract int getCurrentConnectionId();
    public abstract void setCurrentConnectionId(int currentConnectionId);

    @Parameter(required = true)
    public abstract boolean isAddMode();
    public abstract void setAddMode(boolean addMode);

    @Override
    public void pageBeginRender(PageEvent event_) {
        LdapManager ldapManager = getLdapManager();

        if (getConnectionParams() == null || getConnectionParams().getId() != getCurrentConnectionId()) {
            setConnectionParams(ldapManager.getConnectionParams(getCurrentConnectionId()));
        }

        if (getAttrMap() == null || getAttrMap().getId() != getCurrentConnectionId()) {
            setAttrMap(ldapManager.getAttrMap(getCurrentConnectionId()));
        }

        if (getStage() == null) {
            setStage(CONNECTION_STAGE);
        }
    }

    public void addLdapConnection() {
        setAddMode(true);
        setCurrentConnectionId(-1);
    }

    public void removeLdapConnection() {
        setAddMode(false);
        getLdapManager().removeConnectionParams(getCurrentConnectionId());
        List<LdapConnectionParams> allParams = getLdapManager().getAllConnectionParams();
        if (allParams == null || allParams.isEmpty()) {
            setCurrentConnectionId(-1);
            LdapSystemSettings settings = getLdapManager().getSystemSettings();
            settings.setConfigured(false);
            settings.setEnableOpenfireConfiguration(false);
            getLdapManager().saveSystemSettings(settings);
        } else {
            setCurrentConnectionId(allParams.get(0).getId());
        }

    }

    public void applyConnectionParams() {
        applyConnectionParamsWithStage(OBJECT_CLASSES_STAGE);
    }

    public void applyConnectionParamsOnFirstStep() {
        applyConnectionParamsWithStage(CONNECTION_STAGE);
    }

    private void applyConnectionParamsWithStage(String stage) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(getPage());
        LdapConnectionParams connectionParams = getConnectionParams();
        AttrMap attrMap = getAttrMap();
        LdapManager ldapManager = getLdapManager();
        // check if we can connect to LDAP - throws user exception if there are any problems
        // Cannot avoid try/catch here - the exception is displayed on a parent page for this tab
        try {
            ldapManager.verify(connectionParams, attrMap);
            // save new connection params
            ldapManager.setConnectionParams(connectionParams);
            attrMap.setUniqueId(connectionParams.getId());
            ldapManager.setAttrMap(attrMap);
            Schema schema = ldapManager.getSchema(attrMap.getSubschemaSubentry(), connectionParams);
            setSchema(schema);
            setStage(stage);
            setCurrentConnectionId(connectionParams.getId());
            setAddMode(false);
        } catch (UserException e) {
            validator.record(e, getMessages());
        }
    }

    public void cancel() {
        setStage(CONNECTION_STAGE);
    }

    public void applyObjectClassesSelection() {
        Schema schema = getSchema();
        AttrMap attrMap = getAttrMap();
        String[] attributesPool = schema.getAttributesPool(attrMap.getSelectedObjectClasses());
        setSelectedAttributes(attributesPool);

        setStage(ATTRS_STAGE);
    }

    public IPage applyAttrMap(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return null;
        }
        LdapManager ldapManager = getLdapManager();
        ldapManager.setAttrMap(getAttrMap());
        // write openfire.xml file /  mark sipxopenfire service for restart
//        ldapManager.replicateOpenfireConfig();
        // send us to import preview
        LdapImportPreview ldapImportPreview = (LdapImportPreview) cycle.getPage(LdapImportPreview.PAGE);
        ldapImportPreview.setCurrentConnectionId(getCurrentConnectionId());
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
