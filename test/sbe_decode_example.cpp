
#include <catch2/catch_all.hpp>
#include "ns2/MDIncrementalRefreshLimitsBanding50.h" // Example generated header


// following along with example provided at https://cmegroupclientsite.atlassian.net/wiki/spaces/EPICSANDBOX/pages/46181207/MDP+3.0+-+SBE+Decoding+Example


TEST_CASE("MDIncrementalRefreshLimitsBanding Message Parsing", "[MDIncrementalRefreshLimitsBanding]") {
    // example bufffer setup
    uint8_t buffer[] = {
        0xA6, 0xBB, 0x0A, 0x00, 0x5B, 0x19, 0x01, 0x72, 0x1E, 0xEF, 0xA9, 0x16, 0x38, 0x00, 0x0B, 0x00,
        0x32, 0x00, 0x01, 0x00, 0x09, 0x00, 0x4B, 0x52, 0xE8, 0x71, 0x1E, 0xEF, 0xA9, 0x16, 0x00, 0x00,
        0x00, 0x20, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x90, 0xCD, 0x79,
        0x2F, 0x08, 0x00, 0x00, 0x00, 0xE4, 0x0B, 0x54, 0x02, 0x00, 0x00, 0x00, 0xF4, 0x15, 0x00, 0x00,
        0x4D, 0x07, 0x00, 0x00
    };

    const uint64_t actingVersion = 13;

    // Encode the header 
    ns2::MessageHeader hdr;
    hdr.wrap((char*)buffer, 0, actingVersion, sizeof(buffer));


    // Wrap for decoding 
    int templateId = hdr.templateId();
    int actingBlockLength = hdr.blockLength();

    ns2::MDIncrementalRefreshLimitsBanding50 message;
    message.wrapForDecode((char*)buffer, hdr.size(), actingBlockLength, (uint64_t)sizeof(buffer));

    // Assertions
    CHECK(message.securityID() == 5620);
    CHECK(message.MsgSeqNum() == 703398);

}