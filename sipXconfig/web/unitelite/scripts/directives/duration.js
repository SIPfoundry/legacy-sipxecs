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

(function () {

  'use strict';

  uw.directive('duration', function () {
    return {
      restrict: 'A',
      link: function (scope, elem, attrs) {
        // Minutes and seconds
        var time  = attrs.duration;
        var mins  = ~~(time / 60);
        var secs  = time % 60;
        var ret   = '';

        // Output like '1:01' or '4:03:59' or '123:03:59'
        ret += mins + 'min ' + (secs < 10 ? '0' : '');
        ret += '' + secs + 's';
        elem.text(ret);
      }
    }
  })

})();
