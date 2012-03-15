/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#ifndef ZMQPOLL_H
#define	ZMQPOLL_H

#include <cassert>
#include <vector>
#include "ZMQSocket.h"


class ZMQPoll
{
private:
  ZMQSocket* _socket1;
public:
  ZMQPoll(
    ZMQSocket* socket1
  ) :
      _socket1(socket1)
  {
  }

  bool poll(long timeout = -1, int revent = ZMQ_POLLIN)
  {
    zmq::pollitem_t _items[] = {
      { _socket1->zmqSocket(), 0, revent, 0 }
    };

    _socket1->_is_POLLIN = false;
    _socket1->_is_POLLOUT = false;


    int ret = zmq_poll (&_items [0], 1, timeout);
    if (ret <= 0)
      return false;

    if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLIN)
      {
        _socket1->_is_POLLIN = true;
      }

    }
    else
    {
      if (_items [0].revents & ZMQ_POLLOUT)
      {
        _socket1->_is_POLLOUT = true;
      }
    }

    return ret > 0;
  }

};


class ZMQPoll2
{
private:
  ZMQSocket* _socket1;
  ZMQSocket* _socket2;
public:
  ZMQPoll2(
    ZMQSocket* socket1,
    ZMQSocket* socket2
  ) :
      _socket1(socket1),
      _socket2(socket2)
  {
  }

  bool poll(long timeout = -1, int revent = ZMQ_POLLIN)
  {
    zmq::pollitem_t _items[] = {
      { _socket1->zmqSocket(), 0, revent, 0 },
      { _socket2->zmqSocket(), 0, revent, 0 }
    };

    _socket1->_is_POLLIN = false;
    _socket1->_is_POLLOUT = false;

    _socket2->_is_POLLIN = false;
    _socket2->_is_POLLOUT = false;

    int ret = zmq_poll (&_items [0], 2, timeout);
    if (ret <= 0)
      return false;

    if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLIN)
      {
        _socket1->_is_POLLIN = true;
      }

      if (_items [1].revents & ZMQ_POLLIN)
      {
        _socket2->_is_POLLIN = true;
      }
    }
    else if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLOUT)
      {
        _socket1->_is_POLLOUT = true;
      }

      if (_items [1].revents & ZMQ_POLLOUT)
      {
        _socket2->_is_POLLOUT = true;
      }
    }

    return ret > 0;
  }
};

class ZMQPoll3
{
private:
  ZMQSocket* _socket1;
  ZMQSocket* _socket2;
  ZMQSocket* _socket3;
public:
  ZMQPoll3(
    ZMQSocket* socket1,
    ZMQSocket* socket2,
    ZMQSocket* socket3
  ) :
      _socket1(socket1),
      _socket2(socket2),
      _socket3(socket3)
  {
  }

  bool poll(long timeout = -1, int revent = ZMQ_POLLIN)
  {
    zmq::pollitem_t _items[] = {
      { _socket1->zmqSocket(), 0, revent, 0 },
      { _socket2->zmqSocket(), 0, revent, 0 },
      { _socket3->zmqSocket(), 0, revent, 0 }
    };

    _socket1->_is_POLLIN = false;
    _socket1->_is_POLLOUT = false;

    _socket2->_is_POLLIN = false;
    _socket2->_is_POLLOUT = false;

    _socket3->_is_POLLIN = false;
    _socket3->_is_POLLOUT = false;

    int ret = zmq_poll (&_items [0], 3, timeout);
    if (ret <= 0)
      return false;

    if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLIN)
      {
        _socket1->_is_POLLIN = true;
      }

      if (_items [1].revents & ZMQ_POLLIN)
      {
        _socket2->_is_POLLIN = true;
      }

      if (_items [2].revents & ZMQ_POLLIN)
      {
        _socket3->_is_POLLIN = true;
      }
    }
    else if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLOUT)
      {
        _socket1->_is_POLLOUT = true;
      }

      if (_items [1].revents & ZMQ_POLLOUT)
      {
        _socket2->_is_POLLOUT = true;
      }

      if (_items [2].revents & ZMQ_POLLOUT)
      {
        _socket3->_is_POLLOUT = true;
      }
    }

    return ret > 0;
  }
};

class ZMQPoll4
{
private:
  ZMQSocket* _socket1;
  ZMQSocket* _socket2;
  ZMQSocket* _socket3;
  ZMQSocket* _socket4;
public:
  ZMQPoll4(
    ZMQSocket* socket1,
    ZMQSocket* socket2,
    ZMQSocket* socket3,
    ZMQSocket* socket4
  ) :
      _socket1(socket1),
      _socket2(socket2),
      _socket3(socket3),
      _socket4(socket4)
  {
  }

  bool poll(long timeout = -1, int revent = ZMQ_POLLIN)
  {
    zmq::pollitem_t _items[] = {
      { _socket1->zmqSocket(), 0, revent, 0 },
      { _socket2->zmqSocket(), 0, revent, 0 },
      { _socket3->zmqSocket(), 0, revent, 0 },
      { _socket4->zmqSocket(), 0, revent, 0 }
    };

    _socket1->_is_POLLIN = false;
    _socket1->_is_POLLOUT = false;

    _socket2->_is_POLLIN = false;
    _socket2->_is_POLLOUT = false;

    _socket3->_is_POLLIN = false;
    _socket3->_is_POLLOUT = false;

    _socket4->_is_POLLIN = false;
    _socket4->_is_POLLOUT = false;

    int ret = zmq_poll (&_items [0], 4, timeout);
    if (ret <= 0)
      return false;

    if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLIN)
      {
        _socket1->_is_POLLIN = true;
      }

      if (_items [1].revents & ZMQ_POLLIN)
      {
        _socket2->_is_POLLIN = true;
      }

      if (_items [2].revents & ZMQ_POLLIN)
      {
        _socket3->_is_POLLIN = true;
      }

      if (_items [3].revents & ZMQ_POLLIN)
      {
        _socket4->_is_POLLIN = true;
      }
    }
    else if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLOUT)
      {
        _socket1->_is_POLLOUT = true;
      }

      if (_items [1].revents & ZMQ_POLLOUT)
      {
        _socket2->_is_POLLOUT = true;
      }

      if (_items [2].revents & ZMQ_POLLOUT)
      {
        _socket3->_is_POLLOUT = true;
      }

      if (_items [3].revents & ZMQ_POLLOUT)
      {
        _socket4->_is_POLLOUT = true;
      }
    }

    return ret > 0;
  }
};


class ZMQPoll5
{
private:
  ZMQSocket* _socket1;
  ZMQSocket* _socket2;
  ZMQSocket* _socket3;
  ZMQSocket* _socket4;
  ZMQSocket* _socket5;
public:
  ZMQPoll5(
    ZMQSocket* socket1,
    ZMQSocket* socket2,
    ZMQSocket* socket3,
    ZMQSocket* socket4,
    ZMQSocket* socket5
  ) :
      _socket1(socket1),
      _socket2(socket2),
      _socket3(socket3),
      _socket4(socket4),
      _socket5(socket5)
  {
  }

  bool poll(long timeout = -1, int revent = ZMQ_POLLIN)
  {
    zmq::pollitem_t _items[] = {
      { _socket1->zmqSocket(), 0, revent, 0 },
      { _socket2->zmqSocket(), 0, revent, 0 },
      { _socket3->zmqSocket(), 0, revent, 0 },
      { _socket4->zmqSocket(), 0, revent, 0 },
      { _socket5->zmqSocket(), 0, revent, 0 }
    };

    _socket1->_is_POLLIN = false;
    _socket1->_is_POLLOUT = false;

    _socket2->_is_POLLIN = false;
    _socket2->_is_POLLOUT = false;

    _socket3->_is_POLLIN = false;
    _socket3->_is_POLLOUT = false;

    _socket4->_is_POLLIN = false;
    _socket4->_is_POLLOUT = false;

    _socket5->_is_POLLIN = false;
    _socket5->_is_POLLOUT = false;

    int ret = zmq_poll (&_items [0], 5, timeout);
    if (ret <= 0)
      return false;

    if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLIN)
      {
        _socket1->_is_POLLIN = true;
      }

      if (_items [1].revents & ZMQ_POLLIN)
      {
        _socket2->_is_POLLIN = true;
      }

      if (_items [2].revents & ZMQ_POLLIN)
      {
        _socket3->_is_POLLIN = true;
      }

      if (_items [3].revents & ZMQ_POLLIN)
      {
        _socket4->_is_POLLIN = true;
      }

      if (_items [4].revents & ZMQ_POLLIN)
      {
        _socket5->_is_POLLIN = true;
      }
    }
    else if (revent == ZMQ_POLLIN)
    {
      if (_items [0].revents & ZMQ_POLLOUT)
      {
        _socket1->_is_POLLOUT = true;
      }

      if (_items [1].revents & ZMQ_POLLOUT)
      {
        _socket2->_is_POLLOUT = true;
      }

      if (_items [2].revents & ZMQ_POLLOUT)
      {
        _socket3->_is_POLLOUT = true;
      }

      if (_items [3].revents & ZMQ_POLLOUT)
      {
        _socket4->_is_POLLOUT = true;
      }

      if (_items [4].revents & ZMQ_POLLOUT)
      {
        _socket5->_is_POLLOUT = true;
      }
    }

    return ret > 0;
  }
};

#endif	/* ZMQPOLL_H */

