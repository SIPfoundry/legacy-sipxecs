import 'dart:html';
import 'dart:convert';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : false);

main() {
  new DnsDefaultViewEditor();
}

class DnsDefaultViewEditor {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;
  
  DnsDefaultViewEditor() {
    querySelector("#ok").onClick.listen(ok);
    querySelector("#apply").onClick.listen(apply);
    querySelector("#cancel").onClick.listen(cancel);

    querySelector("#customRecordsIdsRow").onClick.listen(loadPreview);      
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
  
  loadPreview([e]) {
    var elem = querySelector("#preview");
    if (api.test) {
      elem.text = 'preview in test mode';
      return;
    }
    var meta = getFormData();
    HttpRequest req = new HttpRequest();
    req.open('POST', api.url("rest/dnsDefaultPreview"));
    req.setRequestHeader("Content-Type", "application/json");
    print(JSON.encode(meta)); 
    req.send(JSON.encode(meta));   
    req.onLoad.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        elem.text = req.responseText;
      }
    });
    
  }
  
  close() {
    window.location.href = 'EditDns.html';      
  }

  save([onOk]) {
    var meta = getFormData();
    HttpRequest req = new HttpRequest();
    var method;
    method = 'PUT';            
    req.open(method, api.url("rest/dnsDefaultView"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send(JSON.encode(meta));
    req.onLoad.listen((e) {
     if (DataLoader.checkResponse(msg, req)) {
        if (onOk != null) {
          onOk();
        }
        msg.success(getString("msg.actionSuccess"));
      }
    });      
  }  
  
  Map<String, Object> getFormData() {
    print("aaa");
    var meta = new Map<String,Object>();
    List<int> customRecordsIds = [];
    meta['customRecordsIds'] = customRecordsIds;
    SelectElement customs = querySelector("#customRecordsIds");
    for (var option in customs.selectedOptions) {
      customRecordsIds.add(int.parse(option.value));
    }
    print(meta['customRecordsIds']);
    return meta;
  }
  
  int safeInt(String value) {
    return value == null || value == "" ? null : int.parse(value);
  }
  
  load() {
    var url = api.url("rest/dnsDefaultView", "edit-view-test.json");
    loader.load(url);
  }
  
  loadCustomRecords(Map<String, String> customRecordsOptions, List<int> customRecordsIds) {
    if (customRecordsOptions.length == 0) {
      // If there are no custom record sets defined, don't bother showing list. It's disconcerting
      // to show select box where there's nothing to select
      querySelector("#customRecordsIdsRow").style.display = "none";
      return;
    }
    SelectElement select = querySelector("#customRecordsIds");
    customRecordsOptions.forEach((id, value) {
      bool selected = (customRecordsIds != null && customRecordsIds.contains(int.parse(id)));
      select.append(new OptionElement(data: value, value: id, selected : selected));
    });    
  }
  
 
  loadForm(json) {
    var data = JSON.decode(json);
    print('$data');
    Map<String, Object> view = data['view'];
    List<int> customRecordsIds;
    if (view != null) {
      customRecordsIds = view['customRecordsIds'];
    }
    loadCustomRecords(data['customRecordsCandidates'], customRecordsIds);
    loadPreview();
  }  
}
