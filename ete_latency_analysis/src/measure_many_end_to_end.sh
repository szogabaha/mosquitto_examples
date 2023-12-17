# This  script creates a server and 50 clients concurrently,
# then it concatenates the clients' output and saves it to a file.
# The server is not destroyed, so if it's run again, the clients might 
# receive multiple responses for the same request.

# Start the server in the background
timeout 1000 ./server > /dev/null &

# Create output dir if it doesn't exist yet
output_dir="../output"
if [ ! -d "$output_dir" ]; then
    mkdir "$output_dir"
fi

# Create a temporary directory for client outputs
tmpdir="${output_dir}/tmp"
if [ ! -d "$tmpdir" ]; then
    mkdir "${tmpdir}"
fi

# Run 10 clients concurrently and redirect their output to separate files

for i in {1..10}; do
  stdbuf -oL ./client > "${tmpdir}/c${i}" &
  sleep 1
done

# Wait for all clients to finish
wait

# Concatenate the output of all clients into a single file
cat "${tmpdir}/c"* > ${output_dir}/many_end_to_end_measurements.csv

# Clean up temporary directory
# rm -r "${tmpdir}"
