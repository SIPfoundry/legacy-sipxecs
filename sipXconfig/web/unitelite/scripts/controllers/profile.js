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

uw.controller('profile', [
  '$sce',
  '$rootScope',
  '$scope',
  '$interval',
  '$timeout',
  'uiService',
  'restService',
  'sharedFactory',
  function ($sce, $rootScope, $scope, $interval, $timeout, uiService, restService, sharedFactory) {

    var templates                 = sharedFactory.settings.miniTemplates;
    var profile                   = restService.cred;
    var timeout;
    var sipMessages               = [
      '',
      {
        '100': 'Trying',
        '101': 'Dialogue Establishment',
        '180': 'Ringing',
        '181': 'Call is Being Forwarded',
        '182': 'Queued',
        '183': 'Session in Progress',
        '199': 'Early Dialog Terminated'
      },
      {
        '200': 'Answered',
        '202': 'Accepted',
        '204': 'No Notification'
      },
      {
        '300': 'Multiple Choices',
        '301': 'Moved Permanently',
        '302': 'Moved Temporarily',
        '305': 'Use Proxy',
        '380': 'Alternative Service'
      },
      {
        '400': 'Bad Request',
        '401': 'Unauthorized',
        '402': 'Payment Required',
        '403': 'Forbidden',
        '404': 'Number Not Found',
        '405': 'Method Not Allowed',
        '406': 'Not Acceptable',
        '407': 'Proxy Authentication Required',
        '408': 'Call Timeout',
        '409': 'Conflict',
        '410': 'Gone',
        '411': 'Length Required',
        '412': 'Conditional Request Failed',
        '413': 'Request Entity Too Large',
        '414': 'Request-URI Too Long',
        '415': 'Unsupported Media Type',
        '416': 'Unsupported URI Scheme',
        '417': 'Unknown Resource-Priority',
        '420': 'Bad Extension',
        '421': 'Extension Required',
        '422': 'Session Interval Too Small',
        '423': 'Interval Too Brief',
        '424': 'Bad Location Information',
        '428': 'Use Identity Header',
        '429': 'Provide Referrer Identity',
        '430': 'Flow Failed',
        '433': 'Anonymity Disallowed',
        '436': 'Bad Identity-Info',
        '437': 'Unsupported Certificate',
        '438': 'Invalid Identity Header',
        '439': 'First Hop Lacks Outbound Support',
        '470': 'Consent Needed',
        '480': 'Number Unavailable',
        '481': 'Call/Transaction Does Not Exist',
        '482': 'Loop Detected.',
        '483': 'Too Many Hops',
        '484': 'Address Incomplete',
        '485': 'Ambiguous response from server',
        '486': 'Number',
        '487': 'Request Terminated',
        '488': 'Not Acceptable Here',
        '489': 'Bad Event',
        '491': 'Request Pending',
        '493': 'Undecipherable',
        '494': 'Security Agreement Required'
      },
      {
        '500': 'Server Internal Error',
        '501': 'Not Implemented',
        '502': 'Bad Gateway',
        '503': 'Service Unavailable',
        '504': 'Server Time-out',
        '505': 'Version Not Supported',
        '513': 'Message Too Large',
        '580': 'Precondition Failure'
      },
      {
        '600': 'Busy Everywhere',
        '603': 'Decline',
        '604': 'Does Not Exist Anywhere',
        '606': 'Not Acceptable'
      }
    ]
    var interval;

    $scope.name                   = restService.cred;
    $scope.isUserProfileVisible   = false;
    $scope.template               = templates[0];

    $scope.ui             = uiService.ui.root;
    $scope.selectedItem   = uiService.ui.root.template;
    $scope.search         = uiService.search;
    $scope.callNo         = '';
    $scope.displayNo      = '';
    $scope.showMainMenu   = false;
    $scope.showDial       = false;
    $scope.startNo        = false;
    $scope.showSearch     = false;
    $scope.sipEnabled     = false;
    $scope.searchResult   = [];

    $scope.showMainMenuFn = function () {
      $scope.showMainMenu   = !$scope.showMainMenu;
      $scope.showDial       = false;
      $scope.showSearch     = false;
      $scope.callNo         = '';
      $scope.phone.msg      = null;

      return
    }
    $scope.broadcastViewChange = function (item) {
      $scope.search.t           = '';
      $scope.showMainMenu       = false;
      $scope.showSearch         = false;
      $scope.selectedItem       = item;
      $scope.phone.msg          = null;

      uiService.util.changeView(item);

      return
    }

    $scope.phone          = {
      call: null,
      active: null,
      msg: null,
      disabled: null
    }
    $scope.keyboard       = [
      {no: '1', text: ''},
      {no: '2', text: 'abc'},
      {no: '3', text: 'def'},
      {no: '4', text: 'ghi'},
      {no: '5', text: 'jkl'},
      {no: '6', text: 'mno'},
      {no: '7', text: 'pqrs'},
      {no: '8', text: 'tuv'},
      {no: '9', text: 'wxyz'},
      {no: '*', text: ''},
      {no: '0', text: ''},
      {no: '#', text: ''}
    ]
    $scope.showDialFn = function (force) {
      if (force) {
        $scope.showDial = true;
      } else {
        $scope.showDial = !$scope.showDial;
      }
      $scope.showMainMenu   = false;
      $scope.showSearch     = false;
      $scope.phone.msg      = null;

      if ($scope.showDial === false) {
        $interval.cancel(interval);
        $scope.phone.disabled = null;
      }

      return;
    }
    $scope.clearNo = function () {
      $scope.callNo = '';
      $scope.displayNo = '';

      return
    }
    $scope.addNo = function (no) {
      $scope.callNo = $scope.callNo + no;
      $scope.displayNo = $scope.displayNo + no;

      return;
    }
    $scope.phone.call = function () {
      var callNo              = angular.copy($scope.callNo);
      $scope.phone.msg        = 'Requesting call...';
      $scope.phone.disabled   = true;

      if (callNo !== '') {
        restService.postBasicCall(callNo)
          .then(function () {
            $scope.phone.disabled   = true;
            $scope.phone.msg        = 'Requesting call info...';
            startInterval(callNo);
          })
          .catch(function (status) {
            $scope.phone.msg = 'Error ' + status;
          });
      }

      function startInterval(no) {
        var match;
        var search;
        interval = $interval(function () {
          restService.getBasicCall(no).then(function (data) {
            if (data.length === 0) {
              $interval.cancel(interval);
              return;
            }

            // SIP status messages according to wikipedia
            //
            for (var i = 1; i < 7; i++) {
              search = data[data.length-1].status.search(i+'[0-9][0-9]');
              if (search !== -1) {
                match = data[data.length-1].status.match(i+'[0-9][0-9]')[0];

                $scope.phone.msg = sipMessages[i][match];

                if (match === '180') {
                  timeout = $timeout(function () {
                    if (interval) {
                      $interval.cancel(interval);
                    }
                    $scope.phone.msg = null;
                  }, 10000);
                }

                if (match === '183' || match === '180') {
                  $scope.phone.disabled = null;
                  break;
                }

                if (match === '200' && interval) {
                  $interval.cancel(interval);
                  $timeout.cancel(timeout);
                  break;
                }

                if (match === '404' || match === '408' || match === '480' || match === '486') {
                  $scope.phone.disabled = null;
                  $timeout.cancel(timeout);
                  if (interval) {
                    $interval.cancel(interval);
                  }
                  break;
                }
                break;
              }
              continue;
            }

          }).catch(function (status) {
            // SIP status messages according to wikipedia
            //
            for (var i = 1; i < 7; i++) {
              search = data[data.length-1].status.search(i+'[0-9][0-9]');
              if (search !== -1) {
                match = data[data.length-1].status.match(i+'[0-9][0-9]')[0];

                $scope.phone.msg = 'Error: ' + sipMessages[i][match];

                $timeout.cancel(timeout);
                if (interval) {
                  $interval.cancel(interval);
                }
                break;
              }
              continue;
            }

          })
        }, 1000);
      }

      return
    }

    $scope.showSearchFn = function () {
      $scope.showSearch = !$scope.showSearch;
      $scope.showMainMenu   = false;
      $scope.showDial       = false;
      $scope.callNo         = '';
      $scope.phone.msg      = null;

      if ($scope.showSearch === false && uiService.ui.root.template === uiService.ui.root.templates[8]) {
        uiService.util.changeView(uiService.ui.root.oldTemplate);
        $scope.search.t = '';
      }
    }

    $scope.searchResultClick = function (item) {
      $scope.displayNo = item.name;
      $scope.callNo = item.profile.vCard['X-INTERN'];
    }

    $scope.$watchCollection('search', function (val) {
      if (val.t === '') {
        uiService.util.changeView(uiService.ui.root.oldTemplate);
      } else if (uiService.ui.root.template !== uiService.ui.root.templates[7]) {
        uiService.util.changeView(uiService.ui.root.templates[7]);
      };

      return
    })

    $scope.$watch('displayNo', function (val) {
      if (val === '') {
        $scope.startNo = false;
        $scope.callNo = val;
      } else {
        $scope.callNo = val;
        var arr = [];
        val = val.toString().toLowerCase();
        $scope.startNo = true;
        arr = _.filter(uiService.rosterList.main, function (item) {
          if (item.name && item.number) {
            return (item.name.indexOf(val) !== -1) || (item.number.indexOf(val) !== -1)
          } else {
            return false;
          }
        });
        if (arr.length === 1 && arr[0].name === $scope.displayNo) {
          $scope.searchResult = [];
        } else {
          $scope.searchResult = arr;
        }
      }

      return
    })

    $rootScope.$on('controller.mainview.callDialpad', function (e, obj) {
      $scope.showDialFn(true);
      $scope.callNo = obj.number;
      $scope.displayNo = obj.number;
    })

  }
]);
})();
