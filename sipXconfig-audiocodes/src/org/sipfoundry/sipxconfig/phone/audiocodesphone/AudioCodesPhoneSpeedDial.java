/*
 * Initial Version Copyright (C) 2011 AudioCodes, All Rights Reserved.
 * Licensed to the User under the LGPL license.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.audiocodesphone;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class AudioCodesPhoneSpeedDial extends ProfileContext {
    private List<Button> m_buttons;

    public AudioCodesPhoneSpeedDial(SpeedDial speedDial) {
        super(null, "audiocodesphone/mac-address-speedDial.vm");

        if (speedDial != null) {
            m_buttons = speedDial.getButtons();
        }
    }

    public Collection<Button> getSpeedDialButtons() {
        return m_buttons;
    }
}
