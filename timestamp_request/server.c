/*
 * Server that gives the current timestamp for clients. It listens on one topic and returns the timestamp
 *  On the topic that the client requests to use.
 */

#define _POSIX_C_SOURCE 199309L
#include <mosquitto.h>
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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

/* get timestamp */
void get_timestamp(char *ts_array, size_t size)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	// Convert tv_sec and tv_nsec to a character array
	snprintf(ts_array, size, "%ld", ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

/* publish responstime to the corresponding topic */
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{

	char *topic_to_use = msg->payload;
	char payload[PAYLOADSIZE]; // 20 bytes long payload
	int rc;

	get_timestamp(payload, PAYLOADSIZE); // 20 bytes long payload

	/* Publish the message
	 * msg contains the topic to use
	 */
	rc = mosquitto_publish(mosq, NULL, topic_to_use, strlen(payload), payload, 2, false);
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
