# TIMESTAMP request

This code contains a client and server implementation, where the client can ask for the current timestamp from the server.

The example script creates 2 clients and 1 server (almost at the same time) to demonstrate how the application works. 

# Latency analysis

I tried analyzing the end-to-end latency using many clients and 1 server. There are 2 running scripts (measure_end_to_end
simply runs the experiment once). The second script (measure_many_end_to_end) runs 100 clients with 1 server and stores
the latency measurements to a csv.

I tried checking whether the bottleneck is the client-to-server, server-inter or server-to-client latency fragment.
The analysis is in the "analysis" directory.