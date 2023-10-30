import requests

def test_http_server():
    base_url = "http://localhost:8080"
    host_header = "example.com"

    # Test GET request
    print_separator("Testing server with valid GET request:")
    test_request(requests.get, f"{base_url}/resource", headers={"Host": host_header})

    # Test POST request
    print_separator("Testing server with POST request:")
    data = {"param1": "value1", "param2": "value2"}
    test_request(requests.post, f"{base_url}/resource", data=data, headers={"Host": host_header})

    # Test invalid requests
    invalid_requests = [
        ("INVALIDMETHOD", "/resource", "Testing server with invalid request (Bad method)..."),
        ("", "/resource", "Testing server with invalid request (No method)..."),
        ("GET", "/resource", "Testing server with invalid request (No Host header)..."),
        ("GET", "/resource", "Testing server with invalid request (No HTTP version)..."),
        ("GET", "/resource", "Testing server with invalid request (Invalid HTTP version)..."),
        ("", "", "Testing server with invalid request (Empty request)..."),
        ("GET", "/resource?param1=value1&param2", "Testing server with invalid request (Invalid URL)..."),
    ]

    for method, path, description in invalid_requests:
        print_separator(description)
        test_request(requests.request, method, f"{base_url}{path}", headers={"Host": host_header})

    # Test GET to retrieve a file
    print_separator("Testing server using GET to retrieve a file:")
    test_request(requests.get, f"{base_url}/resource/file", headers={"Host": host_header})

def test_request(request_func, *args, **kwargs):
    try:
        if request_func == requests.request:
            response = request_func(args[0], args[1], headers=kwargs.get('headers', {}))
        else:
            response = request_func(args[0], **kwargs)
        response.raise_for_status()  # Raise an HTTPError for bad responses
        print_request(*args, **kwargs)
        print_response(response)
        print_result("OK")
    except requests.RequestException as e:
        print_request(*args, **kwargs)
        print_response(e.response)
        print_result(f"FAIL\nError: {e}")

def print_request(*args, **kwargs):
    print(f"Request: {args[1]}")
    print(f"Headers: {kwargs.get('headers', {})}")
    print(f"Data: {kwargs.get('data', {})}")

def print_response(response):
    print(f"Response:")
    print(f"Status Code: {response.status_code}")
    print(f"Headers: {response.headers}")
    print(f"Body: {response.text}")

def print_result(result):
    print(f"Result: {result}")

def print_separator(title):
    print("\n" + "=" * 40)
    print(title)
    print("=" * 40)

if __name__ == "__main__":
    test_http_server()
