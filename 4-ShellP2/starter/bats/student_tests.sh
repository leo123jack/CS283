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

# Test 1: Execute 'ls' with arguments '-l' to check for correct output
@test "External command 'ls -l'" {
    run ./dsh <<EOF
ls -l
EOF

    echo "Captured stdout: $output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
}

# Test 2: Sequential execution of two commands "ls" and "pwd"
@test "Multiple external commands 'ls' and 'pwd'" {
    run ./dsh <<EOF
ls; pwd
EOF

    echo "Captured stdout: $output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
}

# Test 3: Pipe between two external commands "ls | grep d"
@test "Piping between external commands 'ls' and 'grep'" {
    run ./dsh <<EOF
ls | grep d
EOF

    echo "Captured stdout: $output"
    echo "Exit Status: $status"

    [ "$status" -eq 0 ]
}

# Test 4: Echo with a quoted string
@test "It handles quoted spaces" {
    run "./dsh" <<EOF                
echo "hello     world"
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="helloworlddsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]

    [ "$status" -eq 0 ]
}

# Test 5: Exiting the shell
@test "Exit shell" {
    run "./dsh" <<EOF
exit
EOF

    # Check exit status
    echo "Exit Status: $status"

    # Assertions
    [ "$status" -eq 0 ]
}

# Test 6: Cd into invalid directory
@test "Change directory - invalid directory" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF                
cd /invalid/directory
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="cd:Nosuchfileordirectorydsh2>dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    # if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}