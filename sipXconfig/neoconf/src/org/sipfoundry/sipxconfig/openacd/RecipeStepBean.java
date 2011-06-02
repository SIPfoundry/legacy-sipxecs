/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.io.Serializable;

public class RecipeStepBean implements Serializable {
    private OpenAcdRecipeStep m_recipeStep;

    private int m_position;

    public RecipeStepBean(OpenAcdRecipeStep recipeStep, int position) {
        m_recipeStep = recipeStep;
        m_position = position;
    }

    public OpenAcdRecipeStep getRecipeStep() {
        return m_recipeStep;
    }

    public void setRecipeStep(OpenAcdRecipeStep recipeStep) {
        m_recipeStep = recipeStep;
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }
}
