/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
 
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.
    4. This software is subject to the additional restrictions placed on the
       Zigbee Specification's Terms of Use.
    
    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 
    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file constants.h
    \ingroup misc
    \brief FreakZ stack constants

    This file contains many of the constants used in the stack.
*/
/*******************************************************************/
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SIM_ENV 1       ///< Define whether the sim environment is being used or not

// Zigbee 2006 APS Defined Constants
#define APS_MAX_FRAME_RETRIES       3   ///< Max frame retries at the APS level
/// Define amount of time to wait in seconds for an ACK before timeout.
/// Curr value is dummy value. Here is the real value: 0.05 * (2 * nwkcMaxDepth)
#define APS_ACK_WAIT_DURATION       2   

// Zigbee 2007 NWK Defined constants
#define NWK_COORDINATOR_CAPABLE     true                ///< Device is capable of being coordinator 
#define NWK_DEFAULT_SECURITY_LEVEL  ENC_MIC_64  ///< Default security level
#define NWK_DISCOVERY_RETRY_LIMIT   0x03        ///< Number of times to retry network discovery
#define NWK_MIN_HDR_OVERHEAD        0x08        ///< Minimum num bytes for NWK header
//0x2AF8 - add one second to the actual          
// value to reflect uncertainty in the timer    
#define NWK_RTE_DISC_TIME           0x7D0       ///< Time allowed for route discovery before operation is ended
#define NWK_BRC_JITTER              0x40        ///< Max value of broadcast jitter
#define NWK_PASSIVE_ACK_TIMEOUT     1000        ///< Timeout value for passive acks during broadcast
#define NWK_MAX_BRC_RETRIES         0x03        ///< Number of broadcast retries
#define NWK_RREQ_RETRIES            0x03        ///< Number of route request retries
#define NWK_RREQ_RETRY_INTERVAL     0xFE        ///< Time in between route request retries
#define NWK_MIN_RREQ_JITTER         0x01        ///< Min time of rreq jitter
#define NWK_MAX_RREQ_JITTER         0x40        ///< Max time of rreq jitter

// 802.15.4 MAC Defined constants
// Tick value is 1 msec so all time values are converted to units of msec

/*! 802.15.4 defined slot duration. Used to calculate other values in MAC layer.
60 symbols = 960 usec ~ 1 msec*/
#define aBaseSlotDuration           1   
#define aBaseSuperframeDuration     (aBaseSlotDuration * aNumSuperframeSlots)  ///< Length of superframe
#define aGTSDescPersistenceTime     4                   ///< Unused in FreakZ 
#define aMaxBeaconOverhead          75                  ///< Unused in FreakZ 
#define aMaxBeaconPayloadLength     (aMaxPHYPacketSize - aMaxBeaconOverhead) ///< Max payload length for beacons 
#define aMaxLostBeacons             4                   ///< Unused in FreakZ 
#define aMaxMACPayloadSize          (aMaxPHYPacketSize - aMinMPDUOverhead)      ///< Max payload length for MAC frames
#define aMaxMPDUUnsecuredOverhead   25  ///< Unused in FreakZ 
#define aMaxSIFSFrameSize           18  ///< Unused in FreakZ 
#define aMinCAPLength               7   ///< 440 symbols = 7040 usec ~7 msec
#define aMinMPDUOverhead            9   ///< Min MAC header size
#define aNumSuperframeSlots         16  ///< Unused in FreakZ 
#define aUnitBackoffPeriod          1   ///< 20 symbols = 320 usec...round to 1 msec
#define aMinLIFSPeriod              40  ///< Unused in FreakZ 
#define aminSIFSPeriod              12  ///< Unused in FreakZ 
#define aMacAckWaitDuration                     2   ///< 54 symbols = 864 usec ~ 1 msec...round to 2 msec
#define aMacMaxFrameTotalWaitTime   20  ///< 1220 symbols = 19500 usec ~ 20 msec
#define aMacResponseWaitTime        32  ///< default value = 32 * aBaseSlotDuration ~ 32 msec
#define aMacMaxFrameRetries         3   ///< Number of frame retries before we fail the transmission
#define aMaxCsmaBackoffs            5   ///< Max number of CSMA backoffs before we fail the transmission
#define aMinBE                      3   ///< Minimum backoff exponent for calculating CSMA backoff time

// 802.15.4 PHY Defined constants
#define aMaxPHYPacketSize           127 ///< Actual size of 802.15.4 frame
#define aPhyChannelPage             0   ///< Unused in FreakZ 
//#define aPhyChannelsSupported       0x07FFF800  // channels supported by 802.15.4
#define aPhyChannelsSupported       0x00000800  ///< Channels supported by 802.15.4
#define aPhySymbolsPerOctet         2   ///< Takes 2 symbols to produce a byte @ 2.4 GHz OQPSK
#define aPhySHRDuration             10  ///< Each SHR is 5 bytes or 10 symbols (4 bytes Preamble + 1 byte SFD) or 1 tick. Use the value of 2 for safety.
/// Max frame duration for an 802.15.4 frame
#define aPhyMaxFrameDuration        (aPhySHRDuration + ((aMaxPHYPacketSize + 1) * aPhySymbolsPerOctet))

// max data payload sizes
#define MAX_MAC_PAYLOAD             118     ///< Max MAC payload size 127 - 9
#define MAX_NWK_PAYLOAD             110     ///< Max NWK payload size 127 - 9 - 8
#define MAX_APS_PAYLOAD             99      ///< Max Application payload size 127 - 9 - 8 - 10 (currently 1 less...need to find out why)
#define DEFAULT_SCAN_DURATION       10      ///< Default time to wait before timing out a network or energy scan

// Stack constants - These parameters are defined in the Zigbee Profile Spec
// TODO: Move these to a separate file
#define ZIGBEE_PROTOCOL_ID          0           ///< Protocol ID for this stack
#define ZIGBEE_STACK_PROFILE        1       ///< Stack profile
#define ZIGBEE_PROF_ID              1       ///< Profile ID
#define ZIGBEE_PROTOCOL_VERSION     2       ///< Zigbee protocol version
#define ZIGBEE_NWK_SECURITY_LEVEL   5       ///< Network security level
#define ZIGBEE_MAX_DEPTH            3       ///< Max network depth
#define ZIGBEE_MAX_CHILDREN         4       ///< Max child devices
#define ZIGBEE_MAX_ROUTERS          4       ///< Max router devices
#define ZIGBEE_MAX_NWK_BRC_RECORDS  10      ///< Max number of broadcast records
#define ZIGBEE_PASSIVE_ACK_TIMEOUT  CLOCK_SECOND ///< Passive ack timeout ~ 1 sec
#define ZIGBEE_DEVICE_TYPE          NWK_ROUTER  ///< Default device type
#define ZIGBEE_BRC_EXPIRY           3           ///< Time in seconds before you expire a broadcast entry
#define ZIGBEE_DEFAULT_RADIUS       6       ///< Radius value for frames
#define ZIGBEE_POLL_INTERVAL        8       ///< Polling interval
#define ZIGBEE_CONFIRM_INTERVAL     5       ///< Confirmation timeout
#define ZIGBEE_INVALID_HANDLE       0       ///< Invalid handle value
#define ZIGBEE_INVALID_EP           241     ///< Invalid endpoint value

// descriptors
#define ZIGBEE_PRIM_TRUST           0           ///< Primary trust center
#define ZIGBEE_BACKUP_TRUST         0       ///< Backup trust center
#define ZIGBEE_PRIM_BINDING         0       ///< Primary binding cache
#define ZIGBEE_BACKUP_BINDING       0       ///< Backup binding cache
#define ZIGBEE_PRIM_DISCOVERY       0       ///< Primary discovery cache
#define ZIGBEE_BACKUP_DISCOVERY     0       ///< Backup discovery cache
#define ZIGBEE_NWK_MANAGER          0       ///< Network manager
#define ZIGBEE_MAX_USER_DESC_MSG    20      ///< Max string size for user descriptor

// Capability Info
#define ZIGBEE_MAINS_POWERED        1                    ///< Mains powered - must be 1 if its a router.
#define ZIGBEE_RX_ON_WHEN_IDLE      ZIGBEE_MAINS_POWERED    ///< Rx on when idle - rx will be on when idle if its mains powered. otherwise not.
#define ZIGBEE_HIGH_SECURITY        0                    ///< High security is currently not supported

// Node descriptor
#define ZIGBEE_COMPLEX_DESC_AVAIL   0           ///< Complex descriptor present
#define ZIGBEE_USER_DESC_AVAIL      0       ///< User descriptor present
#define ZIGBEE_MANUF_CODE           0       ///< Manufacturer code
#define ZIGBEE_MAX_BUFSIZE          80      ///< Maximum buffer size
#define ZIGBEE_SERVER_MASK          0       ///< Server mask
#define ZIGBEE_MAX_IN_XFER          MAX_APS_PAYLOAD ///< Node descr - max size of in apdu payload           
#define ZIGBEE_MAX_OUT_XFER         MAX_APS_PAYLOAD ///< Node descr - max size of out apdu payload     
#define ZIGBEE_DESC_CAPABILITY      0           ///< Capabilities

#define ZIGBEE_MAX_ENDPOINTS        10          ///< Max endpoints for this device
#define ZIGBEE_MIN_BUFS_NEEDED      1           ///< Minimum number of frame buffers required

// app layer rx defines
#define END_MARKER_CLUST            0xFFFC      ///< Mark the end of a cluster

// misc
#define INVALID_NWK_ADDR            0xFFFF                 ///< Invalid network address value
#define INVALID_EXT_ADDR            0xFFFFFFFFFFFFFFFFULL   ///< Invalid extended address value

#endif // CONSTANTS_H


