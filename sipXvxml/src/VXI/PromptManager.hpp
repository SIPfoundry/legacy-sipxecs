/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.    
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here,
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks
 * International, Inc. in the United States and other countries.
 * 
 ***********************************************************************/

#include "VXIvalue.h"
#include <string>

typedef std::basic_string<VXIchar> vxistring;

extern "C" struct VXIpromptInterface;
class PropertyList;
class SimpleLogger;
class VXMLElement;
class VXMLNode;


class PromptTranslator {
public:
  virtual VXIValue * EvaluateExpression(const vxistring & expression) = 0;
  // Evaluates an expression which arises during prompt construction.  The
  // returned value will be freed by the PromptManager.

  virtual void SetVariable(const vxistring & name, const vxistring & value) =0;
  // Sets a local variable in the current scope.

  PromptTranslator() { }
  virtual ~PromptTranslator() { }
};


class PromptManager {
public:
  enum BargeIn {
    UNSPECIFIED,
    ENABLED,
    DISABLED
  };

  enum SegmentType {
    SEGMENT_AUDIO,
    SEGMENT_SSML,
    SEGMENT_TEXT
  };

  void PlayFiller(const PropertyList &propertylist, 
                  const vxistring & src);
  // Start playing 'filler' material (for fetchaudio, percolation, etc.)

  void Stop();
  // Stops prompt playing

  void Play();
  // Waits until the prompts currenly marked for playing are done.  Then starts
  // playing everything remaining in the queue.  This is generally followed by
  // a recognition / record attempt.

  void PlayAll();
  // Play everything currently in the queue.  The user is unable to barge in.

  void Queue(const VXMLNode& child, const VXMLElement & reference,
             const PropertyList &, PromptTranslator &);
  // This resolves <enumerate> and <value>.

  void Queue(const vxistring & uri);
  // Queues a segment from a known URI.

  int GetMillisecondTimeout() const;
  // Returns: the recognition timeout, in milliseconds, or -1 if none was
  // specified.

public:
  bool ConnectResources(SimpleLogger *, VXIpromptInterface *);
  PromptManager() : log(NULL), prompt(NULL) { }
  ~PromptManager() { }

private:
  void ProcessSegments(const VXMLNode & node,
                       const VXMLElement & item,
                       const PropertyList & propertyList,
                       PromptTranslator & translator,
                       BargeIn,
                       VXIMapHolder & props,
                       vxistring & sofar,
                       bool &);

  bool AddSegment(SegmentType, const vxistring & data,
                  const VXIMapHolder & properties, BargeIn,
                  bool throwExceptions = true);
  // Returns: true - segment successfully queued
  //          false - segment addition failed

  void AddContent(const VXMLElement & elem,
                  const VXMLElement & item,
                  const PropertyList & propertyList,
                  PromptTranslator & translator,
                  BargeIn,
                  VXIMapHolder & props,
                  vxistring & sofar,
                  VXIValue * value,
                  bool &);

private:
  SimpleLogger       * log;
  VXIpromptInterface * prompt;
  bool enabledSegmentInQueue;
  int timeout;
};

