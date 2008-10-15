/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.translator.Translator;
import org.apache.tapestry.form.validator.Validator;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.components.EnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SerialNumberTranslator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.gateway.SipTrunkModel;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class GatewayForm extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract Gateway getGateway();

    @InjectObject(value = "spring:sipTrunkStandard")
    public abstract SipTrunkModel getStandardSipTrunkModel();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:itspTemplateModelSource")
    public abstract ModelSource getTemplateModelSource();

    @InjectObject(value = "spring:modelFilesContext")
    public abstract ModelFilesContext getModelFilesContext();

    public List<Validator> getSerialNumberValidators() {
        return TapestryUtils.getSerialNumberValidators(getGateway().getModel());
    }

    public Translator getSerialNumberTranslator() {
        return new SerialNumberTranslator(getGateway().getModel());
    }

    public abstract String getTestValue();
    public abstract void setTestValue(String testValue);

    @Persist
    public abstract boolean isAdvanced();

    public abstract void setAddressTransportModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getAddressTransportModel();

    @Parameter(required = true)
    public abstract void setSelectedSbcDevice(SbcDevice selectedSbcDevice);

    public abstract SbcDevice getSelectedSbcDevice();

    public ObjectSelectionModel getVersions() {
        ObjectSelectionModel versions = new ObjectSelectionModel();
        versions.setArray(getGateway().getModel().getVersions());
        versions.setLabelExpression("versionId");

        return versions;
    }

    public void pageBeginRender(PageEvent event) {
        IPropertySelectionModel model = getAddressTransportModel();
        if (model == null) {
            EnumPropertySelectionModel rawModel = new EnumPropertySelectionModel();
            rawModel.setEnumClass(Gateway.AddressTransport.class);
            model = new LocalizedOptionModelDecorator(rawModel, getMessages(), "addressTransport.");
            setAddressTransportModel(model);
        }
    }

    public boolean isSipTrunk() {
        return (getGateway() instanceof SipTrunk);
    }

    public SipTrunkModel getTemplate() {
        SipTrunkModel model = (SipTrunkModel) getGateway().getModel();
        if (getTemplateModelSource().getModels().contains(model)) {
            return model;
        }
        return null;
    }

    public void setTemplate(GatewayModel template) {
        if (template != null) {
            getGateway().setModel(template);
        }
    }

    @EventListener(targets = "template", events = "onchange")
    public void selectTemplate(IRequestCycle cycle) {
        SipTrunkModel template = getTemplate();
        if (template != null) {
            Gateway gateway = getGateway();
            gateway.setModel(template);
            Setting templateSettings =
                getModelFilesContext().loadModelFile(template.getItspTemplate(),
                        template.getTemplateLocation());

            gateway.setAddress(templateSettings.getSetting("itsp-account/proxy-domain").getValue());
        }

        cycle.getResponseBuilder().updateComponent("gateway:address");
    }

    public IPropertySelectionModel getTemplateModel() {
        ItspTemplateSelectionModel model = new ItspTemplateSelectionModel();
        model.setModelSource(getTemplateModelSource());
        model.setExtraLabel(getMessages().getMessage("none"));
        model.setExtraOption(getStandardSipTrunkModel());
        return model;
    }

    public static class ItspTemplateSelectionModel extends ExtraOptionModelDecorator {
        public void setModelSource(ModelSource modelSource) {
            ObjectSelectionModel model = new ObjectSelectionModel();
            model.setCollection(modelSource.getModels());
            model.setLabelExpression("itspName");
            setModel(model);
        }
    }
}
