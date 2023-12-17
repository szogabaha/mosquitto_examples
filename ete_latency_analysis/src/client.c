#define _POSIX_C_SOURCE 199309L
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include <sys/types.h> //For the pid_t sturct
#include <time.h>


long published_at;
long response_received_at;
bool RECEIVED = false;


long _get_current_time() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Function to check whether the timeout has occurred
int is_timeout(clock_t start, clock_t timeout) {
    return (clock() - start) >= timeout;
}

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	if (reason_code != 0)
	{
		/* Error handling */
		mosquitto_disconnect(mosq);
	}

	/* obj contains the topic, that the client monitors for a response */
	int rc = mosquitto_subscribe(mosq, NULL, obj, 1);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}
}

/* PUBLISH request to the broker. Response is arriving on the "id" topic */
void publish_timestamp_request(struct mosquitto *mosq, char *id)
{

	published_at = _get_current_time();
	int rc = mosquitto_publish(mosq, NULL, REQUEST_CHANNEL, strlen(id), id, 0, false);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

/* Print reseived timestamp */
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	response_received_at = _get_current_time();
	RECEIVED = true;

	/*Get the response timestamp and the current timetamp*/
	char *received_timestamp = msg->payload;
	printf("%ld\t%s\t%ld\n", published_at, received_timestamp, response_received_at);
}

// Generate random 8 long id. This is what we pass as a payload to the server
void generate_id8(char *randomCharArray)
{
	// Seed the random number generator with the pid
	// (so that 2 clients created at the same time will have different ids)
	pid_t processID = getpid();
	srand((unsigned int)processID + time(NULL));

	// Generate random nunber
	int randomNum = rand() % 10000000 + 1000000; // 7 digits + 1 for the null terminator,  +6M to start with at least 1
	snprintf(randomCharArray, sizeof(randomCharArray), "%d", randomNum);
}

int main(int argc, char *argv[])
{
	char id[8];
	generate_id8(id);
	struct mosquitto *mosq;
	int rc;

	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, id);
	if (mosq == NULL)
	{
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	/* Connect to test.mosquitto.org on port 1883, with a keepalive of X seconds. */
	rc = mosquitto_connect(mosq, HOST, PORT, KEEPALIVE);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

    int loop_interval = 1; // in seconds

    for (int i=0; i < SAMPLING_NUM; i++) { //Run as long as many stamps we need
		// Send request in every cycle
		publish_timestamp_request(mosq, id);


		// Wait until message arrives or QoS 0: 1 sec timeout
		clock_t start_time = clock();
		clock_t timeout_duration = CLOCKS_PER_SEC*5; // 5 second timeout
		while (!RECEIVED && !is_timeout(start_time, timeout_duration))
		{ //Wait until response
		}
		
        // Sleep for the loop interval
        sleep(loop_interval);
		RECEIVED = false;
	}
	
	mosquitto_lib_cleanup();
	return 0;
}
