package org.sipfoundry.sipxconfig.site.region;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

public abstract class RegionSelection extends BaseComponent {

	@InjectObject("spring:regionManager")
	public abstract RegionManager getRegionManager();

	@InjectObject("spring:tapestry")
	public abstract TapestryContext getTapestry();

	public abstract void setRegionId(Integer regionId);
	
	@Parameter(required = true)
	public abstract Integer getRegionId();

	public abstract void setSelectedAction(IActionListener selectedAction);

	public abstract IActionListener getSelectedAction();

	@Override
	protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
		Region region = null;
		Integer regionId = getRegionId();
		if (regionId != null) {
			region = getRegionManager().getRegion(regionId);
			if (region != null) {
				setSelectedAction(new RegionAction(region));			
			}			
		} 
		
		if (region == null) {
			setSelectedAction(null);
		}

		super.renderComponent(writer, cycle);
		if (TapestryUtils.isRewinding(cycle, this)
				&& TapestryUtils.isValid(this)) {
			triggerAction(cycle);
		}
	}

	private void triggerAction(IRequestCycle cycle) {
		IActionListener a = getSelectedAction();
		RegionAction action;
		if (a != null) {
			if (!(a instanceof RegionAction)) {
				return;
			}

			action = (RegionAction) a;
			Region region = action.getSelectedRegion();
			if (region != null) {
				action.setId(region.getId());
			}
		} else {
			// no item is selected
			action = new RegionAction(null);
			action.setId(null);
		}
		action.actionTriggered(this, cycle);
	}

	public IPropertySelectionModel getRegionsModel() {
		List<Region> regiones = getRegionManager().getRegions();
		Collection<OptionAdapter<Region>> options = new ArrayList<OptionAdapter<Region>>();
		for (Region region : regiones) {
			RegionAction adapter = new RegionAction(region);
			options.add(adapter);
		}
		AdaptedSelectionModel regionSelectionModel = new AdaptedSelectionModel();
		regionSelectionModel.setCollection(options);
		return getTapestry().instructUserToSelect(regionSelectionModel,
				getMessages());
	}

	private class RegionAction extends RegionAdapter {
		public RegionAction(Region region) {
			super(region);
		}

		public void actionTriggered(IComponent component,
				final IRequestCycle cycle) {
			Region selected = getSelectedRegion();
			Integer id = (selected == null ? null : selected.getId()); 
			setRegionId(id);
		}

		@Override
		public Object getValue(Object option, int index) {
			return this;
		}

		@Override
		public String squeezeOption(Object option, int index) {
			return getSelectedRegion().getId().toString();
		}

		@Override
		public boolean equals(Object obj) {
			return ObjectUtils.equals(this.getSelectedRegion(),
					((RegionAction) obj).getSelectedRegion());
		}

		@Override
		public int hashCode() {
			return this.getSelectedRegion().hashCode();
		}
	}
}
