package org.sipfoundry.sipxconfig.site.region;

import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.region.Region;

public abstract class RegionAdapter implements OptionAdapter, IActionListener {

    private Integer m_id;

    private Region m_region;

    public RegionAdapter(Region region) {
        m_region = region;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public Region getSelectedRegion() {
        return m_region;
    }

    public void setSelectedRegion(Region region) {
        m_region = region;
    }

    public Object getValue(Object option, int index) {
        return this;
    }

    public String getLabel(Object option, int index) {
        return m_region.getName();
    }

    public String squeezeOption(Object option, int index) {
        return m_region.getId().toString();
    }

    public String getMethodName() {
        return null;
    }
}
