/*
 * Copyright (c) eZuce, Inc. All rights reserved.
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

(function() {

  'use strict';

  uw.filter('translateDays', function () {
    return function (input) {
      var translated = input;

      var dict = [
        { day: -2, human: 'Weekend' },
        { day: -1, human: 'Weekdays' },
        { day: 0, human: 'Every day' },
        { day: 1, human: 'Sunday' },
        { day: 2, human: 'Monday' },
        { day: 3, human: 'Tuesday' },
        { day: 4, human: 'Wednesday' },
        { day: 5, human: 'Thursday' },
        { day: 6, human: 'Friday' },
        { day: 7, human: 'Saturday' }
      ];

      _.find(dict, function (obj) {
        if (translated === obj.day) {
          translated = obj.human;
          return true
        }
      })

      return translated;
    }
  })

})();
