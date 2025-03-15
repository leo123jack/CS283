#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}



# Helper function to start the server
start_server() {
    ./dsh -s -i 0.0.0.0 -p 1234 &
    SERVER_PID=$!
    sleep 1  # Give the server time to start
}

# Helper function to stop the server
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill -TERM $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null || true
    fi
}

setup() {
    start_server
}

teardown() {
    stop_server
}

# Test 1: Check if the server starts successfully
@test "Server starts successfully" {
    run pgrep -f "./dsh -s"
    echo "Server PID: $SERVER_PID"
    echo "pgrep output: $output"
    echo "pgrep status: $status"
    [ "$status" -eq 0 ]
}

# Test 2: Client connects to server
@test "Client connects to server" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
exit
EOF

    echo "Client connection output: $output"
    echo "Client connection status: $status"
    [ "$status" -eq 0 ]
}

# Test 3: Execute 'ls' command on the server
@test "Execute 'ls' command on the server" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
ls
exit
EOF

    echo "'ls' command output: $output"
    echo "'ls' command status: $status"
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh_cli.c"* ]]
}

# Test 4: Execute 'cd' command on the server
@test "Execute 'cd' command on the server" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
cd /tmp
pwd
exit
EOF

    echo "'cd' command output: $output"
    echo "'cd' command status: $status"
    [ "$status" -eq 0 ]
    [[ "$output" == *"/tmp"* ]]
}

# Test 5: Stop the server using 'stop-server' command
@test "Stop the server using 'stop-server' command" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
stop-server
EOF

    echo "Stop-server command output: $output"
    echo "Stop-server command status: $status"
    [ "$status" -eq 0 ]

    run pgrep -f "./dsh -s"
    echo "pgrep output after stop-server: $output"
    echo "pgrep status after stop-server: $status"
    [ "$status" -ne 0 ]
}


# Test 6: Execute pipeline command on the server
@test "Execute pipeline command on the server" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
ls | grep dsh
exit
EOF

    echo "Pipeline command output: $output"
    echo "Pipeline command status: $status"
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh_cli.c"* ]]
}

# Test 7: Execute multiple commands on the server
@test "Execute multiple commands on the server" {
    run ./dsh -c -i 127.0.0.1 -p 1234 <<EOF
ls
pwd
exit
EOF

    echo "Multiple commands output: $output"
    echo "Multiple commands status: $status"
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh_cli.c"* ]]
    [[ "$output" == *$(pwd)* ]]
}