/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000002LL, 0x0000000003LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

//
// Maze
//
/*unsigned char maze[4][5] =
{
  3, 3, 3, 3, 3,
  3, 1, 1, 1, 3,
  3, 2, 0, 1, 2,
  3, 1, 3, 1, 3
};*/


void setup(void)
{
  //
  // Print preamble
  //

  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("* PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    unsigned char new_data;
    unsigned char pos = 4;
    unsigned char state = 0;
    //int start = 0;
    
    new_data =  pos << 3 | state;

    /*if (start == 0) {
      //start = 1;

      // Send the maze in a single payload
      printf("Now sending the maze!\n");
      bool ok = radio.write( maze, sizeof(maze) );
      
      if (ok)
        printf("ok...");
      else
        printf("failed.\n\r");
    }*/

    // Take the time, and send it.  This will block until complete
    printf("Now sending new map data\n");
    bool update = radio.write( &new_data, sizeof(unsigned char) );

     if (update)
      printf("ok update...");
    else
      printf("failed update.\n\r");


    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      printf("Success!");
//      // Grab the response, compare, and send to debugging spew
//      unsigned long got_time;
//      radio.read( &got_time, sizeof(unsigned long) );
//
//      // Spew it
//      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      //unsigned char got_maze[4][5];
      unsigned char got_data;
      //unsigned char start = 0;
      //bool done = false;
      bool doneU = false;
      
      /*while (!done)
      {
        
        // Fetch the payload.
        done = radio.read( got_maze, sizeof(got_maze) );

        // Print the maze
        for (int i=0; i < 4; i++) {
          for (int j=0; j < 5; j++) {
            printf("%d ", got_maze[i][j]);
          }
          printf("\n");
        }

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }*/

      while (!doneU){
        // Fetch the payload, and see if this was the last one.
        doneU = radio.read( &got_data, sizeof(unsigned char) );

        
        
        bitRead(got_data, 0) ? digitalWrite(0, HIGH) : digitalWrite(0, LOW);
        bitRead(got_data, 1) ? digitalWrite(1, HIGH) : digitalWrite(1, LOW);
        bitRead(got_data, 2) ? digitalWrite(2, HIGH) : digitalWrite(2, LOW);
        bitRead(got_data, 3) ? digitalWrite(3, HIGH) : digitalWrite(3, LOW);
        bitRead(got_data, 4) ? digitalWrite(4, HIGH) : digitalWrite(4, LOW);
        bitRead(got_data, 5) ? digitalWrite(5, HIGH) : digitalWrite(5, LOW);
        bitRead(got_data, 6) ? digitalWrite(6, HIGH) : digitalWrite(6, LOW);
        bitRead(got_data, 7) ? digitalWrite(7, HIGH) : digitalWrite(7, LOW);


        //got_maze[x][y] = location;

        // Print the maze

        /*printf("Updated Maze \n");
        for (int i=0; i < 4; i++) {
          for (int j=0; j < 5; j++) {
            printf("%d ", got_maze[i][j]);
          }
          printf("\n");
        }*/
      }
      

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      //radio.write( got_maze, sizeof(got_maze) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("* CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("* CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp