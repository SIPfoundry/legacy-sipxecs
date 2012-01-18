/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ConfigureBundlesPanel extends BaseComponent {
    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

//    @InjectObject("spring:sipxServiceManager")
//    public abstract SipxServiceManager getSipxServiceManager();
//
//    @InjectObject("spring:serviceConfigurator")
//    public abstract ServiceConfigurator getServiceConfigurator();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

//    public abstract List<SipxServiceBundle> getBundles();
//
//    public abstract void setBundles(List<SipxServiceBundle> bundles);
//
//    public abstract ServerRoleLocation getServerRoleLocation();
//
//    public abstract void setServerRoleLocation(ServerRoleLocation serverRoles);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
//        if (getBundles() == null) {
//            Location location = getLocationBean();
//            List<SipxServiceBundle> installedBundles = getSipxServiceManager().getBundlesForLocation(location);
//            setBundles(installedBundles);
//        }
//        ServerRoleLocation role = getServerRoleLocation();
//        if (role == null) {
//            role = getLocationBean().getServerRoles();
//            setServerRoleLocation(role);
//        }
    }

//    public IPropertySelectionModel getBundlesModel() {
//        Collection<SipxServiceBundle> allBundles = getSipxServiceManager().getBundleDefinitions();
//        Collection<SipxServiceBundle> bundles = getLocationBean().getInstallableBundles(allBundles);
//
//        ObjectSelectionModel nakedModel = new ObjectSelectionModel();
//        nakedModel.setCollection(bundles);
//        nakedModel.setLabelExpression("name");
//
//        LocalizedOptionModelDecorator model = new LocalizedOptionModelDecorator();
//        model.setMessages(getMessages());
//        model.setResourcePrefix("bundle.");
//        model.setModel(nakedModel);
//
//        return model;
//    }

    public void saveLocation() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Location location = getLocationBean();
//        List<SipxServiceBundle> bundles = getBundles();
//        Collection<SipxServiceBundle> oldBundles = getSipxServiceManager().getBundlesForLocation(location);
//        Collection<SipxServiceBundle> newBundles = getBundles();
//
//        getSipxServiceManager().setBundlesForLocation(location, bundles);
//        getServiceConfigurator().enforceRole(location);
//
//        Collection<SipxServiceBundle> modifiedBundles = disjunction(oldBundles, newBundles);
//
//        for (SipxServiceBundle bundle : modifiedBundles) {
//            if (bundle.isResetAffectLocation()) {
//                getLocationsManager().saveLocation(location);
//                break;
//            }
//        }
//
//        if (!modifiedBundles.isEmpty()) {
//            ServerRoleLocation role = getServerRoleLocation();
//            role.setModifiedBundles(modifiedBundles);
//            getLocationsManager().storeServerRoleLocation(location, role);
//        }
    }
}
