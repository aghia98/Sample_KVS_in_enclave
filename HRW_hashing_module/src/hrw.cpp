#include <map>
#include <algorithm>
#include "hrw.h"


uint32_t jenkinsHash(string& server_id_with_string) {
    const uint8_t* data = (const uint8_t*) server_id_with_string.c_str();
    uint32_t hash = 0;

    for (size_t i = 0; i < server_id_with_string.size(); ++i) {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

vector<pair<string, uint32_t>> sortMapByValue(const map<string, uint32_t>& inputMap) {
    // Create a vector of pairs from the map
    vector<pair<string, uint32_t>> pairs(inputMap.begin(), inputMap.end());

    // Sort the vector based on the value member using a lambda function
    sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    return pairs;
}

vector<pair<string, uint32_t>> order_HRW(const std::vector<string>& ids, string& key) {
    //vector<int> sortedArr = ids;  // Create a copy of the input array
    
    map<string, uint32_t> ordered_ids_to_hash;
    
    // Sort the array in ascending order
    for(string id:ids){
        string to_hash = id+key;
        ordered_ids_to_hash[id]=jenkinsHash(to_hash);

    }
    vector<pair<string, uint32_t>> sortedPairs = sortMapByValue(ordered_ids_to_hash);
    
    return sortedPairs;
}


 //****************************************************************************

 bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

int extractNumber(string& input) {
    std::string numberStr;
    bool foundNumber = false;

    for (char c : input) {
        if (isDigit(c)) {
            numberStr += c;
            foundNumber = true;
        } else if (foundNumber) {
            // Break loop if a digit was found and a non-digit character follows
            break;
        }
    }

    if (foundNumber) {
        int number = std::stoi(numberStr);
        return number;
    } else {
        return -1; // Indicate no number found
    }
}

//********************************************************************************

vector<string> convert_ids_to_strings_with_id(vector<int> ids, string word){
    vector<string> result;
    for(int id: ids){
        result.push_back(word+to_string(id));
    }  

    return result;
 }

vector<int> convert_strings_with_id_to_ids(vector<string> strings_with_id){
    vector<int> result;
    for(string string_with_id: strings_with_id){
        result.push_back(extractNumber(string_with_id));
    }  

    return result;
 }