/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.asset.AssetFactory;

/**
 * Link that allows for downloading files from web server
 */
public abstract class DartCode extends BaseComponent {
    private static final String DART_JS = "packages/browser/dart.js";

    @Parameter(required = false, defaultValue = "literal:text/javascript")
    public abstract String getType();

    public abstract void setActualType(String type);

    @Parameter(required = true)
    public abstract String getSrc();

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestryContext();

    public String getLink() {
        IAsset asset;
        // might have to exapnd this if we add dart libraries.
        if (getSrc().endsWith(DART_JS)) {
            setActualType(getType());
            asset = getAssetFactory().createAbsoluteAsset(DART_JS, null, null);
        } else {
            setActualType("text/javascript");
            String js = getSrc().replaceAll("\\.dart$", ".js");
            String url = getPage().getLocation().getResource().getRelativeResource(js).getPath();
            String jsPath = url.replaceFirst("/WEB-INF/", "");
            asset = getAssetFactory().createAbsoluteAsset(jsPath, null, null);
        }

        // keep suffix ".dart" and dart java script will look for ".js"
        return asset.buildURL();
    }

    private AssetFactory getAssetFactory() {
        return getTapestryContext().getHivemindContext().getClasspathAssetFactory();
    }
}
