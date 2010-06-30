/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.site.about.AboutContext;

@ComponentClass
public abstract class AboutDialog extends BaseComponent {

    private static final String CLOSE = "close";
    private static final String ABOUT_DIALOG = "aboutDialog";

    @Parameter
    public abstract String getLinkTitle();

    @InitialValue(value = "true")
    public abstract boolean isDialogHidden();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Asset("context:/WEB-INF/admin/AboutDialog.script")
    public abstract IAsset getAboutDialogScript();

    @InjectObject(value = "spring:aboutContext")
    public abstract AboutContext getAbout();

    public abstract void setDialogHidden(boolean dialogHidden);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        AboutContext about = getAbout();
        if (about.getAboutPage() == null) {
            about.setAboutPage(getPage());
        }
    }

    public void showDialog() {
        setDialogHidden(false);
    }

    @EventListener(events = "onclick", targets = CLOSE)
    public void closeDialog(IRequestCycle cycle) {
        setDialogHidden(true);
        cycle.getResponseBuilder().updateComponent(ABOUT_DIALOG);
    }
}
