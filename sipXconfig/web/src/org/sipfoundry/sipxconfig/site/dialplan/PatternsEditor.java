/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.components.LenSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * PatternsEditor - list of pattersn with ability to edit, remove and delete
 */
public abstract class PatternsEditor extends BaseComponent {

    @Bean(lifecycle = Lifecycle.PAGE, initializer = "min=0,max=18")
    public abstract LenSelectionModel getDigitsOnlyModel();

    public abstract boolean getAddPattern();
    public abstract void setAddPattern(boolean addPattern);

    public abstract int getIndexToRemove();
    public abstract void setIndexToRemove(int index);

    @Parameter(required = true)
    public abstract List<DialPattern> getPatterns();

    public abstract int getIndex();

    public abstract int getSize();
    public abstract void setSize(int size);

    public boolean isLast() {
        List<DialPattern> patterns = getPatterns();
        return getIndex() == patterns.size() - 1;
    }

    static final void setCollectionSize(List<DialPattern> patterns, int newSize) {
        while (newSize < patterns.size()) {
            patterns.remove(patterns.size() - 1);
        }
        while (newSize > patterns.size()) {
            patterns.add(new DialPattern());
        }
    }

    public void sizeChanged() {
        List<DialPattern> patterns = getPatterns();
        setCollectionSize(patterns, getSize());
    }

    /**
     * Process pattern adds/deletes.
     */
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        List<DialPattern> patterns = getPatterns();
        if (TapestryUtils.isRewinding(cycle, this)) {
            // reset components before rewind
            setIndexToRemove(-1);
            setAddPattern(false);
        } else {
            setSize(patterns.size());
        }
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(cycle, this)) {
            if (getAddPattern()) {
                patterns.add(new DialPattern());
            }
            int indexToRemove = getIndexToRemove();
            if (indexToRemove >= 0) {
                patterns.remove(indexToRemove);
            }
        }
    }
}
