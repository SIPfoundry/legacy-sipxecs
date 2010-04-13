/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public class ConditionalSet extends SettingSet implements ConditionalSetting {
    private String m_if;
    private String m_unless;

    public String getIf() {
        return m_if;
    }

    public void setIf(String if1) {
        m_if = if1;
    }

    public String getUnless() {
        return m_unless;
    }

    public void setUnless(String unless) {
        m_unless = unless;
    }

    /**
     * Handle if/unless expression with custom evaluator
     */
    public Setting evaluate(SettingExpressionEvaluator evaluator) {
        Setting settings = new ExpressionEvaluatorRunner().evaluate(evaluator, this);
        return settings;
    }

    /**
     * Not re-entrant or multi-threaded. (Use and throw away)
     */
    static class ExpressionEvaluatorRunner implements SettingVisitor {
        private SettingExpressionEvaluator m_evaluator;
        private SettingSet m_copy;

        public Setting evaluate(SettingExpressionEvaluator evaluator, ConditionalSet settings) {
            m_evaluator = evaluator;
            m_copy = (SettingSet) settings.shallowCopy();
            for (Setting s : settings.getValues()) {
                s.acceptVisitor(this);
            }
            return m_copy;
        }

        public void visitSetting(Setting setting) {
            if (setting instanceof ConditionalSetting) {
                ConditionalSetting conditional = (ConditionalSetting) setting;
                if (isTrue(conditional)) {
                    Setting copy = setting.copy();
                    addCopy(copy);
                }
            } else {
                addCopy(setting.copy());
            }
        }

        public boolean visitSettingArray(SettingArray array) {
            addCopy(array.copy());
            return true;
        }

        public boolean visitSettingGroup(SettingSet set) {
            ConditionalSet conditional = (ConditionalSet) set;
            boolean isTrue = isTrue(conditional);
            if (isTrue) {
                Setting copy = conditional.shallowCopy();
                addCopy(copy);
            }
            return isTrue;
        }

        private void addCopy(Setting copy) {
            String parentPath = copy.getParent().getPath();
            Setting parentCopy = m_copy.getSetting(parentPath);
            if (parentCopy != null) {
                parentCopy.addSetting(copy);
            }
        }

        private boolean isTrue(ConditionalSetting setting) {
            boolean isTrue = true;

            String ifExpression = setting.getIf();
            if (ifExpression != null) {
                isTrue = m_evaluator.isExpressionTrue(ifExpression, setting);
            }
            String unlessExpression = setting.getUnless();
            if (isTrue && unlessExpression != null) {
                isTrue = !m_evaluator.isExpressionTrue(unlessExpression, setting);
            }

            return isTrue;
        }
    }
}
