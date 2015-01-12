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

uw.service('restService', [
  '$rootScope',
  '$http',
  '$q',
  '$sce',
  'request',
  'xmlParser',
  'sharedFactory',
  'CONFIG',
  '_',
  function ($rootScope, $http, $q, $sce, request, xmlParser, sharedFactory, CONFIG, _) {
    var baseRest        = CONFIG.baseRest;
    var sipRest         = initSipRest();
    var msgUrls         = {};
    var self            = this;

    this.cred = {
      user: '',
      pass: ''
    }
    this.connected      = false;
    this.phonebook      = null;
    this.activityList   = null;

    // UNIMPLEMENTED
    // set Basic HTTP auth headers for all requests
    // $http.defaults.headers.common.Authorization = 'Basic ' + b64;

    /**
     * updates credentials and sets sipRest $http abstract
     * called from chatService upon successful login
     * @param  {String} user      XMPP user
     * @param  {String} pass      XMPP password
     */
    this.updateCredentials = function (user, pass) {
      this.cred.user = user || null;
      this.cred.pass = pass || null;
    }

    /**
     * gets and parses the CDR log
     * @param  {String} limit       maximum number of call logs to return
     * @return {Promise}            parsed log
     */
    this.getCDRlog = function (limit) {
      var deferred = $q.defer();

      sipRest.getCDRlog(limit).
        success(function (data) {
          parseXml(data, function (parsed) {
            deferred.resolve(parsed);
          })
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    /**
     * gets and parses messages from a folder
     * @param  {String} folder  folder name
     * @return {Promise}        parsed messages
     */
    this.getMessages = function (folder) {

      var deferred  = $q.defer();

      sipRest.getMessages(folder).
        success(function (data) {
          parseXml(data, function (parsed) {
            deferred.resolve(parsed);
          });
        }).
        error(function(e) {
          deferred.reject(e);
        })

      return deferred.promise;

    }

    /**
     * gets one message based on id
     * @param  {String} id      message ID
     * @return {Object}         promise  audio blob URL || error
     */
    this.getMessage = function (id) {
      var deferred    = $q.defer();
      var blob;

      if (msgUrls[id])
        deferred.resolve(msgUrls[id])
      else
        sipRest.getMessage(id).
          success(function (data) {
            blob          = new Blob([data]); // the blob
            msgUrls[id]   = $sce.trustAsResourceUrl(window.URL.createObjectURL(blob)); // the blob URL
            deferred.resolve(msgUrls[id]); // resolved
          }).
          error(function (e) {
            deferred.reject(e);
          })

      return deferred.promise;
    }

    this.delMessage = function (id) {
      var deferred = $q.defer();

      sipRest.putTrashMsg(id).
        success(function (data) {
          delete msgUrls[id];
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.heardMessage = function (id) {
      var deferred = $q.defer();

      sipRest.putHeardMsg(id).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putMoveMsg = function (id, folder) {
      var deferred = $q.defer();
      id = id.trim();

      sipRest.putMoveMsg(id, folder).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getHeardMessage = function (id) {
      var deferred = $q.defer();

      sipRest.getHeardMsg(id).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putSubjectMessage = function (id, sub) {
      var deferred = $q.defer();

      sipRest.putSubjectMessage(id, sub).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.postBasicCall = function (to) {
      var deferred = $q.defer();

      sipRest.postBasicCall(to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getBasicCall = function (to) {
      var deferred = $q.defer();

      sipRest.getBasicCall(to).
        success(function (data) {
          parseXml(data, function (parsed) {
            deferred.resolve(parsed);
          });
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfList = function () {
      var deferred  = $q.defer();

      sipRest.getConfList().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putConfInvite = function (conf, to) {
      var deferred = $q.defer();

      sipRest.putConfInvite(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfPartList = function (conf) {
      var deferred = $q.defer();

      sipRest.getConfPartList(conf).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (data, status) {
          deferred.reject(status);
        })

      return deferred.promise;
    }

    this.getConfSettings = function (conf) {
      var deferred = $q.defer();

      sipRest.getConfSettings(conf).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (data, status) {
          deferred.reject(status);
        })

      return deferred.promise;
    }

    this.putConfSettings = function (conf, data) {
      var deferred = $q.defer();

      sipRest.putConfSettings(conf, data).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (data, status) {
          deferred.reject(status);
        })

      return deferred.promise;
    }

    this.getConfKick = function (conf, to) {
      var deferred = $q.defer();

      sipRest.getConfKick(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfMute = function (conf, to) {
      var deferred = $q.defer();

      sipRest.getConfMute(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfUnmute = function (conf, to) {
      var deferred = $q.defer();

      sipRest.getConfUnmute(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfDeaf = function (conf, to) {
      var deferred = $q.defer();

      sipRest.getConfDeaf(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfUndeaf = function (conf, to) {
      var deferred = $q.defer();

      sipRest.getConfUndeaf(conf, to).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getConfPin = function (conf, pin) {
      var deferred = $q.defer();

      sipRest.getConfPin(conf, pin).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getPhonebook = function () {
      var deferred = $q.defer();

      sipRest.getPhonebook().
        success(function (data) {
          $rootScope.$broadcast('services.rest.phonebook', {});
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putPassword = function (pass) {
      var deferred = $q.defer();

      sipRest.putPassword(pass).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getImBot = function () {
      var deferred = $q.defer();

      sipRest.getImBot().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putImBot = function (data) {
      var deferred = $q.defer();

      sipRest.putImBot(data).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getSpeeddial = function () {
      var deferred = $q.defer();

      sipRest.getSpeeddial().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putSpeeddial = function (data) {
      var deferred = $q.defer();

      sipRest.putSpeeddial(data).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getContactInfo = function () {
      var deferred = $q.defer();

      sipRest.getContactInfo().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putContactInfo = function (data) {
      var deferred = $q.defer();

      sipRest.putContactInfo(data).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getLogindetails = function (data) {
      var deferred = $q.defer();

      sipRest.getLogindetails().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getSchedule = function () {
      var deferred = $q.defer();

      sipRest.getSchedule().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putSchedule = function (id, d) {
      var deferred = $q.defer();

      sipRest.putSchedule(id, d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.postSchedule = function (d) {
      var deferred = $q.defer();

      sipRest.postSchedule(d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.delSchedule = function (id) {
      var deferred = $q.defer();

      sipRest.delSchedule(id).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getForward = function () {
      var deferred = $q.defer();

      sipRest.getForward().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putForward = function (d) {
      var deferred = $q.defer();

      sipRest.putForward(d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getPersonalAttendant = function () {
      var deferred = $q.defer();

      sipRest.getPersonalAttendant().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putPersonalAttendant = function (d) {
      var deferred = $q.defer();

      sipRest.putPersonalAttendant(d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getPersonalAttendantLang = function () {
      var deferred = $q.defer();

      sipRest.getPersonalAttendantLang().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getVmPrefs = function () {
      var deferred = $q.defer();

      sipRest.getVmPrefs().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putVmPrefs = function (d) {
      var deferred = $q.defer();

      sipRest.putVmPrefs(d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.putVmPin = function (d) {
      var deferred = $q.defer();

      sipRest.putVmPin(d).
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getKeepAlive = function () {
      var deferred = $q.defer();

      sipRest.getKeepAlive().
        success(function (data) {
          deferred.resolve(data);
        }).
        error(function (e) {
          deferred.reject(e);
        })

      return deferred.promise;
    }

    this.getAllImages = function (phonebookList) {
      var promises    = createPromiseArray(phonebookList);
      var avatarUrl   = '';
      var url         = '';

      $q.all(promises).
        then(function (responses) {
          createObjectURL(phonebookList, responses);
        }).
        catch(function (err) {
          console.log(err);
        });

      function createPromiseArray(array) {
        var promises = [];

        _.each(array, function (entry) {
          // Production:
          // url = entry['contact-information'].avatar || '';
          if (entry['contact-information'] && entry['contact-information'].avatar) {
            if (entry['contact-information'].avatar.indexOf('/sipxconfig/rest') !== -1) {
              avatarUrl   = entry['contact-information'].avatar.split('/sipxconfig/rest')[1];
              url         = CONFIG.baseRest + avatarUrl;
            } else {
              url = '';
            }
          } else {
            url = '';
          };

          promises.push($http.get(url, {cache: true, responseType: 'arraybuffer'}));
        });

        return promises;
      }

      function createObjectURL(array, values) {
        var i = 0;
        var blob, url;

        array.forEach(function (el, i) {
          if (!_.isUndefined(values[i]) && values[i].config.url !== '') {
            blob              = new Blob([values[i].data], {type: 'image/png'});
            url               = window.URL.createObjectURL(blob)
            array[i].avatar   = url;
          } else if (values[i].config.url === '') {
            array[i].avatar   = sharedFactory.defaultImg;
          }
        })
      }
    }

    /**
     * initializes sipRest methods
     * eZuce proprietary
     * @return {[type]} [description]
     */
    function initSipRest() {
      return {
        getCDRlog: function (limit) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/cdr/' + self.cred.user + '?limit=' + limit
          }))
        },
        getUserInfo: function (user) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/search/phonebook?query=' + user
          }))
        },
        getMessages: function (folder) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/' + folder
          }))
        },
        getMessage: function (id) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/media/' + self.cred.user + '/inbox/' + id,
            responseType: 'arraybuffer'
          }))
        },
        putHeardMsg: function (id) {
          return request(authHeaders({
            method:  'PUT',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/message/' + id + '/heard',
            data:    {}
          }))
        },
        putMoveMsg: function (id, folder) {
          return request(authHeaders({
            method:  'PUT',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/message/' + id + '/move/' + folder,
            data:    {}
          }))
        },
        getHeardMsg: function (id) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/message/' + id + '/heard'
          }))
        },
        putTrashMsg: function (id) {
          return request(authHeaders({
            method:  'PUT',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/message/' + id + '/delete',
            data:    {}
          }))
        },
        putSubjectMessage: function (id, sub) {
          return request(authHeaders({
            method:  'PUT',
            url:     baseRest + '/my/redirect/mailbox/' + self.user + '/message/' + id + '/subject',
            data:    sub
          }))
        },
        getActiveGreeting: function () {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/preferences/activegreeting'
          }))
        },
        delActiveGreeting: function () {
          return request(authHeaders({
            method:  'DELETE',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/preferences/activegreeting'
          }))
        },
        putActiveGreeting: function (greeting) {
          return request(authHeaders({
            method:  'PUT',
            body:    '<activegreeting>' + greeting + '</activegreeting>',
            url:     baseRest + '/my/redirect/mailbox/' + self.cred.user + '/preferences/activegreeting'
          }))
        },
        postBasicCall: function (to) {
          return request(authHeaders({
            method:  'POST',
            url:     baseRest + '/my/redirect/callcontroller/' + self.user + '/' + escape(to)
          }))
        },
        getBasicCall: function (to) {
          return request(authHeaders({
            method:  'GET',
            url:     baseRest + '/my/redirect/callcontroller/' + self.user + '/' + escape(to)
          }))
        },
        getConfList: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conferences',
            headers: {
              'Accept': 'application/json'
            }
          }));
        },
        getConfSettings: function (conf) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conferences/' + conf,
            headers: {
              'Accept': 'application/json'
            }
          }));
        },
        putConfSettings: function (conf, data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/conferences/' + conf,
            data:   data
          }));
        },
        putConfLock: function (name) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/conference/' + name + '/lock'
          }));
        },
        putConfInvite: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/invite\&' + to
          }));
        },
        getConfPartList: function (conf) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conferencedetails/' + conf,
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        getConfKick: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/kick\&' + to
          }));
        },
        getConfMute: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/mute\&' + to
          }));
        },
        getConfUnmute: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/unmute\&' + to
          }));
        },
        getConfDeaf: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/deaf\&' + to
          }));
        },
        getConfUndeaf: function (conf, to) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/undeaf\&' + to
          }));
        },
        getConfPin: function (conf, pin) {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/conference/' + conf + '/pin\&' + pin
          }));
        },
        getPhonebook: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/phonebook',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putPassword: function (pass) {
          return request(authHeaders({
            method:  'PUT',
            url:     baseRest + '/my/portal/password/' + encodeURIComponent(pass),
            data:    {}
          }))
        },
        getImBot: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/imbot/prefs'
          }))
        },
        putImBot: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/imbot/prefs',
            data:   data
          }))
        },
        getSpeeddial: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/speeddial'
          }))
        },
        putSpeeddial: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/speeddial',
            data:   data
          }))
        },
        getContactInfo: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/contact-information',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putContactInfo: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/contact-information',
            data:   data
          }))
        },
        getLogindetails: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/logindetailsunite',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        getSchedule: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/callfwdsched/',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putSchedule: function (id, data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/callfwdsched/' + id,
            data:   data
          }))
        },
        postSchedule: function (data) {
          return request(authHeaders({
            method: 'POST',
            url:    baseRest + '/my/callfwdsched/',
            data:   data
          }))
        },
        delSchedule: function (id, data) {
          return request(authHeaders({
            method: 'DELETE',
            url:    baseRest + '/my/callfwdsched/' + id
          }))
        },
        getForward: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/callfwd/',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putForward: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/callfwd/',
            data:   data
          }))
        },
        getPersonalAttendant: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/voicemail/attendant',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putPersonalAttendant: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/voicemail/attendant',
            data:   data
          }))
        },
        getPersonalAttendantLang: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/voicemail/attendant/lang',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        getVmPrefs: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/vmprefs',
            headers: {
              'Accept': 'application/json'
            }
          }))
        },
        putVmPrefs: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/vmprefs',
            data:   data
          }))
        },
        putVmPin: function (data) {
          return request(authHeaders({
            method: 'PUT',
            url:    baseRest + '/my/voicemail/pin/' + data,
            data:   data
          }))
        },
        getKeepAlive: function () {
          return request(authHeaders({
            method: 'GET',
            url:    baseRest + '/my/keepalive',
          }))
        }

        // generates requests functions
        // like above, but instead passing through a list of objects
        // function genReqFn(list) {
        //   var res;
        //   _.each(list, function (obj, key) {
        //     res[key] = function (param) {
        //       return request(authHeaders({
        //         method: obj.method,
        //         url: obj.url,
        //         body: obj.body || null,
        //         responseType: obj.responseType || null
        //       }))
        //     }
        //   })
        //   return res;
        // }
      }
    }

    /**
     * parses general XML response from the server
     * @param  {String} xml         XML string
     * @param  {Function} cb        callback
     * @return {Array}              array of attributes
     */
    function parseXml (xml, cb) {
      var $             = angular.element;
      var children      = xmlParser.parse(xml).children ? ($(xmlParser.parse(xml).children)[0].children) : ($(xmlParser.parse(xml)).children().children()) ? ($(xmlParser.parse(xml)).children().children()) : null;
      var arr           = [];
      var attrs         = [];
      var obj           = {};
      var subChildren   = [];

      _.each(children, function (child) {
        attrs         = $(child)[0].attributes;
        subChildren   = $(child).children();
        if (!_.isEmpty(attrs)) {
          obj = parseAttrs(child);
          if (!_.isEmpty(obj)) {
            arr.push(obj);
            obj = {};
          }
        };

        if (!_.isEmpty(subChildren)) {
          _.each( subChildren, function (c) {
            switch ($(c)[0].tagName) {
              // CDR
              case 'caller_aor':
                obj['from']       = $(c)[0].textContent; break;
              case 'callee_aor':
                obj['to']         = $(c)[0].textContent; break;
              case 'start_time':
                obj['start']      = $(c)[0].textContent; break;
              case 'duration':
                obj['duration']   = $(c)[0].textContent; break;

              // Call Status
              case 'timestamp': case 'call-id': case 'method':
                break;
              case 'status-line':
                obj['status']     = $(c)[0].textContent; break;
              default:
                break;
            }
          })
          if (!_.isEmpty(obj)) {
            arr.push(obj);
            obj = {};
          }
        }

      });

      cb(arr);
    }

    /**
     * parses NamedNodeMaps attributes
     * @param  {NamedNodeMap} NamedNodeMap    DOM node
     * @return {Object}                       an object containing attribute names and their values
     */
    function parseAttrs (NamedNodeMap) {
      var attrs   = angular.element(NamedNodeMap)[0].attributes;
      var obj     = {};

      _.each(attrs, function (attr) {
        obj[attr.name.toString()] = filterXmlValue(attr.value.toString());
      });

      return obj;
    }

    /**
     * transforms string value into corresponding type
     * works only with Strings, Numbers & Booleans
     * e.g. "1001" into 1001
     *      "true" into true
     * @param  {String} val                the value to be converted
     * @return {String|Number|Boolean}     converted value
     */
    function filterXmlValue(val) {
      var value = val;

      switch (value.toLowerCase()) {
        case 'true': case 'yes': case '1':
          value = true; break;

        case 'false': case 'no': case '0':
          value = false; break;

        default:
          if (!_.isNaN(parseInt(value)))
            value = value.toString();
          else
            value = unescape(val);
      }

      return value;
    }

    /**
     * extends request factory configuration object with auth headers
     * @param  {Object} conf request factory ($http) configuration object
     * @return {Object}      extended object
     */
    function authHeaders(conf) {
      return conf;
    }

  }
]);
})();
