/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
 */

package org.sipfoundry.sipxconfig.phone.yealink;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.phonelog.PhoneLog;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class YealinkPhoneDefaults {
    private final DeviceDefaults m_defaults;
    private final YealinkPhone m_phone;

    public YealinkPhoneDefaults(DeviceDefaults defaults, YealinkPhone phone) {
        m_defaults = defaults;
        m_phone = phone;
    }

    @SettingEntry(paths = {
            YealinkConstants.LOCAL_TIME_SERVER1_V6X_SETTING, YealinkConstants.LOCAL_TIME_SERVER1_V7X_SETTING
            })
    public String getTimeServer1() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(paths = {
            YealinkConstants.LOCAL_TIME_SERVER2_V6X_SETTING, YealinkConstants.LOCAL_TIME_SERVER2_V7X_SETTING
            })
    public String getTimeServer2() {
        return m_defaults.getAlternateNtpServer();
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(paths = {
            YealinkConstants.LOCAL_TIME_ZONE_V6X_SETTING, YealinkConstants.LOCAL_TIME_ZONE_V7X_SETTING
            })
    public String getTimeZone() {
        Integer tz = getZone().getOffsetInHours();
        return (tz < 0 ? "-" : "+") + tz.toString();
    }

    @SettingEntry(paths = {
            YealinkConstants.SYSLOG_SERVER_V6X_SETTING, YealinkConstants.SYSLOG_SERVER_V7X_SETTING
            })
    public String getSyslogdIP() {
        AddressManager addressManager = m_defaults.getAddressManager();
        if (addressManager.getSingleAddress(PhoneLog.PHONELOG) != null) {
            return addressManager.getSingleAddress(PhoneLog.PHONELOG).getAddress();
        }
        return null;
    }

    public String getTFTPServer() {
        Address serverAddress = m_defaults.getTftpServer();
        if (null != serverAddress) {
            return serverAddress.getAddress();
        } else {
            return "";
        }
    }

    @SettingEntry(paths = {
            YealinkConstants.DNS_SERVER1_V6X_SETTING, YealinkConstants.DNS_SERVER1_V7X_SETTING
            })
    public String getNameServer1() {
        AddressManager addressManager = m_defaults.getAddressManager();
        if (addressManager.getSingleAddress(DnsManager.DNS_ADDRESS) != null) {
            return addressManager.getSingleAddress(DnsManager.DNS_ADDRESS).getAddress();
        }
        return null;
    }

    // TODO: Get Second DNS server intead of dublicate first
    @SettingEntry(paths = {
            YealinkConstants.DNS_SERVER2_V6X_SETTING, YealinkConstants.DNS_SERVER2_V7X_SETTING
            })
    public String getNameServer2() {
        AddressManager addressManager = m_defaults.getAddressManager();
        if (addressManager.getSingleAddress(DnsManager.DNS_ADDRESS) != null) {
            return addressManager.getSingleAddress(DnsManager.DNS_ADDRESS).getAddress();
        }
        return null;
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_0_NAME_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_0_NAME_V7X_SETTING
            })
    public String getPhonebook0Name() {
        return getPhonebookName(0);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_1_NAME_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_1_NAME_V7X_SETTING
            })
    public String getPhonebook1Name() {
        return getPhonebookName(1);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_2_NAME_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_2_NAME_V7X_SETTING
            })
    public String getPhonebook2Name() {
        return getPhonebookName(2);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_3_NAME_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_3_NAME_V7X_SETTING
            })
    public String getPhonebook3Name() {
        return getPhonebookName(3);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_4_NAME_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_4_NAME_V7X_SETTING
            })
    public String getPhonebook4Name() {
        return getPhonebookName(4);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_0_URL_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_0_URL_V7X_SETTING
            })
    public String getPhonebook0URL() {
        return String.format("tftp://%s/%s-0-%s", getTFTPServer(), m_phone.getSerialNumber(),
                YealinkConstants.XML_CONTACT_DATA);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_1_URL_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_1_URL_V7X_SETTING
            })
    public String getPhonebook1URL() {
        return String.format("tftp://%s/%s-1-%s", getTFTPServer(), m_phone.getSerialNumber(),
                YealinkConstants.XML_CONTACT_DATA);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_2_URL_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_2_URL_V7X_SETTING
            })
    public String getPhonebook2URL() {
        return String.format("tftp://%s/%s-2-%s", getTFTPServer(), m_phone.getSerialNumber(),
                YealinkConstants.XML_CONTACT_DATA);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_3_URL_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_3_URL_V7X_SETTING
            })
    public String getPhonebook3URL() {
        return String.format("tftp://%s/%s-3-%s", getTFTPServer(), m_phone.getSerialNumber(),
                YealinkConstants.XML_CONTACT_DATA);
    }

    @SettingEntry(paths = {
            YealinkConstants.REMOTE_PHONEBOOK_4_URL_V6X_SETTING, YealinkConstants.REMOTE_PHONEBOOK_4_URL_V7X_SETTING
            })
    public String getPhonebook4URL() {
        return String.format("tftp://%s/%s-4-%s", getTFTPServer(), m_phone.getSerialNumber(),
                YealinkConstants.XML_CONTACT_DATA);
    }

    @SettingEntry(paths = {
            YealinkConstants.FIRMWARE_SERVER_ADDRESS_SETTING, YealinkConstants.XML_BROWSER_SERVER_V6X_SETTING,
            YealinkConstants.XML_BROWSER_SERVER_V7X_SETTING, YealinkConstants.ACTION_URI_LIMIT_IP_V6X_SETTING,
            YealinkConstants.ACTION_URI_LIMIT_IP_V7X_SETTING
            })
    public String getServerIP() {
        return getTFTPServer();
    }

    @SettingEntry(path = YealinkConstants.FIRMWARE_HTTP_URL_SETTING)
    public String getHTTPURL() {
        YealinkModel model = (YealinkModel) m_phone.getModel();
        return String.format("%s/%s.rom", m_defaults.getProfileRootUrl(), model.getName());
    }

    @SettingEntry(paths = {
            YealinkConstants.FIRMWARE_URL_V6X_SETTING, YealinkConstants.FIRMWARE_URL_V7X_SETTING
            })
    public String getURL() {
        YealinkModel model = (YealinkModel) m_phone.getModel();
        return String.format("tftp://%s/%s.rom", getTFTPServer(), model.getName());
    }

    @SettingEntry(paths = {
            YealinkConstants.AUTOPROVISIONING_SERVER_URL_V6X_SETTING,
            YealinkConstants.AUTOPROVISIONING_SERVER_URL_V7X_SETTING,
            })
    public String getstrServerURL() {
        return String.format("tftp://%s/", getTFTPServer());
    }

    @SettingEntry(paths = {
            YealinkConstants.AUTOPROVISIONING_SERVER_ADDRESS_V6X_SETTING
            })
    public String getServerAddress() {
        return String.format("tftp://%s", getTFTPServer());
    }

    @SettingEntry(path = YealinkConstants.FIRMWARE_NAME_SETTING)
    public String getFirmwareName() {
        YealinkModel model = (YealinkModel) m_phone.getModel();
        return model.getName() + ".rom";
    }

    @SettingEntry(paths = {
            YealinkConstants.LOGO_FILE_NAME_V6X_SETTING, YealinkConstants.LOGO_FILE_NAME_V7X_SETTING
            })
    public String getLogoURL() {
        return String.format("tftp://%s/yealinkLogo132x64.dob", getTFTPServer());
    }

    private String getPhonebookName(Integer index) {
        User user = m_phone.getPrimaryUser();
        if (user != null) {
            PhonebookManager pbm = m_phone.getPhonebookManager();
            if (pbm != null) {
                Collection<Phonebook> books = pbm.getAllPhonebooksByUser(user);
                if (books != null) {
                    List list = new ArrayList(books);
                    if (index < list.size()) {
                        Phonebook pb0 = (Phonebook) list.get(index);
                        if (pb0 != null) {
                            if (pb0.getShowOnPhone()) {
                                String pbName = pb0.getName();
                                if (pbName != null) {
                                    return pbName;
                                }
                            }
                        }
                    }
                }
            }
        }
        return null;
    }

    // LDAP Defaults
    private LdapConnectionParams getLdapConnectionParams() {
        List<LdapConnectionParams> allParams = m_phone.getLdapManager().getAllConnectionParams();
        if (allParams.size() > 0) {
            return allParams.get(0);
        } else {
            return null;
        }
    }

    @SettingEntry(path = YealinkConstants.LDAP_ENABLE)
    public boolean getLDAPEnable() {
        return (m_phone.getLdapManager().getSystemSettings().isConfigured() && (null != getLdapConnectionParams()));
    }

    @SettingEntry(path = YealinkConstants.LDAP_HOST)
    public String getLDAPHost() {
        LdapConnectionParams p = getLdapConnectionParams();
        if (null != p) {
            return p.getHost();
        } else {
            return StringUtils.EMPTY;
        }
    }

    @SettingEntry(path = YealinkConstants.LDAP_PORT)
    public String getLDAPPort() {
        LdapConnectionParams p = getLdapConnectionParams();
        if (null != p) {
            return p.getPortToUse().toString();
        } else {
            return StringUtils.EMPTY;
        }
    }

    @SettingEntry(path = YealinkConstants.LDAP_USER)
    public String getLDAPUser() {
        LdapConnectionParams p = getLdapConnectionParams();
        if (null != p) {
            return p.getPrincipal();
        } else {
            return StringUtils.EMPTY;
        }
    }

    @SettingEntry(path = YealinkConstants.LDAP_PASSWORD)
    public String getLDAPPassword() {
        LdapConnectionParams p = getLdapConnectionParams();
        if (null != p) {
            return p.getSecret();
        } else {
            return StringUtils.EMPTY;
        }
    }

    @SettingEntry(path = YealinkConstants.LDAP_BASE)
    public String getLDAPBase() {
        LdapConnectionParams p = getLdapConnectionParams();
        if (null != p) {
            AttrMap attrMap = m_phone.getLdapManager().getAttrMap(p.getId());
            return attrMap.getSearchBase();
        } else {
            return StringUtils.EMPTY;
        }
    }

    @SettingEntry(path = YealinkConstants.DIRECT_CALL_PICKUP_CODE_V7X_SETTING)
    public String getDirectedCallPickupString() {
        return m_defaults.getDirectedCallPickupCode();
    }
}
