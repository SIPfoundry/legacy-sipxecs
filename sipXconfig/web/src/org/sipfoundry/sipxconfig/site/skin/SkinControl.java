/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.asset.AssetFactory;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;

/**
 * UI control such as stylesheet assets to change skin
 */
public class SkinControl implements BeanFactoryAware {
    public static final String CONTEXT_BEAN_NAME = "skin";

    public static final String ASSET_LICENSE = "license.txt";
    static final Pattern IE_REGEX = Pattern.compile(".*(\\s+|\\()MSIE ([0-9]{1,}[\\.0-9]{0,}).*");
    private static final String ASSET_COLORS = "colors.css";
    private static final String ASSET_LAYOUT = "layout.css";
    private static final String ASSET_IE6 = "ie6-hacks.css";
    private static final String ASSET_IE7 = "ie7-hacks.css";
    private static final String ASSET_OPERA = "opera-hacks.css";

    private TapestryContext m_tapestryContext;
    // overrideable in skin
    private Map<String, String> m_assets = new HashMap();
    private String m_messageSourceBeanId;
    private MessageSource m_messageSource;
    private BeanFactory m_beanFactory;

    public SkinControl() {
        String pkg = getClass().getPackage().getName().replace('.', '/');
        // default skin resources
        m_assets.put("loginBackgroundGradient.png", pkg + "/loginBackgroundGradient.png");
        m_assets.put("loginFormBackground.png", pkg + "/loginFormBackground.png");
        m_assets.put("favicon.ico", pkg + "/favicon.ico");
        m_assets.put("go.png", pkg + "/go.png");
        m_assets.put("search.png", pkg + "/search.png");
        m_assets.put("logo.png", pkg + "/sipxconfig-logo.png");
        m_assets.put("login.jpg", pkg + "/login.jpg");
        m_assets.put("home.png", pkg + "/home.png");
        m_assets.put("home-hover.png", pkg + "/home-hover.png");
        m_assets.put("logout.png", pkg + "/logout.png");
        m_assets.put("logout-hover.png", pkg + "/logout-hover.png");
        m_assets.put("help.png", pkg + "/help.png");
        m_assets.put("help-hover.png", pkg + "/help-hover.png");
        m_assets.put("banner-background.png", pkg + "/banner-background.png");
        m_assets.put("nav-background.png", pkg + "/nav-background.png");
        m_assets.put("left-background.png", pkg + "/left-background.png");
        m_assets.put("leftNav-background.png", pkg + "/leftNav-background.png");
        m_assets.put("setting-bullet.gif", pkg + "/setting-bullet.gif");
        m_assets.put("setting-bullet-active.gif", pkg + "/setting-bullet-active.gif");
        m_assets.put(ASSET_LAYOUT, pkg + "/layout.css");
        m_assets.put(ASSET_COLORS, pkg + "/colors.css");
        m_assets.put(ASSET_IE6, pkg + "/ie6-hacks.css");
        m_assets.put(ASSET_IE7, pkg + "/ie7-hacks.css");
        m_assets.put(ASSET_OPERA, pkg + "/opera-hacks.css");
    }

    public IAsset[] getStylesheetAssets(String userAgent) {
        Matcher ie = IE_REGEX.matcher(userAgent);
        boolean isIe = ie.matches();
        List<IAsset> assets = new ArrayList<IAsset>();
        assets.add(getAsset(ASSET_COLORS));
        assets.add(getAsset(ASSET_LAYOUT));
        if (isIe) {
            String ver = ie.group(2);
            Float fver = Float.parseFloat(ver);
            if (fver < 7) {
                assets.add(getAsset(ASSET_IE6));
            } else {
                assets.add(getAsset(ASSET_IE7));
            }
        }
        if (userAgent.contains("Opera")) {
            assets.add(getAsset(ASSET_OPERA));
        }
        return assets.toArray(new IAsset[assets.size()]);
    }

    private AssetFactory getAssetFactory() {
        return m_tapestryContext.getHivemindContext().getClasspathAssetFactory();
    }

    public Map<String, String> getAssets() {
        return m_assets;
    }

    public void setAssets(Map<String, String> assets) {
        m_assets.putAll(assets);
    }

    public IAsset getAsset(String path) {
        String resourcePath = m_assets.get(path);
        if (resourcePath == null) {
            return null;
        }

        return getAssetFactory().createAbsoluteAsset(resourcePath, null, null);
    }

    public void setTapestryContext(TapestryContext tapestryContext) {
        m_tapestryContext = tapestryContext;
    }

    /**
     * To override any resource string
     */
    public void setMessageSourceBeanId(String messageSourceBeanId) {
        m_messageSourceBeanId = messageSourceBeanId;
    }

    public String getLocalizeString(String key, Locale locale, String defaultString) {
        if (m_messageSourceBeanId == null) {
            return defaultString;
        }

        if (m_messageSource == null) {
            m_messageSource = (MessageSource) m_beanFactory.getBean(m_messageSourceBeanId);
        }
        try {
            // pass null for message args, tapestry will use MessageFormat later.
            return m_messageSource.getMessage(key, null, locale);
        } catch (NoSuchMessageException extremelyLikely) {
            return defaultString;
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
