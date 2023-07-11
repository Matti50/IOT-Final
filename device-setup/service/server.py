import json

from os import environ
import mysql.connector
from mysql.connector import errorcode

from coapthon.server.coap import CoAP
from coapthon.resources.resource import Resource


class DeviceResource(Resource):
    def __init__(self, name="DeviceResource", coap_server=None):
        super(DeviceResource, self).__init__(name, coap_server, visible=True,
                                            observable=True, allow_children=True)
        self.payload = "Device Resource"

    def render_GET_advanced(self, request, response):

        response_payload = {}

        query_string = str(request.uri_query)

        print(query_string)

        contains_device_id = query_string.find("id=") != -1

        if not contains_device_id:
            response.code = "400"
            response_payload["message"] = "Provide id as a query param"
            response.payload = str(response_payload)
            return self, response

        device_id = query_string.split("id=")[1]

        mysql_host = environ.get("DB_HOST", "127.0.0.1")
        mysql_user = environ.get("DB_USER", "root")
        mysql_password = environ.get("DB_PASSWORD", "root")
        mysql_database = environ.get("DB_NAME", "origin")

        try:
            connection = mysql.connector.connect(user=mysql_user, password=mysql_password,
                                                 host=mysql_host, database=mysql_database)

            cursor = connection.cursor()

            query = f"select d.device_id, p.name, p.preferred_watering_time from devices d inner join plants p on d.plant_id = p.plant_id where device_id = {device_id}"

            cursor.execute(query)

            data = cursor.fetchone()

            print(data)

            if data is None:
                response.code = "404"
                response_payload["message"] = f"The device with id {device_id} was not found."
                response.payload = json.dumps(response_payload)
                print(response.payload)
                return self, response

            response.code = 200
            response_payload["message"] = "The request was successful"
            response_payload["device"] = data[0]
            response_payload["name"] = data[1]

            time = data[2]

            if time is not None:
                time = time.time().hour

            response_payload["time"] = time

            print(response)

            cursor.close()

            response.payload = json.dumps(response_payload)
            return self, response

        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                print(err)
            elif err.errno == errorcode.ER_BAD_DB_ERROR:
                print("Database does not exist")
            else:
                print(err)
            response.code = 500
            response_payload["message"] = "There was an error in the server"
            response.payload = json.dumps(response_payload)
            connection.close()
            return self, response

        return self, response

    def render_PUT_advanced(self, request, response):
        self.payload = request.payload

        response_payload = {}

        device_id = request.payload

        if device_id is None or device_id == "":
            response_payload["code"] = 400
            response_payload["message"] = "Please provide a device id"
            response.payload = json.dumps(response_payload)
            return self, response

        mysql_host = environ.get("DB_HOST", "127.0.0.1")
        mysql_user = environ.get("DB_USER", "root")
        mysql_password = environ.get("DB_PASSWORD", "root")
        mysql_database = environ.get("DB_NAME", "origin")

        try:
            connection = mysql.connector.connect(user=mysql_user, password=mysql_password,
                                                 host=mysql_host, database=mysql_database)

            cursor = connection.cursor()

            query = f"select d.device_id, p.name, p.preferred_watering_time from devices d inner join plants p on d.plant_id = p.plant_id where device_id = '{device_id}'"

            cursor.execute(query)

            data = cursor.fetchone()

            if data is None:
                response_payload["code"] = 404
                response_payload["message"] = "The device was not found"
                response.payload = json.dumps(response_payload)
                return self, response

            response_payload["code"] = 200
            response_payload["message"] = "The request was successful"
            response_payload["device"] = data[0]
            response_payload["name"] = data[1]

            time = data[2]

            if time is not None:
                time = time.time().hour
            else:
                time = -1

            response_payload["time"] = time

            response.payload = json.dumps(response_payload)

            cursor.close()

            return self, response

        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                print(err)
            elif err.errno == errorcode.ER_BAD_DB_ERROR:
                print("Database does not exist")
            else:
                print(err)
            response_payload["code"] = 500
            response_payload["message"] = "There was an error in the server"
            response.payload = json.dumps(response_payload)
            connection.close()
            return self, response

        response.payload = json.dumps(response_payload)

        connection.close()
        return self, response

    def render_POST(self, request):
        res = {}

        req = json.loads(request.payload)

        mysql_host = environ.get("DB_HOST", "127.0.0.1")
        mysql_user = environ.get("DB_USER", "root")
        mysql_password = environ.get("DB_PASSWORD", "root")
        mysql_database = environ.get("DB_NAME", "origin")

        try:
            connection = mysql.connector.connect(user=mysql_user, password=mysql_password,
                                                 host=mysql_host, database=mysql_database)

            cursor = connection.cursor()

            query = f"""insert into devices values ('{req["device"]}', {req["plant"]})"""

            cursor.execute(query)

            cursor.close()
            connection.close()

        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
                print(err)
            elif err.errno == errorcode.ER_BAD_DB_ERROR:
                print("Database does not exist")
            else:
                print(err)
            connection.close()
            return res

        return res

    def render_DELETE(self, request):
        return True


class CoAPServer(CoAP):
    def __init__(self, host, port):
        CoAP.__init__(self, (host, port))
        self.add_resource('devices/', DeviceResource())


def main():
    server = CoAPServer("0.0.0.0", 5683)
    try:
        server.listen(10)
    except KeyboardInterrupt:
        print("Server Shutdown")
        server.close()
        print("Exiting...")


if __name__ == '__main__':
    main()
