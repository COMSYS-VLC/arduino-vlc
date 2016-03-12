#ifndef VLC_STATIONCONTROLLER_HPP
#define VLC_STATIONCONTROLLER_HPP


#include "VLCMAC.hpp"
#include "VLCPHY.hpp"
#include "TurnoutController.hpp"
#include "BLE.hpp"

/** Controller for defining the overall behavior of the station/track. */
class StationController {
public:
    /** Constructor */
    StationController();

    /** Starts the station run loop */
    void run() __attribute__((noreturn));

private:
    /**
     * Callback for completely received payloads via VLC.
     */
    static void payloadCallback(const uint8_t* payload, uint8_t len, void* data);
    /**
     * Callback for received acknowledgements via VLC.
     */
    static void ackCallback(uint8_t msgId, void* data);
    /**
     * Callback for completely received payloads via BLE.
     */
    static void BLECallback(uint8_t* payload, uint8_t len, void* data);

    /** PHY for station <-> train comm */
    VLCPHY mPhy;
    /** MAC for station <-> train comm */
    VLCMAC mMac;
    /** Controller for the turnout */
    TurnoutController mTurnout;
    /** BLE protocol and communication abstraction for comm with the app */
    BLE mBLE;

    /** Send the current velocity to the train and app */
    void sendVelocity();
    /** VLC message id of the currently being sent velocity update */
    uint8_t mVelocityMsgId;
    /** current velocity */
    uint8_t mVelocity;
    /** current direction */
    bool mForward;

    /** current turnout position */
    bool mTurnoutStraight;

    /** Send the current LED state to the train and app */
    void sendLEDstates();
    /** VLC message id of the currently being sent LED update */
    uint8_t mLEDstatesMsgId;
    /** "opaque" current LED state */
    uint8_t mLEDs[4];

    /** Send the current velocity to the app */
    void sendBLEVelocity();
    /** Send the current LED state to the app */
    void sendBLELEDStates();
    /** Send the current turnout state to the app */
    void sendBLETurnoutState();
};


#endif //VLC_STATIONCONTROLLER_HPP
