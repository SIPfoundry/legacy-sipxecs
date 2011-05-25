require 'webrick'
require 'rexml/document'

class ActiveCdrs < WEBrick::HTTPServlet::AbstractServlet
  def initialize(cdrService, log=nil)
    @cdrService = cdrService
    @log = log
    @user_re = /\s*<?(?:sip:)?(.+?)[@;].+/
    @fulluser_re = /\s*(?:"?\s*([^"<]+?)\s*"?)?\s*<(?:sip:)?(.+?)[@;].+/
  end

  #
  ## Servlet interfaces for WEBrick.
  #
  def get_instance(config, *options)
    @config = config
    self
  end

  def do_GET(req, resp)
    active_calls = @cdrService.getActiveCalls
    @log.debug("getActiveCalls by REST #{active_calls.size}") if @log

    doc = REXML::Document.new '<?xml version="1.0" encoding="UTF-8"?><cdrs/>'
    name = req.query['name']
    active_calls.each do | active |
       add_cdr(doc, active, name)
    end

    resp.body="#{doc}"
    raise WEBrick::HTTPStatus::OK
  end

  def add_cdr(doc, active, name)
    from = "#{active.from}"
    to = "#{active.to}"
    cdr = REXML::Element.new("cdr")

    cdr.add_element("from")
    cdr.elements["from"].text = from
    cdr.add_element("to")
    cdr.elements["to"].text = to
    cdr.add_element("recipient")
    cdr.elements["recipient"].text = "#{active.recipient}"
    cdr.add_element("start_time")
    cdr.elements["start_time"].text = "#{active.start_time.to_i * 1000}"
    cdr.add_element("duration")
    cdr.elements["duration"].text = "#{active.duration}"
    unless name
        doc.root.elements << cdr
    else
      unless matcher(doc, cdr, from, name)
        matcher(doc, cdr, to, name)
      end
    end
  end

  def matcher(doc, cdr, from_to, name)
      m = @user_re.match from_to
      if m
         if m[1]==name
            doc.root.elements << cdr
            return doc
         end
      end
      m_full = @fulluser_re.match from_to
      if m_full
         if m_full[2]==name
            doc.root.elements << cdr
         end
      end
  end
end
