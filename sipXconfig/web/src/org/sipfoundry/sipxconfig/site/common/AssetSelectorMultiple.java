/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.File;

import org.apache.commons.io.filefilter.FileFileFilter;
import org.apache.commons.lang.ArrayUtils;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;

/**
 * Component that allows user to select from existing set of assets (prompts etc.) or upload a new
 * asset.
 */
@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class AssetSelectorMultiple extends AssetSelector {
    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getSubmitOnChange();

    public IPropertySelectionModel getAssetSelectionModel() {
        File assetDir = new File(getAssetDir());
        // make sure it exists
        assetDir.mkdirs();
        // list only files
        String[] assets = assetDir.list(FileFileFilter.FILE);
        if (assets == null) {
            assets = ArrayUtils.EMPTY_STRING_ARRAY;
        }
        return new StringPropertySelectionModel(assets);
    }
}
