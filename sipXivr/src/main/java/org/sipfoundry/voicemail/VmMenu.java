/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.voicemail;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.IvrChoice;

/**
 * Customize sipxivr.Menu with VoiceMail specific options
 */
public class VmMenu extends org.sipfoundry.sipxivr.Menu {
    private VoiceMail m_vm;
    private boolean m_speakCanceled;
    private boolean m_operatorOn0;
    
    public VmMenu(VoiceMail vm) {
        super(vm.getLoc());
        m_vm = vm ;
        m_speakCanceled = true;
        m_operatorOn0 = true;
        setInvalidMax(vm.getConfig().getInvalidResponseCount());
        setTimeoutMax(vm.getConfig().getNoInputCount());
        setInitialTimeout(vm.getConfig().getInitialTimeout());
        setInterDigitTimeout(vm.getConfig().getInterDigitTimeout());
        setExtraDigitTimeout(vm.getConfig().getExtraDigitTimeout());
        setErrorPl(vm.getLoc().getPromptList("invalid_try_again"));
    }

    @Override
    public IvrChoice collectDigit(PromptList menuPl, String validDigits) {
        setInterDigitTimeout(0);
        setExtraDigitTimeout(0);
        
        return checkChoice(super.collectDigit(menuPl, validDigits+"0"));
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
            m_vm.failure();
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
                    m_vm.operator();
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
