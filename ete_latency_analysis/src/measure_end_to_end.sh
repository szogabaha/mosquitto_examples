# This sample script creates a server with a client.
# The client runs for a specified time (configured by the code).
# The output is continuously forwarded to a file within the output directory
# The server is not destroyed, so if it's run again, the clients might 
# receive multiple responses for the same request.

# Create output dir if it doesn't exist yet
output_dir="../output"
if [ ! -d "$output_dir" ]; then
    mkdir "$output_dir"
fi

# Start the server in the background
timeout 1000 ./server > /dev/null &


# stdbuf -oL is used to set the redirection buffering per line
stdbuf -oL ./client > "../output/end_to_end_measurement.csv" &

# Wait for all clients to finish
wait
