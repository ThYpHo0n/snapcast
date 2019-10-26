/***
    This file is part of snapcast
    Copyright (C) 2014-2019  Johannes Pohl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#ifndef STREAM_SESSION_H
#define STREAM_SESSION_H

#include "common/queue.h"
#include "message/message.hpp"
#include "streamreader/stream_manager.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>


using boost::asio::ip::tcp;


class StreamSession;


/// Interface: callback for a received message.
class MessageReceiver
{
public:
    virtual void onMessageReceived(StreamSession* connection, const msg::BaseMessage& baseMessage, char* buffer) = 0;
    virtual void onDisconnect(StreamSession* connection) = 0;
};


/// Endpoint for a connected client.
/**
 * Endpoint for a connected client.
 * Messages are sent to the client with the "send" method.
 * Received messages from the client are passed to the MessageReceiver callback
 */
class StreamSession
{
public:
    /// ctor. Received message from the client are passed to MessageReceiver
    StreamSession(MessageReceiver* receiver, tcp::socket&& socket);
    ~StreamSession();
    void start();
    void stop();

    /// Sends a message to the client (synchronous)
    bool send(const msg::message_ptr& message);

    /// Sends a message to the client (asynchronous)
    void sendAsync(const msg::message_ptr& message, bool sendNow = false);

    bool active() const;

    /// Max playout latency. No need to send PCM data that is older than bufferMs
    void setBufferMs(size_t bufferMs);

    std::string clientId;

    std::string getIP()
    {
        return socket_.remote_endpoint().address().to_string();
    }

    void setPcmStream(PcmStreamPtr pcmStream);
    const PcmStreamPtr pcmStream() const;

protected:
    void socketRead(void* _to, size_t _bytes);
    void getNextMessage();
    void reader();
    void writer();

    mutable std::mutex activeMutex_;
    std::atomic<bool> active_;

    std::unique_ptr<std::thread> readerThread_;
    std::unique_ptr<std::thread> writerThread_;
    mutable std::mutex socketMutex_;
    tcp::socket socket_;
    MessageReceiver* messageReceiver_;
    Queue<std::shared_ptr<msg::BaseMessage>> messages_;
    size_t bufferMs_;
    PcmStreamPtr pcmStream_;
};



#endif
