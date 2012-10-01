#ifndef SUBSCRIPTIONQUEUE_H
#define	SUBSCRIPTIONQUEUE_H

#include <string>
#include <queue>
#include <map>
#include <cassert>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <net/SipMessage.h>
#include <statusserver/MwiMessageCounter.h>

#define MWI_PLUGIN_QUEUE_MAX_SIZE 1000

class MwiPluginQueue : boost::noncopyable
{
public:
  struct MwiData
  {
    SipMessage* subscribe;
    std::string mailBox;
    std::string domain;
    std::string mailBoxData;
    MwiData() : subscribe(0){}
  };
  
private:
  class Semaphore
  {
      //The current semaphore count.
      unsigned int _count;

      //_mutex protects _count.
      //Any code that reads or writes the _count data must hold a lock on
      //the mutex.
      boost::mutex _mutex;

      //Code that increments _count must notify the condition variable.
      boost::condition_variable _condition;

  public:
      explicit Semaphore(unsigned int initial_count = 0)
         : _count(initial_count),
           _mutex(),
           _condition()
      {
      }


      void signal() //called "release" in Java
      {
          boost::unique_lock<boost::mutex> lock(_mutex);

          ++_count;

          //Wake up any waiting threads.
          //Always do this, even if _count wasn't 0 on entry.
          //Otherwise, we might not wake up enough waiting threads if we
          //get a number of signal() calls in a row.
          _condition.notify_one();
      }

      void wait() //called "acquire" in Java
      {
          boost::unique_lock<boost::mutex> lock(_mutex);
          while (_count == 0)
          {
               _condition.wait(lock);
          }
          --_count;
      }

  };

  
  class BlockingQueue : boost::noncopyable
  {
  protected:
    std::queue<MwiData> _queue;
    Semaphore _semaphore;
    std::size_t _maxQueueSize;
    typedef boost::recursive_mutex mutex;
    typedef boost::lock_guard<mutex> mutex_lock;
    mutex _mutex;
    bool _terminating;
  public:
    BlockingQueue(std::size_t maxQueueSize) :
      _maxQueueSize(maxQueueSize),
      _terminating(false)
    {
    }

    ~BlockingQueue()
    {
      terminate();
    }

    void terminate()
    {
      //
      // Unblock dequeue
      //
      mutex_lock lock(_mutex);
      _terminating = true;
      _semaphore.signal();
    }

    bool dequeue(MwiData& data)
    {
      _semaphore.wait();
      mutex_lock lock(_mutex);
      if (_queue.empty() || _terminating)
        return false;
      data = _queue.front();
      _queue.pop();
      return true;
    }

    void enqueue(const MwiData& data)
    {
      _mutex.lock();
      if (_maxQueueSize && _queue.size() > _maxQueueSize)
      {
        _mutex.unlock();
        return;
      }
      _queue.push(data);
      _mutex.unlock();
      _semaphore.signal();
    }
  };

public:

  typedef boost::function<void(MwiData&)> Handler;

  MwiPluginQueue() :
    _queue(MWI_PLUGIN_QUEUE_MAX_SIZE),
    _pRunThread(0),
    _isTerminated(false)
  {
  }

  ~MwiPluginQueue()
  {
  }

  /*
   struct MwiData
  {
    SipMessage* subscribe;
    std::string mailBox;
    std::string domain;
  };
   */

  void enqueue(const MwiData& data)
  {
    _queue.enqueue(data);
  }

  void run(Handler handler)
  {
    _handler = handler;
    assert(!_pRunThread);
    _pRunThread = new boost::thread(boost::bind(&MwiPluginQueue::internal_run, this));
  }

  void stop()
  {
    if (!_pRunThread)
      return;
    _isTerminated = true;
    MwiData data;
    data.subscribe = 0;
    data.mailBox = "__TERMINATE__";
    data.domain = "__TERMINATE__";
    _queue.enqueue(data);
    _pRunThread->join();
    delete _pRunThread;
    _pRunThread = 0;
  }

private:

  void internal_run()
  {
    _isTerminated = false;
    while (!_isTerminated)
    {
      MwiData data;
      if (_queue.dequeue(data))
      {
        if (data.mailBox == "__TERMINATE__")
          break;

        if (data.subscribe)
        {
          //
          // Check if we have cached it previously
          //
          std::string identity = data.mailBox;
          identity += "@";
          identity += data.domain;
          std::map<std::string, std::string>::const_iterator iter = _notifyData.find(identity);
          if ( iter != _notifyData.end())
          {
            data.mailBox = identity;
            data.mailBoxData = iter->second;
          }
          else
          {
            MwiMessageCounter counter(data.mailBox);
            data.mailBox = identity;
            data.mailBoxData = counter.getMailBoxData(data.domain);
          }
        }

        _handler(data);

        //
        // Cache it
        //
        if (data.mailBox.find("@") != std::string::npos && !data.mailBoxData.empty())
          _notifyData[data.mailBox] = data.mailBoxData;

        if (data.subscribe)
          delete data.subscribe;
      }
    }
  }

  BlockingQueue _queue;
  Handler _handler;
  boost::thread* _pRunThread;
  bool _isTerminated;
  std::map<std::string, std::string> _notifyData;
};


#endif	/* SUBSCRIPTIONQUEUE_H */

