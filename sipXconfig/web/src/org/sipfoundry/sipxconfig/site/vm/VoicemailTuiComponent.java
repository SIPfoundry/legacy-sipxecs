/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.vm;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;

public abstract class VoicemailTuiComponent extends BaseComponent {
    private static final String VOICEMAIL_TUI_SETTING = "voicemail/mailbox/voicemail-tui";
    private static final String VOICEMAIL_TUI_TYPE = "voicemailTuiType.";

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @Parameter(required = true)
    public abstract Setting getSettings();

    public abstract VoicemailTuiType getVoicemailTui();

    public abstract void setVoicemailTui(VoicemailTuiType tuiSetting);

    private VoicemailTuiType[] getOptionsForVoicemailTui(String promptDir) {
        List list = new ArrayList();
        list.add(VoicemailTuiType.STANDARD);
        // Check that the voicemail stdprompts directory is available
        if (promptDir != null) {
            // Check if the optional scs-callpilot-prompts package is installed
            String cpPromptDir = promptDir + "/cpui";
            if ((new File(cpPromptDir)).exists()) {
                list.add(VoicemailTuiType.CALLPILOT);
            }
        }
        // Add any additional voicemail prompts packages here
        return (VoicemailTuiType[]) list.toArray(new VoicemailTuiType[0]);
    }

    public IPropertySelectionModel getModel() {
        NewEnumPropertySelectionModel<VoicemailTuiType> rawModel =
            new NewEnumPropertySelectionModel<VoicemailTuiType>();
        String promptDir = getMailboxManager().getStdpromptDirectory();
        rawModel.setOptions(getOptionsForVoicemailTui(promptDir));
        IPropertySelectionModel model = new LocalizedOptionModelDecorator(rawModel, getMessages(), VOICEMAIL_TUI_TYPE);
        return model;
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (!TapestryUtils.isRewinding(cycle, this)) {
            Setting tuiSetting = getSettings().getSetting(VOICEMAIL_TUI_SETTING);
            setVoicemailTui(VoicemailTuiType.fromValue(tuiSetting.getValue()));
        }
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            getSettings().getSetting(VOICEMAIL_TUI_SETTING).setValue(getVoicemailTui().getValue());
        }
    }

}
