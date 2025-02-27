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

# Test 1: Pipeline execution
@test "Execute a pipeline" {
    run ./dsh <<EOF
echo hello | grep hello
EOF

    echo "Captured stdout: $output"
    echo "Exit Status: $status"

    stripped_output=$(echo "$output" | head -n 1)
    [ "$status" -eq 0 ]
    [ "$stripped_output" = "hello" ]
}

# Test 2: Pipeline with multiple commands
@test "Execute a pipeline with multiple commands" {
    run ./dsh <<EOF
echo hello | grep hello | wc -c
EOF

    echo "Captured stdout: $output"
    echo "Exit Status: $status"

    stripped_output=$(echo "$output" | head -n 1)
    [ "$status" -eq 0 ]
    [ "$stripped_output" -eq 6 ]
}

# Test 3: Pipeline with invalid command
@test "Handle pipeline with invalid command" {
    run "./dsh" <<EOF
echo hello | invalid_command
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="execvp:Nosuchfileordirectorydsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

# Test 4: Pipeline with too many commands
@test "Handle pipeline with too many commands" {
    run "./dsh" <<EOF
echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7 | echo 8 | echo 9
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="error:pipinglimitedto8commandsdsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

# Test 5: Command with mixed quotes
@test "Handle command with mixed quotes" {
    run "./dsh" <<EOF
echo 'hello' | grep hello
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="'hello'dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}
