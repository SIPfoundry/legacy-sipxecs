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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * Component that allows user to select from existing set of assets (prompts etc.) or upload a new
 * asset.
 */
@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class AssetSelectorSingle extends AssetSelector {

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (TapestryUtils.isRewinding(cycle, this) && getAssetExists()) {
            // reset the value of the asset if associated file does not exist
            File assetFile = new File(getAssetDir(), getAsset());
            if (!assetFile.exists()) {
                setAsset(null);
            }
        }
    }
}
