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

  /**
   * scroll document element into view
   * useful when body has overflow:hidden
   */
  uw.directive('clickView', function () {
    return {
      restrict: 'A',
      link: function (scope, elem, attrs) {
        elem.on('click', function () {
          if (scope.item) {
            if (scope.item.type === 'right') {
              document.querySelector('.right-side-view').scrollIntoView();
            }
            return;
          } else if (attrs.clickView === 'true') {
            document.querySelector('.right-side-view').scrollIntoView();
            return;
          } else {
            document.querySelector('.left-side-view').scrollIntoView();
            return;
          }
        })

      }
    }
  })

})();
