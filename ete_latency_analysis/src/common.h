/*
    This file contains the variables that the client and server entities share.
*/

#define PAYLOADSIZE 22
/* The channel on which the requests arrive to the server */
char* REQUEST_CHANNEL = "ntnu/szogabaha";

int SAMPLING_NUM = 600;

/*Broker related variables*/
char* HOST = "test.mosquitto.org";
int PORT = 1883;
int KEEPALIVE = 1000;


