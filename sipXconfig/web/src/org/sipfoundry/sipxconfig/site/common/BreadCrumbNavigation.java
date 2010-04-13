/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;

@ComponentClass(allowInformalParameters = false)
public abstract class BreadCrumbNavigation extends BaseComponent {

    @Asset("/images/breadcrumb_separator.png")
    public abstract IAsset getBreadcrumbsSeparator();

    public abstract Integer getCrumbId();

    @Parameter(required = true)
    public abstract List<BreadCrumb> getBreadCrumbs();

    public abstract BreadCrumb getBreadCrumb();

    public IPage doDirectLink(Integer id) {
        return id < getBreadCrumbs().size() ? getBreadCrumbs().get(id).getDirectLink() : null;
    }

    public boolean isLastLink() {
        return getCrumbId() >= (getBreadCrumbs().size() - 1) ? true : false;
    }

    public boolean isBreadCrumbsEnabled() {
        if (null != getBreadCrumbs() && getBreadCrumbs().size() > 1) {
            return true;
        }
        return false;
    }
}
