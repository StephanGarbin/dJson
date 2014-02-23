#include "dJson.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

int main() {
	//test json string from http://en.wikipedia.org/wiki/JSON
	string json_string(R"xzy({"firstName":"John","lastName":"Smith","age": 25,"address":{"streetAddress":"21 2nd Street","city":"New York","state":"NY","postalCode":10021},"phoneNumbers":[{"type":"home","number":"212 555-1234"},{"type":"fax","number":"646 555-4567"}]})xzy");
	
	//Unordered map to hold the person object
	unordered_map<string, string> person;

	//Unordered map to hold the adress sub-object
	unordered_map<string, string> address;

	//vector to hold the phone numbers array
	vector<unordered_map<string, string>> phone_numbers;
	
	//parse the input string
	dJson::json_to_table(json_string, person);

	//parse the sub-object
	dJson::json_to_table(person["address"], address);

	//now parse the array
	dJson::json_array_to_elements(person["phoneNumbers"], phone_numbers);



	//print out some stuff for testing
	cout << "First Name: " << person["firstName"] << endl;
	cout << "Last Name : " << person["lastName"] << endl;
	cout << "Age:" << dJson::str2int(person["age"]) << endl;
	cout << "Postal Code : " << dJson::str2int(address["postalCode"]) << endl;

	cout << "Phone Numbers ----------" << endl << endl;

	for (auto& item : phone_numbers) {
		cout << "Type: " << item["type"] << endl;
		cout << "Number: " << item["number"] << endl << endl;
	}
}