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
import 'dart:html';
import 'dart:convert';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : false);

main() {
  print("hello");
  new DnsCustomEditor();  
}

class DnsCustomEditor {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;
  int uid = 0;
  int dnsCustomId;
  
  DnsCustomEditor() {
    querySelector("#ok").onClick.listen(ok);
    querySelector("#apply").onClick.listen(apply);
    querySelector("#cancel").onClick.listen(cancel);
    Location l = document.window.location;
    var params = Uri.parse(l.href).queryParameters;
    if (params['dnsCustomId'] != null) {
      dnsCustomId = int.parse(params['dnsCustomId']);
    }
    loader = new DataLoader(this.msg, loadForm);    
    load();
  }
  
  ok(e) {
    save(close);
  }
  
  apply(e) {
    save();
  }
  
  cancel(e) {
    close();
  }
  
  void save([onOk]) {
    var id = (dnsCustomId != null ? dnsCustomId : '');
    HttpRequest req = new HttpRequest();   
    var meta = {};
    var method;
    if (dnsCustomId != null) {
      meta['id'] = dnsCustomId;
      method = 'PUT';
    } else {
      method = 'POST';      
    }
    meta['name'] = name().value;
    meta['records'] = records().value;
    req.open(method, api.url("rest/dnsCustom/${id}"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send(JSON.encode(meta));
    req.onLoadEnd.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        msg.success(getString("msg.actionSuccess"));
        if (dnsCustomId == null) {
          dnsCustomId = int.parse(req.responseText);
        }
        if (onOk != null) {
          onOk();
        }
      }
    });      
  }
    
  close() {
    window.location.href = 'EditDns.html';      
  }
  
  load() {
    if (dnsCustomId == null) {
      return;
    }
    var url = api.url("rest/dnsCustom/${dnsCustomId}", "edit-custom-test.json");
    loader.load(url);
  }
  
  InputElement name() {
    return querySelector("#name");    
  }
  
  TextAreaElement records() {
    return querySelector("#records");
  }
  
  loadForm(json) {
    var data = JSON.decode(json);
    var custom = data['custom']; 
    name().value = custom['name'];
    records().value = custom['records'];
  }  
}
