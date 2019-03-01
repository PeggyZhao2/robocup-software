#pragma once

#include <protobuf/RadioRx.pb.h>
#include <protobuf/RadioTx.pb.h>

#include <mutex>
#include <queue>

/**
 * @brief Sends and receives information to/from our robots.
 *
 * @details This is the abstract superclass for USBRadio and SimRadio, which do
 * the actual work - this just declares the interface.
 */
class Radio {
public:
    Radio() { _channel = 0; }

    virtual bool isOpen() const = 0;
    virtual void send(Packet::RadioTx& packet) = 0;
    virtual void receive() = 0;

    virtual void switchTeam(bool blueTeam) = 0;

    virtual void channel(int n) { _channel = n; }

    int channel() const { return _channel; }

    bool hasReversePackets() {
        std::lock_guard<std::mutex> lock(_reverse_packets_mutex);
        return _reversePackets.size();
    }

    inline const Packet::RadioRx popReversePacket() {
        std::lock_guard<std::mutex> lock(_reverse_packets_mutex);
        Packet::RadioRx packet = std::move(_reversePackets.front());
        _reversePackets.pop();
        return packet;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(_reverse_packets_mutex);

        // Clear the reverse packets queue by copy-swap
        std::queue<Packet::RadioRx> cleared;
        std::swap(cleared, _reversePackets);
    }

protected:
    // A queue for the reverse packets as they come in through libusb.
    // Access to this queue should be controlled by locking the mutex.
    std::queue<Packet::RadioRx> _reversePackets;
    std::mutex _reverse_packets_mutex;

    int _channel;
};
