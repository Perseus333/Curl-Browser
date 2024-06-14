// utils.h
#ifndef UTILS_H
#define UTILS_H


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <chrono>

using namespace std::chrono;
using namespace std;

// From ChatGPT
template<typename Func, typename... Args>
auto profile(const string& funcName, Func&& func, Args&&... args) -> decltype(func(forward<Args>(args)...)) {
    auto start = high_resolution_clock::now();
    auto result = func(forward<Args>(args)...);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();
    cout << "Time taken to " << funcName << ": " << duration << " microseconds" << endl;
    return result;
}

string readTextFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << filePath << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf(); 
    return buffer.str();
}

string decorateText(const string& rawText) {
    int textSize = rawText.length();
    string borderLine(textSize + 4, '@');
    string titleLine = "@ " + rawText + " @";

    string decoratedText = borderLine + "\n" + 
                           titleLine  + "\n" + 
                           borderLine + "\n\n";

    return decoratedText;
}

string extractText(const string& fullText,
                   const string& start,
                   const string& end,
                   bool includeTags) {
                                
    size_t startPos = fullText.find(start);
    if (startPos == string::npos) {
        return "";
    }
    if (!includeTags) {
        startPos += start.length();
    }
    size_t endPos = fullText.find(end, startPos);
    if (endPos == string::npos) {
        return "";
    }
    if (!includeTags) {
        return fullText.substr(startPos, endPos - startPos);
    } else {
        return fullText.substr(startPos, endPos - startPos + end.length());
    }
}

bool isInArray(const vector<string>& arr, const string& value) {
    return find(arr.begin(), arr.end(), value) != arr.end();
}

string trimToFirstNChars(const std::string& str, std::size_t n) {
    if (str.length() <= n) {
        return str; // No need to trim if the string is shorter than or equal to n
    }
    return str.substr(0, n); // Return the substring of the first n characters
}

string vectorIntToString(const vector<int> arr) {
    string str = "";
    for (int item : arr) {
        str += to_string(item);
    }
    return str;
}

void removeSection(string& mainStr, const string& toRemove) {
    size_t pos = mainStr.find(toRemove);
    while (pos != string::npos) {
        mainStr.erase(pos, toRemove.length());
        pos = mainStr.find(toRemove);
    }
}

// Function from ChatGPT
string cleanString(const std::string& input) {
    // Use a single regex to handle multiple spaces, tabs, and trailing spaces
    regex pattern("(\\s{2,}|\\t|\\s+$)");
    string result = std::regex_replace(input, pattern, " ");
    
    // Remove the single space added to the end if it was replaced
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }

    return result;
}

vector<size_t> findCharPositions(const string& globalString, char target) {

    vector<size_t> positions;

    for (size_t i = 0; i < globalString.size(); i++) {
        if (globalString[i] == target) {
            positions.push_back(i);
        }   
    }

    return positions;
}

int countAppearances(const string& str, const string& selection) {
    int count = 0;
    size_t pos = str.find(selection);

    while (pos != string::npos) {
        count++;
        pos = str.find(selection, pos + selection.length());
    }

    return count;
}

string getAttribute(const std::string& str, const std::string& attributeName) {
    string attribute;
    string strToFind = attributeName + "=\"";
    size_t initialPos = str.find(strToFind);
    
    if (initialPos != string::npos) {
        initialPos += strToFind.length();
        size_t endPos = str.find("\"", initialPos);

        if (endPos != string::npos) {
            attribute = str.substr(initialPos, endPos - initialPos);
        }
    }
    return attribute;
}

#endif // UTILS.H