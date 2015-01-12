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

uw.controller('secview', [
  '$scope',
  'uiService',
  function ($scope, uiService) {

    $scope.template           = uiService.secondary.template;
    $scope.chat               = uiService.secondary.chat;
    $scope.voicemail          = uiService.secondary.voicemail;
    $scope.conf               = uiService.secondary.conference;
    $scope.myprofile          = uiService.secondary.profile;

    // ngOptions
    // $scope.voicemail.folder   = $scope.voicemail.folders[0];

    $scope.$on('services.uiservice.changeview', function (e, obj) {
      if (obj.type) {
        // $scope.$destroy();
      }
    });

    $scope.$on('services.chat.receivedPresence', function () {
      $scope.$apply();
    });

    $scope.$on('services.ui.queryOccupants', function () {
      $scope.$apply();
    });

    $scope.$on('services.chat.receivedChatstate', function () {
      $scope.$apply();
    })

  }
]);
})();
