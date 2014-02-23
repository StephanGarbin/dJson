#pragma once
#ifndef DRACARYS_dJson_H
#define DRACARYS_dJson_H
//STL
#include <cstdlib>
#include <vector>
#include <string>
#include <unordered_map>

//TBB

/*!
*  \brief     This class presents a static interface for converting to and from json.
*  \details   While used heavily internally, the user is encourage to make use of this class when implementing custom nodes as it makes json conversion easier and standardises procedure.
*  \author    Stephan J. Garbin
*  \version   2.4
*  \date      2013-2014
*  \warning   This class is not fully optimised or multi-threaded.
*  \copyright Apache License, Version 2.0.
*/
class dJson {
public:
	//! Converts a json input string to a std::unordered_map
	/*!
	\param json the input json string
	\param item the output map
	*/
	static void json_to_table(const std::string& json, std::unordered_map<std::string, std::string>& item);

	//! Converts a json array to a std::vector of std::unordered_map
	/*!
	\param json the input json string
	\param items the vector of items
	*/
	static void json_array_to_elements(const std::string& json, std::vector<std::unordered_map<std::string, std::string>>& items);

	//! This function converts the string input to boolean.
	/*! If the input cannot be parsed, false will be returned.
	\param input the input string
	*/
	static bool str2bool(const std::string& input);

	//! This function converts the string input to an int.
	/*!
	\param input the input string
	*/
	static int str2int(const std::string& input);

	//! This function converts the string input to an long.
	/*!
	\param input the input string
	*/
	static long str2long(const std::string& input);

	//! This function converts the string input to an double.
	/*!
	\param input the input string
	*/
	static double str2double(const std::string& input);

	//! Converts the input vector into a json array of strings of the form [ "<value1>", "<value2>", ...]
	template<typename T>
	static std::string vector2jsonStrArray(const std::vector<T>& input);

	//! Converts the input vector into a json array of strings of the form [ <value1>, <value2>, ...]
	template<typename T>
	static std::string vector2jsonNumArray(const std::vector<T>& input);

private:
	//CONVERSION HELPER FUNCTIONS------------------------------
	//! Returns substring from specified start up to and including the delimiting char.
	/*!
	\param message the input string
	\param start the start index from which to scan the message
	\param delimiter a function taking (const std::string& message, const size_t index) that should return true if the delimiting char is at index.
	\return a substring if a delimiting char is found, the empty string otherwise (and prints error)
	*/
	template <typename FUNCTION>
	static std::string get_enclosed_substr(const std::string& message, size_t& start, FUNCTION delimiter);

	//! Returns substring from specified start up to and including the delimiting char.
	/*! Overload that allows specificatin of the check function (that checks whether this call results in a empty string).
	If you do want this check to take place, always make the checker return false.
	\sa get_enclosed_substr(const std::string message, size_t& start, FUNCTION delimiter)
	*/
	template <typename FUNCTION, typename FUNCTION_2>
	static std::string get_enclosed_substr(const std::string message, size_t& start, FUNCTION delimiter, FUNCTION_2 checker = nullptr);

	//! Returns true if the specified char of the input string is 0-9 or '.'.
	/*!
	\param value the input string
	\index the index of the input string char to compare
	\return true if input[index] == 0-9 || '.' ; false otherwise.
	*/
	static inline bool is_number_literal(const std::string& value, size_t index);

	//! Removes unncessary whitespaces from the input string.
	/*!
		\param input the input string
		\warning This is DEPRECATED.
	*/
	static inline void remove_whitespaces(std::string& input);

	//This class is only supposed to be used statically
	//! Ctor. This is private as this class is only supposed to be used statically.
	dJson() { };
	//! Dtor. This is private as this class is only supposed to be used statically.
	~dJson() { };

}; //class dJson

//TEMPLATE IMPLEMENTATION ------------------------------------------------------------------------------------------------------------------------

template<typename T>
std::string dJson::vector2jsonStrArray(const std::vector<T>& input) {
	std::string temp("[");
	for (size_t i = 0; i < input.size(); ++i) {
		temp.append("\"" + std::to_string(input[i]) + "\"");
		if (i < input.size() - 1) {
			temp.append(",");
		}
	}
	temp.append("]");

	return temp;
}

template<typename T>
std::string dJson::vector2jsonNumArray(const std::vector<T>& input){
	std::string temp("[");
	for (size_t i = 0; i < input.size(); ++i) {
		temp.append(std::to_string(input[i]));
		if (i < input.size() - 1) {
			temp.append(",");
		}
	}
	temp.append("]");

	return temp;
}

//IMPLEMENTATION ------------------------------------------------------------------------------------------------------------------------

void
dJson::json_array_to_elements(const std::string& json, std::vector<std::unordered_map<std::string, std::string>>& items) {

	std::unordered_map<std::string, std::string> item;

	for (size_t i = 0; i < json.size(); ++i) {
		if (json[i] == '{') {
			std::string value = get_enclosed_substr(json, i, [](const std::string& message, const size_t i) {
				return (message[i] == '}');
			});
			//read the array element
			json_to_table(value, item);
			//add the element to the vector
			items.push_back(item);
		}
	}
}

void
dJson::json_to_table(const std::string& json, std::unordered_map<std::string, std::string>& item) {
	//Some temporaries for the loop
	bool read_key = false;
	bool read_value = false;
	std::string key;
	std::string value;

	for (size_t i = 0; i < json.size(); ++i) {

		//1. Read key (if we haven't already)
		if (!read_key && json[i] == '"')  {
			if (!read_key) { //we are reading the key
				key = std::move(get_enclosed_substr(json, i, [](const std::string& message, const size_t index) {
					//Note: ONLY '"' is delimiting!
					return (message[index] == '"') ? true : false; //verbose style unnecessary but easier to read
				}));
				read_key = true;
			}
		}

		//2. Read Value (if we've read a key already)
		if (read_key && json[i] == ':') {
			//move to beginning of value sequence
			++i;
			//2.1 Discard any unncessary whitespace
			while (json[i] == ' ') {
				++i;
			}

			//evaluate based on type of argument
			if (json[i] == '"') {
				value = std::move(get_enclosed_substr(json, i, [](const std::string& message, const size_t index) {
					//Note: ONLY '"' is delimiting!
					return (message[index] == '"') ? true : false;
				}));
				read_value = true;
			}
			else if (json[i] == '[') {
				value = std::move(get_enclosed_substr(json, i, [](const std::string& message, const size_t index) {
					return (message[index] == ']') ? true : false;
				}));
				read_value = true;
			}
			else if (json[i] == '{') {
				value = std::move(get_enclosed_substr(json, i, [](const std::string& message, const size_t index) {
					return (message[index] == '}') ? true : false;
				}));
				read_value = true;
			}
			else if (is_number_literal(json, i)) {
				value = std::move(get_enclosed_substr(json, i, [&](const std::string& message, const size_t index) {
					//we are inverting here any char that is not numerical is a deliminator
					return !is_number_literal(json, index);
				}, [](const std::string& message, const size_t index) {
					// Numbers CAN be 1 char long, so we can omit this test
					return false;
				}));

				read_value = true;
			}
			//special case 1: true / false for booleans
			//OPTIMISATION: we do not need to check all letters here
			else if (json[i] == 't' && json[i + 1] == 'r'/* && json[i + 2] == 'u' && json[i + 3] == 'e'*/) {
				value = "true";
				read_value = true;
				i += 4;
			}
			else if (json[i] == 'f' && json[i + 1] == 'a' /*&& json[i + 2] == 'l' && json[i + 3] == 's' && json[i + 4] == 'e'*/){
				value = "false";
				read_value = true;
				i += 5;
			}


			//3. Move to end of value sequence or end of file - 1 (to allow the print at the start to work)
			while (json[i] != ',' && i < json.size() - 2) {
				++i;
			}

			//4. Check If we've read a pair
			if (read_key && read_value) {
				//delete unnecessary double quotes when reading
				//OPTIMISATION: We do not need to check end here
				key = (key[0] == '"' /*&& key[key.size() - 1] == '"'*/) ? std::move(key.substr(1, key.size() - 2)) : key;
				value = (value[0] == '"' /*&& value[value.size() - 1] == '"'*/) ? std::move(value.substr(1, value.size() - 2)) : value;
				//add pair to map
				item[std::move(key)] = std::move(value); //we can move here, because those value will be overriden.
				//reset bool read checks
				read_key = false;
				read_value = false;
			}
		}
	}
}

template <typename FUNCTION>
std::string
dJson::get_enclosed_substr(const std::string& message, size_t& start, FUNCTION delimiter) {
	//Handle 'empty' cases such as: "attr":"";
	if (delimiter(message, start + 1)) {
		return "";
	}

	size_t curr_index = start + 1;

	while (!delimiter(message, curr_index)) {
		//std::cout << "adding:" << message[curr_index] << endl;
		//Test we haven't reached the current end of file
		if (curr_index + 1 == message.size()) {
			puts("Substr could not be read. Reached end of file.");
			break;
		}
		++curr_index;
	}

	size_t old_Start = start;
	start = ++curr_index;
	return message.substr(old_Start, curr_index - old_Start);
}

template <typename FUNCTION, typename FUNCTION_2>
std::string
dJson::get_enclosed_substr(const std::string message, size_t& start, FUNCTION delimiter, FUNCTION_2 checker) {
	//Handle 'empty' cases such as: "attr":"";
	if (checker(message, start + 1)) {
		return "";
	}

	size_t curr_index = start + 1;

	while (!delimiter(message, curr_index)) {
		//std::cout << "adding:" << message[curr_index] << endl;
		//Test we haven't reached the current end of file
		if (curr_index == message.size() - 1) {
			puts("Substr could not be read. Reached end of file.");
			break;
		}
		++curr_index;
	}

	size_t old_Start = start;
	start = curr_index;
	return message.substr(old_Start, curr_index - old_Start);
}

bool
dJson::is_number_literal(const std::string& value, size_t index) {
	return (value[index] == '0' || value[index] == '1' || value[index] == '2' || value[index] == '3' || value[index] == '4' || value[index] == '5'
		|| value[index] == '6' || value[index] == '7' || value[index] == '8' || value[index] == '9' || value[index] == '.') ? true : false;
}


void
remove_whitespaces(std::string& input) {
	//string to store the result
	std::string result;
	//avoid unnecessary reallocations. we are going to need a minimum of this length
	result.resize(input.size());

	bool in_substring = false;

	for (size_t i = 0; i < input.size(); ++i) {
		if (in_substring == false && input[i] != ' ') {
			result.push_back(input[i]);
		}

		if (input[i] != '\"') {
			in_substring == true;
		}
		else if(input[i] != '\"' && in_substring == true) {
			in_substring == false;
		}
	}

	input = std::move(result);
}

bool
dJson::str2bool(const std::string& input) {
	return (input.compare("true") == 0) ? true : false;
}

int
dJson::str2int(const std::string& input) {
	return std::atoi(input.c_str());
}

long
dJson::str2long(const std::string& input) {
	return std::atol(input.c_str());
}

double
dJson::str2double(const std::string& input) {
	return std::atof(input.c_str());
}




#endif //DRACARYS_dJson_H