/* -*- mode: c++ ; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */
/*
  Modeling CRC-8 error detection rates for burst insertions into ISM-band transmissions

  Vary the 'init' value to determine if there is a "best" value for 'init'.

  This variant forces a block of 4 bytes of 0's to start the otherwise random message.

  The 80-bit message data payload is 72 randomized bits; the last byte is CRC-8.
  
  Insert a block of 1 to 8 0's and then 1's into the 80-bit message as a block 
  pattern that starts at bit (leftmost) and marches across the message bits until 
  only the left-most bit of the insertion is affecting the 80th bit of the message.

  That is, the effect of the insertion block on CRC-8 error detection is tested
  for every contiguous block within the message.
  
  Average the results over 100 randomized message packets.
  
  Some block insertions don't change the packet bit pattern, so exclude those
  from effectiveness calculations but report as "NoEffect".
  
  Of the changed patterns, recompute the CRC-8 of the changed pattern and
  compare with the CRC-8 of the original message to determine if
  burst insertion would have been detected.

  Report for each possible value of 'init' the count and % of
  corrupted packets that were missed and were detected
  when blocks of either 0's or 1's were inserted into the message.
  
  hdtodd@gmail.com, 2025.05.01
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "crc8.h"

#define DEBUG false

#define MaxErrBurst 8  // not designed to work for bursts >8
#define MsgSize  10
#define REPEATS 100

typedef struct {
    int noEffect;
    int foundError[2];      // Track separately how many 0 & 1 block
    int missedError[2];   // insertions were found by CRC8 
} stats;

void checkBursts(uint8_t init, uint8_t *msg, int burstSize, stats *result);

int main(void) {
    uint8_t init = 0x00;
    uint8_t msg[MsgSize];
    stats results[MaxErrBurst+1];
    
    printf("Testing CRC8 error detection of block insertions into ISM messages");

    for (int inits=0; inits<256; inits++) {
    init = (uint8_t) inits;
    printf("\n------------------------------------------------------------------");
    printf(  "------------------------------------------------------------------");
    
    // Clear the results tables
    for (int i=0; i<=MaxErrBurst; i++) {
        results[i].noEffect=0;
        results[i].foundError[0]=0; results[i].foundError[1]=0;
        results[i].missedError[0]=0; results[i].missedError[1]=0;
    };
    
    for (int burstSize=1; burstSize<=MaxErrBurst; burstSize++) {
        if (DEBUG) {
            printf("\nTesting with error burst block size = %d\n", burstSize);
            printf("Repeatting test with %d randomized message payloads\n", REPEATS);
        };
        for (int iteration = 0; iteration<REPEATS; iteration ++) {
            // Generate a message block of random bits of size MsgSize-1
            // Use the middle 8 bits of a 32-bit random #
            for (int i=0; i<4; i++) msg[i] = 0x00;
            for (int i=4; i<MsgSize-1; i++) msg[i] = (uint8_t) ((arc4random()>>12) & 0xFF);
            msg[MsgSize-1] = crc8(msg, 9, init);

            // See if the generated burst would be detected if it were all 0's or all 1's
            //   and record the results for tabulation
            checkBursts(init, msg, burstSize, &results[burstSize]);
        };
    };
    printf("\n\nResults:\n");
    printf("                             --------Missed--------     --------Detected-------");
    printf("   -------% Missed-------     ------% Detected-------\n");
    printf("'init'  BurstSize  Noeffect  0's block    1's block     0's block     1's block");
    printf("   0's block    1's block     0's block     1's block\n");
    for (int burstSize=1; burstSize<=MaxErrBurst; burstSize++) {
        printf(" 0x%02x    %3d         %4d        %4d         %4d          %4d          %4d",
               init, burstSize, results[burstSize].noEffect,
               results[burstSize].missedError[0], results[burstSize].missedError[1],
               results[burstSize].foundError[0], results[burstSize].foundError[1]);
        int total0Corrupted = results[burstSize].missedError[0] + results[burstSize].foundError[0];
        int total1Corrupted = results[burstSize].missedError[1] + results[burstSize].foundError[1];
        printf("      %3d%%         %3d%%          %3d%%          %3d%%\n",
               (int) (100.0*results[burstSize].missedError[0]/total0Corrupted + 0.5),
               (int) (100.0*results[burstSize].missedError[1]/total1Corrupted + 0.5),
               (int) (100.0*results[burstSize].foundError[0]/total0Corrupted + 0.5),
               (int) (100.0*results[burstSize].foundError[1]/total1Corrupted + 0.5)
               );
    };
    };
};

/*  checkBursts()
    Given
      -  an initial CRC8 remainder, "init",
      -  a message with 9 bytes of payload and 1 byte CRC8 checksum,
      -  a burst of length "burstSize" of 0's and then 1's to be
         inserted into the message,
   this procedure inserts the burst into the message and recomputes
   the CRC8 checkbyte.

   -   If the burst did not change the message, it is recorded as
       a "noEffect";
   -   if the error burst WAS NOT detected because the CRC8 byte
       computed from the corrupted message matched the original
       CRC8 byte, the corrupted message is recorded as "";
   -   if the error WAS detected, it is recorded as "".

   Results of the series of tests across the message returned
   in "result"
*/
void checkBursts(uint8_t init, uint8_t *msg, int burstSize, stats *result) {
    uint8_t cmsg[MsgSize+1];  // extra byte for masking loop
    uint8_t check, bursth, burstl;
    uint16_t burst, burstMaster;
    bool noEffect;

    //  Make a copy of the message for corrupting
    memcpy(cmsg, msg, MsgSize);

    // Create error burst block of length 'burstSize' and left-align it
    burst = 0;
    for (int i=0; i<burstSize; i++) burst = (burst<<1) | 0x01;
    burst <<= (16-burstSize);
    burstMaster = burst;
    if (DEBUG) printf("Initial error burst mask = 0b%016b = 0x%04x\n", burst, burst);

    /*  Test for error detection of the burst and then shift the mask right
        by one bit for the next iteration.

        Test for the effect of insertion of a 1's burst and of a 0's burst
        for each block position in the message.

    */
    if (DEBUG) printf("Now shift the mask right across the 80-bit message\n");
    for (int bitPos=0; bitPos<MsgSize*8; bitPos++) {
        uint8_t byteNum;
        bursth = (uint8_t) (burst>>8);
        burstl = (uint8_t) (burst & 0x00ff);
        byteNum = bitPos/8;
        if (DEBUG) {
            printf("Mask left bit position %d, burst = 0x%04x, bursth = 0x%02x, burstl = 0x%02x, burstMaster = 0x%04x\n",
                   bitPos, burst, bursth, burstl, burstMaster);
            printf("\tMask byte %d with %08b", byteNum, bursth);
            if (burstl == 0) {
                printf("\n");
            } else {
            printf(", and byte %d with %08b\n", byteNum+1, burstl);
            };
        };

        /* In practice, the receiver discards the message if the CRC computed from
           the message payload doesn't match the CRC received in the message itself.
           Emulate that behavior in counting success/failure in this simulation.
           
           Corrupt the copy at bytes 'byteNum' & maybe 'byteNum'+1 with the bitmasks
           -  set the block first to 0's then to 1's to test both error patterns
           -  for each error pattern, recompute the checksum of the DATA bytes in the corrupted message
           -  count as "noEffect" if the error block doesn't change the message
           -  count as error "missed" if the CRCs of original and corrupted messages
              are EQUAL even though the message was corrupted
           -  count as error "found" if the CRCs of original message and
              corrupted message are NOT EQUAL 
        */

        if (DEBUG) printf("bursth = %08b, ~bursth = %08b, msg[byteNum] = %08b, & ~ = %08b\n",
               bursth, (uint8_t) (~bursth), msg[byteNum], msg[byteNum] & (uint8_t) (~bursth));

        // Insert 0's block and check
        cmsg[byteNum] = msg[byteNum] & (uint8_t) (~bursth);
        if (burstl != 0 ) cmsg[byteNum+1] = msg[byteNum+1] & (uint8_t) (~burstl);
        noEffect = (cmsg[byteNum]==msg[byteNum]) && (cmsg[byteNum+1]==msg[byteNum+1]);
        check = crc8(cmsg, 9, init);
        if (DEBUG) {
            printf("Original msg:                 0b");
            for (int k=0; k<MsgSize; k++) printf("%08b", msg[k]);
            printf("\n");
            printf("Corrupted msg with 0's block: 0b");
            for (int k=0; k<MsgSize; k++) printf("%08b", cmsg[k]);
        }
        if (noEffect) {
            result->noEffect++;
            if (DEBUG) printf(" NO EFFECT\n");
        };
        if (!noEffect) {
            if (check==msg[9]) {
                result->missedError[0]++;
                if (DEBUG) printf(" FAILED\n");
            } else {
                result->foundError[0]++;
                if (DEBUG) printf(" FOUND\n");
            };
        };

        // Insert 1's block and check
        cmsg[byteNum] = msg[byteNum] | bursth;
        if (burstl != 0) cmsg[byteNum+1] = msg[byteNum+1] | burstl;
        noEffect = (cmsg[byteNum]==msg[byteNum]) && (cmsg[byteNum+1]==msg[byteNum+1]);
        check = crc8(cmsg, 9, init);
        if (DEBUG) {
            printf("Corrupted msg with 1's block: 0b");
            for (int k=0; k<MsgSize; k++) printf("%08b", cmsg[k]);
        };
        if (noEffect) {
            result->noEffect++;
            if (DEBUG) printf(" NO EFFECT\n");
        };
        if (!noEffect) {
            if (check==msg[9]) {
                result->missedError[1]++;
                if (DEBUG) printf(" FAILED\n");
            } else {
                result->foundError[1]++;
                if (DEBUG) printf(" FOUND\n");
            };
        };

        // restore the "corrupted bytes" from msg
        cmsg[byteNum]   = msg[byteNum];
        cmsg[byteNum+1] = msg[byteNum+1];

        // Done with testing for this mask position; prep for next iteration
        // Shift the mask right one bit, but reset at the end of a byte
        burst = (bitPos%8 == 7) ? burstMaster : burst >> 1;
    };
};
