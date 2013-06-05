package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class PhoneBasePage extends PageWithCallback {

	public abstract Phone getPhone();

	public abstract void setPhone(Phone phone);

	@Persist
	public abstract Integer getPhoneId();

	public abstract void setPhoneId(Integer id);

	@InjectObject(value = "spring:phoneContext")
	public abstract PhoneContext getPhoneContext();

	@Bean
	public abstract SipxValidationDelegate getValidator();

}
