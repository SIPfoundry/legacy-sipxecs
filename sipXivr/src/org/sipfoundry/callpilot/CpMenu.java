/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.callpilot;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.IvrChoice;
import org.sipfoundry.voicemail.VoiceMail;

/**
 * Customize sipxivr.Menu with VoiceMail specific options
 */
public class CpMenu extends org.sipfoundry.sipxivr.Menu {
    
    public CpMenu(VoiceMail vm) {
        super(vm.getLoc());
        setInvalidMax(vm.getConfig().getInvalidResponseCount());
        setTimeoutMax(0);
        setInitialTimeout(3500);
        setInterDigitTimeout(vm.getConfig().getInterDigitTimeout());
        setExtraDigitTimeout(vm.getConfig().getExtraDigitTimeout());
    }

    @Override
    public IvrChoice collectDigit(PromptList menuPl, String validDigits) {
        setInterDigitTimeout(0);
        setExtraDigitTimeout(0);      
        
        return super.collect(menuPl, validDigits);
    }

    @Override
    public IvrChoice collectDigits(PromptList menuPl, int maxDigits) {
        return super.collectDtmf(menuPl, maxDigits);
    }

}
