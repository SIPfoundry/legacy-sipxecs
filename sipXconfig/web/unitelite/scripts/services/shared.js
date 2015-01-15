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

uw.service('sharedFactory', [
  'localStorageService',
  '_',
  'CONFIG',
  function (localStorage, _, CONFIG) {
    var keyConversationList   = CONFIG.keyConversationList;
    var keyArchiveObj         = CONFIG.keyArchiveObj;
    var jid                   = '';

    this.archive = {};

    var archive = this.archive;

    this.init = function (ownJid) {
      jid = ownJid;
    }

    this.archiveMessage = function (o) {
      var obj = angular.copy(o);
      // console.log(obj);
      var done = false;
      var addr = (obj.from && obj.to) ? (obj.to) : (obj.from);
      // let's check for trailing/leading whitespace
      addr = addr.replace(/^\s+|\s+$/g,'');
      if (!archive[addr]) archive[addr] = [];
      if (archive[addr].length === 0) archive[addr].push(obj);
      else {
        for (var i = 0; i < archive[addr].length; i++) {
          if (archive[addr][i].timestamp.toString() === obj.timestamp.toString()) {
            archive[addr][i] = obj;
            done = true;
          }
        }
        if (!done) {
          archive[addr].push(obj);
        }
      };


      this.saveArchiveObj();

    };

    this.flushContactMessageList = function (jid) {
      archive[jid] = [];
      console.log('flush');
      this.saveArchiveObj(archive);
    };

    this.savePhotos = function (photos) {
      this.save(CONFIG.keyPhotos, photos);
    }

    this.getPhotos = function () {
      var json = this.get(CONFIG.keyPhotos);
      return _.isEmpty(json) ? [] : json;
    }

    this.saveRoster = function (roster) {
      this.save(CONFIG.keyRoster, roster);
    }

    this.getRoster = function () {
      var json = this.get(CONFIG.keyRoster);
      return _.isEmpty(json) ? [] : json;
    }

    this.getGroupChats = function () {
      var json = this.get(CONFIG.keyGroupChats);
      return _.isEmpty(JSON.parse(json)) ? [] : json;
    }

    this.saveGroupChats = function (groups) {
      this.save(CONFIG.keyGroupChats, groups);
    }

    this.getGroups = function () {
      var json = this.get(CONFIG.keyRosterGroups);
      return _.isEmpty(json) ? [] : json;
    }

    this.saveGroups = function (groups) {
      this.save(CONFIG.keyRosterGroups, groups);
    }

    this.saveGroupsContact = function (obj) {
      var roster = this.getRoster();
      var groups = this.getGroups();
      _.extend(roster[obj.jid], obj);

      for (var i = 0; i < groups.length; i++) {
        for (var j = 0; j < groups[i].length; j++) {
          if (groups[i][j].jid = obj.jid) {
            _.extend(groups[i][j], obj);
          }
        }
      }

      this.saveGroups(groups);
      this.saveRoster(roster);
    };

    this.getContactMessageList = function (contactJid) {
      // console.log(contactJid);
      // console.log(archive[contactJid]);
      // console.log(this.archive[contactJid]);
      if (!archive[contactJid]) archive[contactJid] = [];
      return archive[contactJid];
      // var bareJid = strophe.getBareJidFromJid(contactJid);
      // var list = _.find(archive, function (messageList, key) {
      //   return key === bareJid;
      // });
      // return list ? list : [];
    };

    this.delContactMessageList = function (contactJid) {
      if (!archive[contactJid]) return;
      delete archive[contactJid];
      this.saveArchiveObj();
    }

    this.getConversationList = function () {
      var json = this.get(CONFIG.keyConversationList);
      return _.isEmpty(json) ? [] : json; // return a list if empty
    };

    this.saveConversationList = function (list) {
      this.save(CONFIG.keyConversationList, list);
    };

    this.saveConversationListContact = function (obj) {
      // console.log(groups);
    }

    this.saveArchiveObj = function () {
      this.save(CONFIG.keyArchiveObj, archive);
    };

    this.getArchiveObj = function () {
      // this.archive = ;
      // this.archive = archive;
      this.archive = this.get(CONFIG.keyArchiveObj);
      if (typeof this.archive !== 'string')
        _.extend(archive, this.archive);
      // archive = this.archive || {};
    };

    this.save = function (key, json) {
      var suff = this.getUserSuffix();
      if (!suff) return;
      var str = angular.toJson(json);
      localStorage.add(key + suff, str);
    };

    this.get = function (key) {
      var str = localStorage.get(key + this.getUserSuffix()) || '{}';
      return str;
    };

    this.getUserSuffix = function () {
      var user = jid.split('#')[0];
      if (user === '') return false;

      return '.' + user;
    };

    // original code: https://github.com/eligrey/canvas-toBlob.js/blob/master/canvas-toBlob.js#L25
    // license: https://raw.github.com/eligrey/canvas-toBlob.js/master/LICENSE.md
    this.decodeBase64 = function(base64, cb) {
      var base64Ranks = new Uint8Array([
          62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1
        , -1, -1,  0, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9
        , 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
        , -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
        , 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
      ]);
      var len     = base64.length;
      var buffer  = new Uint8Array(len / 4 * 3 | 0);
      var i       = 0;
      var outptr  = 0;
      var last    = [0, 0];
      var state   = 0;
      var save    = 0;
      var rank
      var code
      var undef
      while (len--) {
        code = base64.charCodeAt(i++);
        rank = base64Ranks[code-43];
        if (rank !== 255 && rank !== undef) {
          last[1] = last[0];
          last[0] = code;
          save = (save << 6) | rank;
          state++;
          if (state === 4) {
            buffer[outptr++] = save >>> 16;
            if (last[1] !== 61 /* padding character */) {
              buffer[outptr++] = save >>> 8;
            }
            if (last[0] !== 61 /* padding character */) {
              buffer[outptr++] = save;
            }
            state = 0;
          }
        }
      }
      // 2/3 chance there's going to be some null bytes at the end, but that
      // doesn't really matter with most image formats.
      // If it somehow matters for you, truncate the buffer up outptr.
      cb(buffer);
      return buffer;
    }

    this.settings = {
      mainMenu: [
        {
          name: 'Activity List',
          url: 'views/activitylist.html',
          iconClass:  'icon-activity_stream',
          show: 'false'
        },
        {
          name: 'Contacts',
          url: 'views/rosterlist.html',
          iconClass:  'icon-presence_meeting',
          show: 'true'
        },
        {
          name: 'Conference Bridge',
          url: 'views/conference-bridge.html',
          iconClass: 'icon-conference_call',
          type: 'right',
          show: 'true',
          fn: 'conference'
        },
        {
          name: 'Voicemails',
          url: 'views/voicemails.html',
          iconClass: 'icon-voicemail',
          type: 'right',
          show: 'true',
          fn: 'voicemail'
        },
        {
          name: 'My profile',
          url: 'views/my-profile.html',
          iconClass: 'icon-my-profile',
          type: 'right',
          show: 'true',
          fn: 'profile'
        },
        {
          name: 'Settings',
          url: 'views/settings.html',
          iconClass: 'icon-settings_cogs',
          type: 'right',
          show: 'true'
        },
        {
          name: 'Logout',
          url: '/',
          iconClass: 'icon-logout',
          show: 'true',
          type: 'right',
          fn: 'logout'
        },
        {
          name: 'Search',
          url: 'views/search.html',
          iconClass: 'icon-settings_cogs',
          show: 'false'
        },
        {
          name: 'Chat',
          url: 'views/chat.html',
          iconClass: 'icon-settings',
          type: 'right',
          show: 'false'
        },
        {
          name: 'default',
          url: 'views/default-right.html',
          iconClass: 'icon-settings_cogs',
          type: 'right',
          show: 'false'
        }
      ],
      miniTemplates: [
        {
          name: 'Profile',
          url: 'views/profile.html',
        }
      ],
      defaultPresences: [
        {
          icon: 'icon-status_available',
          show: 'Available',
          status: '',
          color: '#38b427'
        },
        {
          icon: 'icon-clock_fill_away',
          show: 'Away',
          status: 'away',
          color: '#f18703'
        },
        {
          icon: 'icon-presence_dnd',
          show: 'Do not disturb',
          status: 'dnd',
          color: '#e3352d'
        },
        {
          icon: 'icon-clock_fill_xa',
          show: 'Extended away',
          status: 'xa',
          color: '#0d73b5'
        },
        {
          icon: 'icon-presence_invisible',
          show: 'Invisible',
          status: 'invisible',
          color: '#bdbfc0'
        }
      ],
      defaultLocations: [
        {
          icon: '',
          text: 'Home'
        },
        {
          icon: '',
          text: 'At work'
        },
        {
          icon: '',
          text: 'On the road'
        }
      ],
    };

    this.defaultImg = 'data:image/png;base64,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';

    return this;
  }
]);
})();
