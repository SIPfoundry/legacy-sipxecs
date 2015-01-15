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

(function(){
'use strict';

uw.controller('settingsController', [
  '$rootScope',
  '$scope',
  '$cookieStore',
  'restService',
  'uiService',
  'CONFIG',
  '_',
  function ($rootScope, $scope, $cookieStore, restService, uiService, CONFIG, _) {
    var authCookie    = $cookieStore.get('JSESSIONID');

    $scope.debug                  = (CONFIG.debug) ? 'off' : 'on';
    $scope.autoLogin              = (!_.isUndefined(authCookie)) ? 'off' : 'on';
    $scope.showResetButtonConn    = false;
    $scope.showResetButtonDebug   = false;
    $scope.toggleResetButton = function (str) {
      switch (str) {
        case 'conn':
          $scope.showResetButtonConn = !$scope.showResetButtonConn;
          break;
      }
    }
    $scope.settings   = [
      {
        icon: 'chat_to_call',
        name: 'Personal Attendant'
      },
      {
        icon: 'follow_me',
        name: 'Call Forwarding'
      },
      {
        icon: 'dialpad',
        name: 'Speed Dials'
      },
      {
        icon: 'settings_cogs',
        name: 'User Settings'
      }
    ];
    $scope.tooltips = {
      personalAttendant: {
        add: {
          'title': 'Add dialpad entry',
          'checked': false
        },
        rem: {
          'title': 'Remove entry',
          'checked': false
        }
      },
      fwd: {
        setup: {
          add: {
            'title': 'Add ring',
            'checked': false
          },
          rem: {
            'title': 'Remove ring',
            'checked': false
          }
        },
        sched: {
          add: {
            'title': 'Add time period',
            'checked': false
          },
          rem: {
            'title': 'Remove time period',
            'checked': false
          },
          remSelected: {
            'title': 'Discard schedule',
            'checked': false
          }
        }
      },
      speed: {
        add: {
          'title': 'Add speed dial',
          'checked': false
        },
        rem: {
          'title': 'Remove speed dial',
          'checked': false
        }
      }
    }
    $scope.selectOption = function () {
      _.each($scope.settings, function (o) {
        o.isSelected = false;
      });
      this.option.isSelected = true;
      $scope.selected = this.option.name;

      switch ($scope.selected) {
        case 'User Settings':
          $scope.userSettings.user.init();
          break;

        case 'Speed Dials':
          $scope.userSettings.speed.init();
          break;

        case 'Call Forwarding':
          $scope.userSettings.fwd.setup.init();
          break;

        case 'Personal Attendant':
          $scope.userSettings.personalAttendant.init();
          break;
      }
    };
    $scope.reloadApp      = uiService.secondary.logout.init;
    $scope.userSettings   = uiService.secondary.settings;
  }
]);
})();
