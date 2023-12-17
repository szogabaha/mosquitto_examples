
#define _POSIX_C_SOURCE 199309L
#include <mosquitto.h>
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

long _get_current_time() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if (reason_code != 0)
	{
		mosquitto_disconnect(mosq);
	}

	/* Subscribe to the common topic, that the requests arrive on */
	rc = mosquitto_subscribe(mosq, NULL, REQUEST_CHANNEL, 1);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}
}

/* publish on_receive and resp_sent_at timestamps to the requested topic */
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	long request_received_at = _get_current_time();
	char *topic_to_use = msg->payload;
	char payload[PAYLOADSIZE];
	snprintf(payload, PAYLOADSIZE, "%ld", request_received_at);
	
	// Call _get_current_time() again
	long response_sent_at = _get_current_time();

	// Append the result to the existing payload
	snprintf(payload + strlen(payload), PAYLOADSIZE - strlen(payload), "\t%ld", response_sent_at);
	
	//Send response to client
	int rc;
	rc = mosquitto_publish(mosq, NULL, topic_to_use, strlen(payload), payload, 0, false);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

int main(int argc, char *argv[])
{
	struct mosquitto *mosq;
	int rc;

	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	/* Create a new client instance. */
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
	{
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	/* Connect to test.mosquitto.org on port 1883 */
	rc = mosquitto_connect(mosq, HOST, PORT, KEEPALIVE);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a blocking call.*/
	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_lib_cleanup();
	return 0;
}
