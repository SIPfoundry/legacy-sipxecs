package org.sipfoundry.voicemail;

import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.sipxivr.PromptList;

/**
 * Customize sipxivr.Menu with VoiceMail specific options
 */
public class VmMenu extends org.sipfoundry.sipxivr.Menu {
    private VoiceMail m_vm;
    private boolean m_speakCanceled;
    
    public VmMenu(VoiceMail vm) {
        super(vm.getLoc());
        m_vm = vm ;
        m_speakCanceled = true;
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
            // "0" means transfer to operator
            if (choice.getDigits().equals("0")) {
                m_vm.operator();
                return null;
            }
        }
        return choice;
    }

    public void setSpeakCanceled(boolean speakCanceled) {
        m_speakCanceled = speakCanceled;
    }
    

}
