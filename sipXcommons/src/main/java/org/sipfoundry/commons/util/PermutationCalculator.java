/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.util;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public class PermutationCalculator<T> {

    private final List<List<T>> permutations;

    public PermutationCalculator(List<T> elements) {
        List<T> initialPermutation = new LinkedList<T>();
        permutations = new LinkedList<List<T>>();

        for (T element : elements) {
            initialPermutation.add(element);
        }

        for (int shift = 0; shift > -elements.size(); shift--) {
            List<T> currentPermutation = new LinkedList<T>(initialPermutation);
            Collections.rotate(currentPermutation, shift);
            permutations.add(currentPermutation);
        }
    }

    public List<T> getPermutation(int index) {
        return getPermutation(index, (T[]) null);
    }

    public List<T> getPermutation(int index, T... excludedElements) {
        List<T> elementCopy = new LinkedList<T>(permutations.get(index));
        if (excludedElements != null) {
            for (T element : excludedElements) {
                elementCopy.remove(element);
            }
        }

        return elementCopy;
    }
}
