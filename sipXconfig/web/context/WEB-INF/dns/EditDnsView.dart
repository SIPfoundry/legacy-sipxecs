import 'dart:html';
import 'dart:convert';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : false);

main() {
  new DnsViewEditor();
}

class DnsViewEditor {
  var msg = new UserMessage(querySelector("#message"));
  DataLoader loader;
  int dnsViewId;
  
  DnsViewEditor() {
    querySelector("#ok").onClick.listen(ok);
    querySelector("#apply").onClick.listen(apply);
    querySelector("#cancel").onClick.listen(cancel);
    
    for (var elemId in ["#regionId", "#planId", "#customRecordsIdsRow", "input[name=preview-show]"] ) {
      querySelectorAll(elemId).onClick.listen(loadPreview);      
    }
    Location l = document.window.location;
    var params = Uri.parse(l.href).queryParameters;
    if (params['dnsViewId'] != null) {
      dnsViewId = int.parse(params['dnsViewId']);
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
  
  loadPreview([e]) {
    var elem = querySelector("#preview");
    if (api.test) {
      elem.text = 'preview in test mode';
      return;
    }
    var meta = getFormData();
    String show = 'ALL';
    for (InputElement e in querySelectorAll("input[name=preview-show]")) {
      if (e.checked) {
        show = e.value;
        break;
      }
    }
    HttpRequest req = new HttpRequest();
    req.open('POST', api.url("rest/dnsPreview/${show}"));
    req.setRequestHeader("Content-Type", "application/json"); 
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
    var id = '';        
    if (dnsViewId == null) {
      method = 'POST';            
    } else {
      id = "${dnsViewId}/";
      meta['id'] = dnsViewId;
      method = 'PUT';            
    }
    req.open(method, api.url("rest/dnsView/${id}"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send(JSON.encode(meta));
    req.onLoad.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        if (dnsViewId == null) {
          dnsViewId = int.parse(req.responseText);
        }
        if (onOk != null) {
          msg.success("Save successful");
          onOk();
        }
      }
    });      
  }  
  
  Map<String, Object> getFormData() {
    var meta = new Map<String,Object>();
    meta['planId'] = safeInt((querySelector("#planId") as SelectElement).value);
    meta['name'] = (querySelector("#name") as InputElement).value;
    meta['regionId'] = safeInt((querySelector("#regionId") as SelectElement).value);
    List<int> customRecordsIds = [];
    meta['customRecordsIds'] = customRecordsIds;
    SelectElement customs = querySelector("#customRecordsIds");
    for (var option in customs.selectedOptions) {
      customRecordsIds.add(int.parse(option.value));
    }    
    return meta;
  }
  
  int safeInt(String value) {
    return value == null || value == "" ? null : int.parse(value);
  }
  
  load() {
    var id = (dnsViewId != null ? '${dnsViewId}' : 'blank');
    var url = api.url("rest/dnsView/${id}", "edit-view-test.json");
    loader.load(url);
  }
  
  loadRegionCandidates(Map<String, String> regionOptions, int regionId) {
    String regionIdStr = (regionId == null ? null : regionId.toString());
    SelectElement select = querySelector("#regionId");
    regionOptions.forEach((id, value) {
      bool selected = (int.parse(id) == regionId);
      select.append(new OptionElement(data: value, value: id, selected : selected));        
    });
  }
  
  loadPlanOptions(Map<String, String> planOptions, int planId) {
    String planIdStr = (planId == null ? null : planId.toString());
    SelectElement select = querySelector("#planId");
    planOptions.forEach((id, value) {
      bool selected = (int.parse(id) == planId);
      select.append(new OptionElement(data: value, value: id, selected : selected));
    });
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
    Map<String, Object> view = data['view'];
    int regionId;
    int planId;
    List<int> customRecordsIds;
    if (view != null) {
      (querySelector("#name") as InputElement).value = view['name'];
      regionId = view['regionId'];
      planId = view['planId'];
      customRecordsIds = view['customRecordsIds'];
    }
    loadRegionCandidates(data['regionCandidates'], regionId);
    loadPlanOptions(data['planCandidates'], planId);
    loadCustomRecords(data['customRecordsCandidates'], customRecordsIds);
    
    loadPreview();
  }  
}
