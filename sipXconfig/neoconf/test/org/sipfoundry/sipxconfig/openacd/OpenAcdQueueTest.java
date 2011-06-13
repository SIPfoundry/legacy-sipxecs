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

import java.util.Collections;
import java.util.List;

import com.mongodb.BasicDBObject;

import junit.framework.TestCase;

public class OpenAcdQueueTest extends TestCase {

    public void testGetConditionMongoObject() throws Exception {
        OpenAcdRecipeCondition condition = new OpenAcdRecipeCondition();
        condition.setCondition(OpenAcdRecipeCondition.CONDITION.TICK_INTERVAL.toString());
        condition.setRelation(OpenAcdRecipeCondition.RELATION.IS.toString());
        condition.setValueCondition("5");
        BasicDBObject mongoObject = condition.getMongoObject();
        assertEquals("ticks", mongoObject.get("condition"));
        assertEquals("=", mongoObject.get("relation"));
        assertEquals("5", mongoObject.get("value"));

        condition.setCondition(OpenAcdRecipeCondition.CONDITION.AGENTS_AVAILABLE.toString());
        condition.setRelation(OpenAcdRecipeCondition.RELATION.GREATER.toString());
        mongoObject = condition.getMongoObject();
        assertEquals("available_agents", mongoObject.get("condition"));
        assertEquals(">", mongoObject.get("relation"));

        condition.setCondition(OpenAcdRecipeCondition.CONDITION.MEDIA_TYPE.toString());
        condition.setRelation(OpenAcdRecipeCondition.RELATION.LESS.toString());
        mongoObject = condition.getMongoObject();
        assertEquals("type", mongoObject.get("condition"));
        assertEquals("<", mongoObject.get("relation"));

        condition.setCondition(OpenAcdRecipeCondition.CONDITION.CALLS_IN_QUEUE.toString());
        condition.setRelation(OpenAcdRecipeCondition.RELATION.NOT.toString());
        mongoObject = condition.getMongoObject();
        assertEquals("calls_queued", mongoObject.get("condition"));
        assertEquals("!=", mongoObject.get("relation"));
    }

    public void testGetActionMongoObject() throws Exception {
        OpenAcdRecipeAction action = new OpenAcdRecipeAction();
        action.setAction(OpenAcdRecipeAction.ACTION.SET_PRIORITY.toString());
        action.setActionValue("5");
        BasicDBObject mongoObject = action.getMongoObject();
        assertEquals("set_priority", mongoObject.get("action"));
        assertEquals("5", mongoObject.get("actionValue"));

        OpenAcdSkill skill1 = new OpenAcdSkill();
        skill1.setAtom("_en");
        OpenAcdSkill skill2 = new OpenAcdSkill();
        skill2.setAtom("_de");
        action.addSkill(skill1);
        action.addSkill(skill2);
        action.setAction(OpenAcdRecipeAction.ACTION.ADD_SKILLS.toString());
        mongoObject = action.getMongoObject();
        assertEquals("add_skills", mongoObject.get("action"));
        assertEquals("_en, _de", mongoObject.get("actionValue"));
        action.setAction(OpenAcdRecipeAction.ACTION.REMOVE_SKILLS.toString());
        mongoObject = action.getMongoObject();
        assertEquals("remove_skills", mongoObject.get("action"));
        assertEquals("_en, _de", mongoObject.get("actionValue"));
        action.setAction(OpenAcdRecipeAction.ACTION.SEND_TO_VOICEMAIL.toString());
        mongoObject = action.getMongoObject();
        assertEquals("voicemail", mongoObject.get("action"));
        assertEquals("", mongoObject.get("actionValue"));
    }

    public void testGetAdditionalObjects() throws Exception {
        OpenAcdQueue queue = new OpenAcdQueue();
        OpenAcdRecipeStep step1 = new OpenAcdRecipeStep();
        OpenAcdRecipeCondition condition1 = new OpenAcdRecipeCondition();
        condition1.setCondition(OpenAcdRecipeCondition.CONDITION.TICK_INTERVAL.toString());
        condition1.setRelation(OpenAcdRecipeCondition.RELATION.IS.toString());
        condition1.setValueCondition("5");
        step1.setConditions(Collections.singletonList(condition1));
        OpenAcdRecipeAction action1 = new OpenAcdRecipeAction();
        action1.setAction(OpenAcdRecipeAction.ACTION.SEND_TO_VOICEMAIL.toString());
        action1.setActionValue("");
        step1.setAction(action1);
        step1.setFrequency(OpenAcdRecipeStep.FREQUENCY.RUN_MANY.toString());
        OpenAcdRecipeStep step2 = new OpenAcdRecipeStep();
        OpenAcdRecipeAction action2 = new OpenAcdRecipeAction();
        action2.setAction(OpenAcdRecipeAction.ACTION.MEDIA_ANNOUCE.toString());
        action2.setActionValue("blah");
        OpenAcdRecipeCondition condition2 = new OpenAcdRecipeCondition();
        condition2.setCondition(OpenAcdRecipeCondition.CONDITION.AGENTS_AVAILABLE.toString());
        condition2.setRelation(OpenAcdRecipeCondition.RELATION.IS.toString());
        condition2.setValueCondition("10");
        step2.setConditions(Collections.singletonList(condition2));
        step2.setAction(action2);
        step2.setFrequency(OpenAcdRecipeStep.FREQUENCY.RUN_ONCE.toString());
        queue.addStep(step1);
        queue.addStep(step2);

        List<BasicDBObject> steps = queue.getAdditionalObjects();
        assertEquals(2, steps.size());
        BasicDBObject obj1 = steps.get(0);
        assertEquals("run_many", obj1.get("frequency"));
        assertEquals("New Step", obj1.get("stepName"));
        assertEquals(1, ((List<BasicDBObject>) obj1.get("conditions")).size());
        BasicDBObject obj2 = steps.get(1);
        BasicDBObject actionObj = (BasicDBObject) obj2.get("action");
        assertEquals("announce", actionObj.get("action"));
        assertEquals("blah", actionObj.get("actionValue"));
    }

}
