import os
import requests
import unittest

#config gile = test3.conf
class TestMyHTTPServer(unittest.TestCase):
	base_url = "http://localhost:8080"  # Update with your server's URL
	upload_file = "./cgi_bin/upload/NOVO_TESTE.txt"  # Update with the path to your test file

	def test_lol_endpoint_get(self):
		# Test GET request
		response_get = requests.get(f"{self.base_url}/lol")
		self.assertEqual(response_get.status_code, 200)
		self.assertIn("index.html", response_get.text)
		self.assertIn("<!-- Empty body -->", response_get.text)
		#assert it gets an index file
		
	def test_lol_endpoint_post(self):
		# Test POST request
		payload = {"param1": "value1", "param2": "value2"}
		response_post = requests.post(f"{self.base_url}/lol", data=payload)
		self.assertEqual(response_post.status_code, 201)
		
	def test_lol_endpoint_delete(self):
		# Test DELETE request
		response_delete = requests.delete(f"{self.base_url}/lol/downloads/apagar.txt")
		self.assertEqual(response_delete.status_code, 204)

	def test_lol_endpoint_autoindex(self):
		response = requests.get(f"{self.base_url}/lol")
		self.assertEqual(response.status_code, 200)
		self.assertIn("index.html", response.text)
		self.assertIn("<!-- Empty body -->", response.text)

	#def test_lol_endpoint_uploads(self):
	#	print("Current Directory: " + os.getcwd())
	#	files = {"upload_file": open(self.upload_file, "rb")}
	#	response = requests.post(f"{self.base_url}/lol", files=files)
	#	self.assertEqual(response.status_code, 201)
	#	self.assertIn("Upload successful", response.text)
	
	def test_wrong_endpoint(self):
		response = requests.get(f"{self.base_url}/WRONG_ENDPOINT")
		self.assertEqual(response.status_code, 404)

	def test_wrong_method(self):
		response = requests.put(f"{self.base_url}/lol")
		self.assertEqual(response.status_code, 405)

	def test_resource_not_found(self):
		response = requests.get(f"{self.base_url}/lol/WRONG_FILE")
		self.assertEqual(response.status_code, 404)

	def test_cgi_get(self):
		response = requests.get(f"{self.base_url}/cgi-bin")
		self.assertEqual(response.status_code, 200)
		self.assertIn("Current Time", response.text)

if __name__ == "__main__":
	unittest.main()
