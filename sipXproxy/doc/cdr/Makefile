
all: call-state-events.txt call-state-events.html

xml_entity_escape=perl -pe 's/&/&amp;/g; s/</&lt;/g; s/>/&gt;/g; s/\"/&quot;/g;'

call-state-events.txt: call-state-events.xml call_event_schema.esc call_event_examples.esc call_detail_record_schema.esc
	xml2txt call-state-events.xml

call-state-events.html: call-state-events.xml call_event_schema.esc call_event_examples.esc call_detail_record_schema.esc
	xml2html call-state-events.xml

call_event_schema.esc: ../../meta/call_event.xsd
	$(xml_entity_escape) < ../../meta/call_event.xsd > call_event_schema.esc

call_event_examples.esc: call_event_examples.xml
	$(xml_entity_escape) < call_event_examples.xml > call_event_examples.esc

call_detail_record_schema.esc: ../../meta/call_detail_record.xsd
	$(xml_entity_escape) < ../../meta/call_detail_record.xsd > call_detail_record_schema.esc
