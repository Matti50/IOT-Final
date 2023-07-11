from coapthon.client.helperclient import HelperClient

client = HelperClient(server=("127.0.0.1", 5683))

response = client.get("devices?id=002")

print("hey")
print(response.pretty_print())
client.stop()
