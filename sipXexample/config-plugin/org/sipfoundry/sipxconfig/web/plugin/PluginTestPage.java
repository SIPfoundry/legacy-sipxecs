package org.sipfoundry.sipxconfig.web.plugin;

import org.apache.tapestry.html.BasePage;

public abstract class PluginTestPage extends BasePage {

    public String getTestNoValue() {
        return "testValue";
    }

    public String getBorderTitle() {
        return "PagePlugin";
    }

}
