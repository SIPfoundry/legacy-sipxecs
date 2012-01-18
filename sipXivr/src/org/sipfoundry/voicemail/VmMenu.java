/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.voicemail;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.ApplicationConfiguraton;
import org.sipfoundry.sipxivr.common.IvrChoice;
import org.sipfoundry.sipxivr.common.IvrChoice.IvrChoiceReason;

/**
 * Customize sipxivr.Menu with VoiceMail specific options
 */
public class VmMenu extends org.sipfoundry.voicemail.Menu {
    private boolean m_speakCanceled;
    private boolean m_operatorOn0;
    private VmEslRequestController m_controller;
    
    public VmMenu(VmEslRequestController controller) {
        super(controller.getLocalization());
        m_speakCanceled = true;
        m_operatorOn0 = true;
        ApplicationConfiguraton config = controller.getVoicemailConfiguration();
        setInvalidMax(config.getInvalidResponseCount());
        setTimeoutMax(config.getNoInputCount());
        setInitialTimeout(config.getInitialTimeout());
        setInterDigitTimeout(config.getInterDigitTimeout());
        setExtraDigitTimeout(config.getExtraDigitTimeout());
        setErrorPl(controller.getPromptList("invalid_try_again"));
        m_controller = controller;
    }

    @Override
    public IvrChoice collectDigit(PromptList menuPl, String validDigits) {
        setInterDigitTimeout(0);
        setExtraDigitTimeout(0);
        
        return checkChoice(super.collectDigit(menuPl, validDigits+"0"));
    }

    public IvrChoice collectDigitIgnoreFailureOrTimeout(PromptList menuPl, String validDigits) {
        setInterDigitTimeout(0);
        setExtraDigitTimeout(0);
        IvrChoice choice = super.collectDigit(menuPl, validDigits+"0");
        if (choice.getIvrChoiceReason().equals(IvrChoiceReason.TIMEOUT) ||
                choice.getIvrChoiceReason().equals(IvrChoiceReason.FAILURE)) {
            return choice;
        }
        return checkChoice(choice);
    }


    @Override
    public IvrChoice collectDigits(PromptList menuPl, int maxDigits) {
        return checkChoice(super.collectDigits(menuPl, maxDigits));
    }

    /**
     * Handle the cases of failure, "0" for operator, and "*" for canceled
     * @param choice
     * @return choice, or null if handled already
     */
    private IvrChoice checkChoice(IvrChoice choice) {
        switch (choice.getIvrChoiceReason()) {
        case FAILURE:
        case TIMEOUT:
            m_controller.failure();
            return null;
        case CANCELED:
            if (m_speakCanceled) {
                // "Canceled."
                m_loc.play("canceled", "");
            }
            break;
        case SUCCESS:
            if (m_operatorOn0) {
                // "0" means transfer to operator
                if (choice.getDigits().equals("0")) {
                    m_controller.transferToOperator();
                    return null;
                }
            }
        }
        return choice;
    }

    public void setSpeakCanceled(boolean speakCanceled) {
        m_speakCanceled = speakCanceled;
    }
    
    public void setOperatorOn0(boolean operatorOn0) {
        m_operatorOn0 = operatorOn0;
    }

}
