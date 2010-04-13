/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IBeanProvider;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.contrib.table.model.IAdvancedTableColumn;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.form.validator.Required;
import org.apache.tapestry.form.validator.Validator;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

/**
 * Utility method for tapestry pages and components
 */
public final class TapestryUtils {
    /**
     * Standard name for form validation delegate
     */
    public static final String VALIDATOR = "validator";

    /**
     * restrict construction
     */
    private TapestryUtils() {
        // intentionally empty
    }

    /**
     * Utility method to provide more descriptive unchecked exceptions for unmarshalling object
     * from Tapestry service Parameters.
     *
     * Please note that in most cases it is better to use listeners parameters directly.
     *
     * @throws IllegalArgumentException if parameter is not there is wrong class type
     */
    public static final Object assertParameter(Class expectedClass, Object[] params, int index) {
        if (params == null || params.length < index) {
            throw new IllegalArgumentException("Missing parameter at position " + index);
        }

        if (params[index] != null) {
            Class actualClass = params[index].getClass();
            if (!expectedClass.isAssignableFrom(actualClass)) {
                throw new IllegalArgumentException("Object of class type " + expectedClass.getName()
                        + " was expected at position " + index + " but class type was " + actualClass.getName());
            }
        }

        return params[index];
    }

    /**
     * Specialized version of assertParameter. Extracts beanId (Integer) from the first (index 0)
     * parameter of the cycle service
     *
     * @param cycle current request cycle
     * @return bean id - exception is thrown if no id found
     */
    public static Integer getBeanId(IRequestCycle cycle) {
        return (Integer) assertParameter(Integer.class, cycle.getListenerParameters(), 0);
    }

    /**
     * Helper method to display standard "nice" stale link message
     *
     * @param page page on which stale link is discovered
     * @param validatorName name of the validator delegate bean used to record validation errors
     */
    public static void staleLinkDetected(AbstractPage page, String validatorName) {
        IValidationDelegate validator = (IValidationDelegate) page.getBeans().getBean(validatorName);
        validator.setFormComponent(null);
        validator.record(new ValidatorException("The page is out of date. Please refresh."));
        throw new PageRedirectException(page);
    }

    /**
     * Helper method to display standard "nice" stale link message. Use only if standard
     * "validator" name has been used.
     *
     * @param page page on which stale link is discovered
     */
    public static void staleLinkDetected(AbstractPage page) {
        staleLinkDetected(page, VALIDATOR);
    }

    /**
     * Check if there are any validation errors on the page. Use only if standard "validator" name
     * has been used.
     *
     * Please note: this will only work properly if called after all components had a chance to
     * append register validation errors. Do not use in submit listeners other than form submit
     * listener.
     *
     * @param page
     * @return true if no errors found
     */
    public static boolean isValid(IComponent page) {
        IValidationDelegate validator = getValidator(page);
        if (validator == null) {
            return false;
        }
        return !validator.getHasErrors();
    }

    /**
     * Preferred version of isValid. Looks for form delegate instead of looking for a bean with a
     * specific name.
     */
    public static boolean isValid(IRequestCycle cycle, IComponent component) {
        IForm form = org.apache.tapestry.TapestryUtils.getForm(cycle, component);
        if (form == null) {
            return false;
        }
        IValidationDelegate validator = form.getDelegate();
        if (validator == null) {
            return false;
        }
        return !validator.getHasErrors();
    }

    /**
     * Checks is cycle is rewinding and form we are in is rewinding.
     *
     * Not sure why we need to check the cycle but this is what all standard tapestry components
     * are doing. Call this is renderComponent that needs to participate in form rewinding.
     *
     */
    public static boolean isRewinding(IRequestCycle cycle, IComponent component) {
        return cycle.isRewinding() && org.apache.tapestry.TapestryUtils.getForm(cycle, component).isRewinding();
    }

    /**
     * Retrieves the validator for the current page. Use only if standard "validator" name has
     * been used.
     *
     * Use to record errors not related to any specific component.
     *
     * @param page
     * @return validation delegate component
     */
    public static IValidationDelegate getValidator(IComponent page) {
        for (IComponent c = page; c != null; c = c.getContainer()) {
            IBeanProvider beans = c.getBeans();
            if (beans.canProvideBean(VALIDATOR)) {
                return (IValidationDelegate) c.getBeans().getBean(VALIDATOR);
            }
        }
        return null;
    }

    /**
     * send the error recorded in a validator to the affected page (the page must contain a
     * validator bean)
     *
     * @param page - affected page
     * @param validator - validator with error recorded
     */
    public static void sendValidatorError(IComponent page, ValidationDelegate validator) {
        IValidationDelegate pageValidator = TapestryUtils.getValidator(page);
        if (pageValidator != null && validator.getHasErrors() && !pageValidator.getHasErrors()) {
            pageValidator.record(new ValidatorException(validator.getFirstError().toString()));
        }
    }

    public static void recordSuccess(IComponent page, String msg) {
        IValidationDelegate delegate = getValidator(page);
        if (delegate instanceof SipxValidationDelegate) {
            SipxValidationDelegate validator = (SipxValidationDelegate) delegate;
            validator.recordSuccess(msg);
        }
    }

    /**
     * Creates column model that can be used to display dates
     *
     * @param columnName used as internal column name, user visible name (through getMessage), and
     *        OGNL expression to calculate the date
     * @param messages used to retrieve column title
     * @return newly create column model
     */
    public static ITableColumn createDateColumn(String columnName, Messages messages,
            ExpressionEvaluator expressionEvaluator, Locale locale) {
        String columnTitle = messages.getMessage(columnName);
        IAdvancedTableColumn column = new ExpressionTableColumn(columnName, columnTitle, columnName, true,
                expressionEvaluator);
        column.setValueRendererSource(new DateTableRendererSource(getDateFormat(locale)));
        return column;
    }

    public static DateFormat getDateFormat(Locale locale) {
        return DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT, locale);
    }

    /**
     * For auto completetion of space delimited fields. Collection is represented named
     *
     * @param namedItems collection of objects that implement NamedItem
     * @param currentValue what user have entered so far including
     * @return collection of Strings of possible auto completed items
     */
    public static Collection getAutoCompleteCandidates(Collection namedItems, String currentValue) {
        String targetGroup;
        String prefix;
        if (StringUtils.isBlank(currentValue)) {
            targetGroup = null;
            prefix = "";
        } else if (currentValue.endsWith(" ")) {
            targetGroup = null;
            prefix = currentValue;
        } else {
            String[] groups = splitBySpace(currentValue);
            int ignore = groups.length - 1;
            targetGroup = groups[ignore].toLowerCase();
            StringBuffer sb = new StringBuffer();
            for (int i = 0; i < ignore; i++) {
                sb.append(groups[i]).append(' ');
            }
            prefix = sb.toString();
        }

        List candidates = new ArrayList();
        for (Iterator i = namedItems.iterator(); i.hasNext();) {
            NamedObject candidate = (NamedObject) i.next();
            String candidateName = candidate.getName();
            // candidates suggestions are case insensitve, doesn't mean groups are
            // though
            if (targetGroup == null || candidateName.toLowerCase().startsWith(targetGroup)) {
                // do not list items twice
                if (prefix.indexOf(candidateName + ' ') < 0) {
                    candidates.add(prefix + candidateName);
                }
            }
        }

        return candidates;
    }

    public static PrintWriter getCsvExportWriter(WebResponse response, String filename) throws IOException {
        prepareResponse(response, filename);
        ContentType csvType = new ContentType("text/comma-separated-values");
        return response.getPrintWriter(csvType);
    }

    public static OutputStream getResponseOutputStream(WebResponse response, String filename, String contentType)
        throws IOException {
        prepareResponse(response, filename);
        ContentType ct = new ContentType(contentType);
        return response.getOutputStream(ct);
    }

    private static void prepareResponse(WebResponse response, String filename) {
        response.setHeader("Expires", "0");
        response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
        response.setHeader("Pragma", "public");
        response.setHeader("Content-Disposition", String.format("attachment; filename=\"%s\"", filename));
    }

    public static String joinBySpace(String[] array) {
        String s = null;
        if (array != null) {
            s = StringUtils.join(array, ' ');
        }
        return s;
    }

    public static String[] splitBySpace(String s) {
        String[] array = null;
        if (s != null) {
            array = s.split("\\s+");
        }
        return array;
    }

    public static List<Validator> getSerialNumberValidators(DeviceDescriptor model) {
        List<Validator> vs = new ArrayList<Validator>(2);
        vs.add(new Required());
        if (model.getHasSerialNumber()) {
            Pattern pattern = new Pattern();
            pattern.setPattern(model.getSerialNumberPattern());
            vs.add(pattern);
        }
        return vs;
    }
}
