/*
    This file contains the variables that the client and server entities share.
*/

#define PAYLOADSIZE 20
/* The channel on which the requests arrive to the server */
char* REQUEST_CHANNEL = "ntnu/szogabaha";

/*Broker related variables*/
char* HOST = "test.mosquitto.org";
int PORT = 1883;
int KEEPALIVE = 60;

