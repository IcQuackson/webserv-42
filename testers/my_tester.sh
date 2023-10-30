#!/bin/bash

function test_request() {
    local description=$1
    local command=$2

    echo -e "\nTesting server: $description"
    eval $command

    if [ $? -eq 0 ]; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

# Test valid requests
test_request "Valid GET request" 'curl -X GET "http://localhost:8080/resource" -H "Host: example.com"'
test_request "Valid POST request" 'curl -X POST "http://localhost:8080/resource?param1=value1&param2=value2" -H "Host: example.com"'

# Test invalid requests
test_request "Invalid request (Bad method)" 'curl -X INVALIDMETHOD "http://localhost:8080/resource" -H "Host: example.com"'
test_request "Invalid request (No Host header)" 'printf "GET /resource HTTP/1.1\r\n\r\n" | nc localhost 8080'
test_request "Invalid request (No method)" 'printf "/resource HTTP/1.1\r\nHost: example.com\r\n\r\n" | nc localhost 8080'
test_request "Invalid request (No HTTP version)" 'printf "GET /resource\r\nHost: example.com\r\n\r\n" | nc localhost 8080'
test_request "Invalid request (Invalid HTTP version)" 'printf "GET /resource HTTP/2.0\r\nHost: example.com\r\n\r\n" | nc localhost 8080'
test_request "Invalid request (Empty request)" 'printf "\r\n" | nc localhost 8080'
test_request "Invalid request (Invalid URL)" 'curl -X GET "http://localhost:8080/resource?param1=value1&param2" -H "Host: example.com"'
test_request "GET request to retrieve a file" 'curl -X GET "http://localhost:8080/resource/file" -H "Host: example.com"'
