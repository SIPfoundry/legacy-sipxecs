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

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.setting.Group;

@ComponentClass
public abstract class GroupMultiSelect extends BaseComponent {

    public abstract Collection<Group> getSource();

    public abstract void setGroupCandidates(Collection<Group> candidates);

    @Parameter(defaultValue = "literal:gms:groups")
    public abstract String getHtmlId();

    public void buildGroupCandidates(String groupsString) {
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(getSource(), groupsString);
        setGroupCandidates(candidates);
    }
}
