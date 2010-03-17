/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.validator.Max;
import org.apache.tapestry.form.validator.MaxLength;
import org.apache.tapestry.form.validator.Min;
import org.apache.tapestry.form.validator.MinLength;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.form.validator.Required;
import org.apache.tapestry.form.validator.Validator;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.setting.CustomSettingMessages;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.HostnameSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.IpAddrSetting;
import org.sipfoundry.sipxconfig.setting.type.PhonePadPinSetting;
import org.sipfoundry.sipxconfig.setting.type.RealSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;
import org.springframework.context.MessageSource;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class SettingEditor extends BaseComponent {

    private static final String LISTEN_ON_CHANGE = "ListenOnChange";

    @Parameter(required = true)
    public abstract Setting getSetting();

    public abstract void setSetting(Setting setting);

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    /**
     * Should be called 'enforceRequired'. If it is set to false the 'required' constraints are
     * not enforced. It is used when editing group settings since groups do not have to provide
     * settings values even it the settings has required flag.
     */
    @Parameter(required = true)
    public abstract boolean isRequiredEnabled();

    @Parameter(defaultValue = "true")
    public abstract boolean isEnabled();

    /**
     * Set to false to render only setting widget without label, default value and description.
     */
    @Parameter(defaultValue = "true")
    public abstract boolean isDecorated();

    /**
     * Spring MessageSource interface based on resource bundle with translations for the model.
     */
    @Parameter
    public abstract MessageSource getMessageSource();

    public boolean isDisabled() {
        return !isEnabled();
    }

    /**
     * @return id of the widget that is used to edit setting value, it's different than the ID of
     *         the SettingEditorComponent
     */
    public String getWidgetId() {
        return "setting:" + getSetting().getName();
    }

    /**
     * @return block that contains HTML for the widget used to edit setting value
     */
    public IComponent getWidgetBlock() {
        SettingType type = getSetting().getType();
        String typeName = type.getName();
        if (type instanceof EnumSetting && ((EnumSetting) type).isListenOnChange()) {
            typeName += LISTEN_ON_CHANGE;
        }
        String blockName = typeName + "Block";
        return getComponent(blockName);
    }

    /**
     * @return list of Validator objects
     */
    public List getValidatorList() {
        SettingType type = getSetting().getType();
        // see XCF-1726 'required' constraint *is* enforced (even if isRequiredEnabled is false)
        // for all settings that have non empty default values
        boolean hasDefault = StringUtils.isNotEmpty(getSetting().getDefaultValue());
        return validatorListForType(type, hasDefault || isRequiredEnabled());
    }

    static List validatorListForType(SettingType type, boolean enforceRequired) {
        List<Validator> validators = new ArrayList<Validator>();
        if (type.isRequired() && enforceRequired) {
            validators.add(new Required());
        }
        if (type instanceof IntegerSetting) {
            IntegerSetting integerType = (IntegerSetting) type;
            Min min = new Min();
            min.setMin(integerType.getMin());
            validators.add(min);
            Max max = new Max();
            max.setMax(integerType.getMax());
            validators.add(max);
        }
        if (type instanceof RealSetting) {
            RealSetting realType = (RealSetting) type;
            Min min = new Min();
            min.setMin(realType.getMin());
            validators.add(min);
            Max max = new Max();
            max.setMax(realType.getMax());
            validators.add(max);
        }
        if (type instanceof StringSetting) {
            StringSetting stringType = (StringSetting) type;
            MaxLength maxLen = new MaxLength();
            maxLen.setMaxLength(stringType.getMaxLen());
            validators.add(maxLen);
            int minLen = stringType.getMinLen();
            if (minLen > 0) {
                MinLength minLenValid = new MinLength();
                minLenValid.setMinLength(minLen);
                validators.add(minLenValid);
            }
            String patternString = stringType.getPattern();
            if (StringUtils.isNotEmpty(patternString)) {
                Pattern pattern = new Pattern();
                pattern.setPattern(patternString);

                // /////////////////////////////////////////////////////////////////
                // Settings with specific string pattern requirements might lead to
                // error messages with complicated regular expressions in them.
                // We want to display a custom message for these setting types.
                // (e.g IpAddrSetting and HostnameSetting).
                // /////////////////////////////////////////////////////////////////

                if (type instanceof HostnameSetting) {
                    String customMessage = CustomSettingMessages.getMessagePattern(
                            CustomSettingMessages.INVALID_HOSTNAME_PATTERN, Locale.getDefault());
                    pattern.setMessage(customMessage);
                } else if (type instanceof IpAddrSetting) {
                    String customMessage = CustomSettingMessages.getMessagePattern(
                            CustomSettingMessages.INVALID_IPADDR_PATTERN, Locale.getDefault());
                    pattern.setMessage(customMessage);
                } else if (type instanceof PhonePadPinSetting) {
                    String customMessage = CustomSettingMessages.getMessagePattern(
                            CustomSettingMessages.INVALID_PHONEPADPIN_PATTERN, Locale.getDefault());
                    pattern.setMessage(customMessage);
                }

                validators.add(pattern);
            }
        }
        return validators;
    }

    /**
     * In order to make FieldLabel happy we need IFormComponent instance. In most cases this is
     * actually the widget directly passed to setting editor. However in some cases our widget is
     * a collection of components, and there is no easy way of extracting a usable IFormElement. I
     * tried searching for IFormComponent among component children: this does not work since there
     * is no guarantee that found component will be actually renderer (it can be inside of if
     * block)
     */
    /**
     * @return IFormComponent or null if labeled component is not IFormComponent
     */
    public IFormComponent getFormComponent() {
        SettingType type = getSetting().getType();
        String componentName = type.getName();
        if (type instanceof EnumSetting) {
            EnumSetting enumType = (EnumSetting) type;
            if (enumType.isListenOnChange()) {
                componentName += LISTEN_ON_CHANGE;
            }
        }
        if (type instanceof FileSetting) {
            FileSetting fsType = (FileSetting) type;
            if (fsType.isVariable()) {
                componentName += "Single";
            } else {
                componentName += "Multiple";
            }
        }
        IComponent component = getComponent(componentName + "Field");
        if (component instanceof IFormComponent) {
            return (IFormComponent) component;
        }
        return null;
    }

    public IPropertySelectionModel getEnumModel() {
        Setting setting = getSetting();
        SettingType type = setting.getType();
        if (!(type instanceof EnumSetting)) {
            return null;
        }
        EnumSetting enumType = (EnumSetting) type;
        MessageSource messageSource = getMessageSource();
        IPropertySelectionModel model = null;
        model = (messageSource != null) ? localizedModelForType(setting, enumType, messageSource, getPage()
                .getLocale()) : enumModelForType(enumType);

        if (enumType.isPromptSelect()) {
            model = getTapestry().instructUserToSelect(model, getMessages());

        }

        return model;
    }

    /**
     * Retrieve default value for current setting. This is less than ideal implementation since
     * Enum and Boolean types are treated very differently from all other types.
     */
    public String getDefaultValue() {
        Setting setting = getSetting();
        SettingType type = setting.getType();
        String defaultValue = setting.getDefaultValue();
        if (defaultValue == null) {
            // no need to localize empty default labels
            return null;
        }
        String defaultValueLabel = type.getLabel(defaultValue);
        if (type instanceof EnumSetting) {
            MessageSource messageSource = getMessageSource();
            if (messageSource != null) {
                EnumSetting enumType = (EnumSetting) type;
                String code = enumType.getLabelKey(setting, defaultValue);
                Locale locale = getPage().getLocale();
                return messageSource.getMessage(code, null, defaultValueLabel, locale);
            }

        } else if (type instanceof BooleanSetting) {
            return LocalizationUtils.getMessage(getMessages(), defaultValueLabel, defaultValueLabel);
        }
        return defaultValueLabel;
    }

    public String getDescription() {
        Setting setting = getSetting();
        return LocalizationUtils.getSettingDescription(this, setting);
    }

    public String getLabel() {
        Setting setting = getSetting();
        return LocalizationUtils.getSettingLabel(this, setting);
    }

    static IPropertySelectionModel enumModelForType(EnumSetting enumType) {
        Map<String, String> enums = enumType.getEnums();
        return new NamedValuesSelectionModel(enums);
    }

    static IPropertySelectionModel localizedModelForType(Setting setting, EnumSetting enumType,
            MessageSource messageSource, Locale locale) {
        Map<String, String> enums = enumType.getEnums();
        int size = enums.size();
        String[] options = new String[size];
        String[] labels = new String[size];

        int i = 0;
        for (Map.Entry<String, String> entry : enums.entrySet()) {
            String key = entry.getKey();
            options[i] = key;
            String code = enumType.getLabelKey(setting, key);
            labels[i] = messageSource.getMessage(code, null, entry.getValue(), locale);
            i++;
        }
        return new NamedValuesSelectionModel(options, labels);
    }

    public boolean isModified() {
        Object val = getSetting().getValue();
        Object def = getSetting().getDefaultValue();
        return val == null ? def != null : !val.equals(def);
    }
}
