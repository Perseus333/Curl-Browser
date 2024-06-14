#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <cmath>
#include "utils.h"

using namespace std;

bool doDebug = true;

string configureUrlRequest(string url) {
    /*
    if (url.substr(0, 4) != "www." && url.length() > 6 && countAppearances(url, ".") < 2) {
        url = "www." + url;
    }

    url = "https://" + url;
    */

    ofstream configFile("requestConfig.txt");
    if (configFile.is_open()) {
        configFile << url;
        configFile.close();
    } else {
        cerr << "Unable to open file for writing." << endl;
        return "ERROR";
    }

    return url;
}

string addHeader(string& webContents) {
    // Adds the formatted title
    string header;

    string title = extractText(webContents, "<title>", "</title>", false);
    string formattedTitle;
    if (title.size() > 0) {
        formattedTitle = decorateText(title);
    }
    header += formattedTitle;
    return header;
}

void removeBrowserExclusiveElements(string& webContents) {
    // Removes browser-exclusive elements
    string elementsToRemove[4] = {"head", "script", "style", "meta"};
    for (const string& element : elementsToRemove) {
        string startTag = "<" + element;
        string endTag = "</" + element + ">";
        int numOfAppearances = countAppearances(webContents, startTag);

        for (int i = 0; i < numOfAppearances; i++) {
            string toRemove = extractText(webContents, startTag, endTag, true);
            removeSection(webContents, toRemove);
        }
    }
    // Remove comments
    for (const string& element : elementsToRemove) {
        string startTag = "<!--";
        string endTag = "-->";
        int numOfAppearances = countAppearances(webContents, startTag);

        for (int i = 0; i < numOfAppearances; i++) {
            string toRemove = extractText(webContents, startTag, endTag, true);
            removeSection(webContents, toRemove);
        }
    }
}

string getTagType(string html, int tagPos) {
    string tag;
    // tagPos +1 is to ignore the '<' character
    for (size_t pos = tagPos; pos < html.size(); pos++) {
        if (html[pos] != ' ' && html[pos] != '>') {
            tag += html[pos];
        }
        else {
            break;
        }
    }
    return tag;
}
string getAttributes(string html, int attributesPos) {
    string attributes;
    for (size_t pos = attributesPos; pos < html.size(); pos++) {
        if (html[pos] != '>') {
            attributes += html[pos];
        }
        else {
            break;
        }
    }
    return attributes;
}

string getText(string html, int textPos) {
    string text;
    for (size_t pos = textPos; pos < html.size(); pos++) {
        if (html[pos] != '<') {
            text += html[pos];
        }
        else if(html[pos] == '>') {
            // Do nothing
        }
        else {
            break;
        }
    }
    text = cleanString(text);
    return text;
}

struct TagData {
    string type;
    string attributes;
    string text;
    vector<int> relation;
};

vector<TagData> parsedArray;


vector<vector<int>> getRelations(const vector<TagData>& parsedArray) {
    vector<vector<int>> relations;

    // 1D vectors for easier reference
    vector<bool> tagStates;
    vector<string> types;

    vector<string> selfClosingTags = {
        "area", "base", "br", 
        "hr", "input", "link",
        "col", "img", "meta",
        "source", "wbr", "track", 
        "!DOCTYPE"};

    // Determine tag states (open or closed) and tag types
    for (const auto& row : parsedArray) {
        bool state = (row.type[0] != '/'); // '/' indicates closing tag
        string type = row.type;
        tagStates.push_back(state);
        types.push_back(type);
    }


    // Calculate relations based on tag states
    for (size_t pos = 0; pos < tagStates.size(); ++pos) {
        vector<int> currentRelation = {1}; // Default relation value
        // The first position is always 1
        if (pos != 0) {
            vector<int> previousRelation = relations.back();
            bool previousOpen = tagStates[pos - 1];
            bool open = tagStates[pos];

            // To deal with self closing tags
            if (isInArray(selfClosingTags, types[pos])) {
                currentRelation = previousRelation;
                currentRelation.back() += 1;
                tagStates[pos] = false;
            } 
            else { 
                if (previousOpen) {
                    if (open) {
                        previousRelation.push_back(1); // Increment depth of relation
                        currentRelation = previousRelation;
                    } else {
                        currentRelation = previousRelation; // Maintain relation value
                    }
                } else {
                    if (open) {
                        currentRelation = previousRelation;
                        currentRelation.back() += 1; // Increment relation value
                    } else {
                        if (!previousRelation.empty()) {
                            previousRelation.pop_back(); // Decrease depth of relation
                        }
                        currentRelation = previousRelation;
                    }
                }
            }
        }

        /*
        Example of the expect output of the relations logic:
        <html> (1)
            <body> (1,1)
                <div> (1,1,1)
                    <h1> (1111) Header</h1> (1111)
                    <p> (1112) This domain...</p> (1112)
                    <p> (1113)
                        <a (11131) href="example">More information...</a> (11131)
                    </p> (1113)
                </div> (111)
            </body> (11)
        </html> (1)
        */

        relations.push_back(currentRelation);
    }

    return relations;
}

string formatRules(TagData tag, string inputText) {
    string type = tag.type;
    string attributes = tag.attributes;
    string text = inputText;
    vector<int> relation = tag.relation;
    if (text != "") {
        if (type == "p") {
            // Do nothing, it's just plain text
        }
        else if (type == "b" || type == "strong") {
            text = "**" + text + "**";
        }
        else if (type == "em" || type == "i" || type == "ins") {
            text = "*" + text + "*";
        }
        else if (type == "marked") {
            text = "==" + text + "==";
        }
        else if (type == "del") {
            text = "~~" + text + "~~";
        }
        else if (type == "sub") {
            text = "~" + text + "~";
        }
        else if (type == "sup") {
            text = "^" + text + "^";
        }
        else if (type == "li") {
            text = "- " + text;
        }
        else if (type == "a") {
            string href = getAttribute(attributes, "href");
            if (href[0] == '.') {
                string tempHref = href;
                tempHref.erase(0);
                string url = readTextFromFile("requestConfig.txt");
                href = url + tempHref;
            }
            text = "[" + text + "](" + href + ")";
        }
        // Find how to manage ol
        else if (type == "h1") {
            text = "# " + text;
        }
        else if (type == "h2") {
            text = "## " + text;
        }
        else if (type == "h3") {
            text = "### " + text;
        }
        else if (type == "h4") {
            text = "#### " + text;
        }
        else if (type == "h5") {
            text = "##### " + text;
        }
        else if (type == "h6") {
            text = "###### " + text;
        }
    // <hr> never have text
    } else if(type == "hr") {
        text = "\n---";
    }
    return text;
}

string formatContents(vector<TagData> parsedArray) {
    string output;
    vector<vector<int>> tree;
    // Puts all the relations into a "tree"
    for (TagData tag : parsedArray) {
        tree.push_back(tag.relation);
    }

    // Main loop
    for (TagData tag : parsedArray) {
        // "Decompressing the array"
        vector<int> currentTree = tag.relation;

        // Look up parents
        // Each of all the possible parents can stack their properties onto their childs
        // Parents are all those vectors that can be created by taking the first n items of the relation attribute
        int amountOfParents = currentTree.size();
        if (doDebug) {
        cout << "Tag with relation:" << vectorIntToString(currentTree) << " has: " << to_string(amountOfParents) << "parents\n";
        }
        // Initial values
        string currentText = tag.text;

        for (int generation = amountOfParents; generation > 0; --generation) {
            
            vector<int> parent = currentTree;
            parent.resize(generation);
            if (doDebug) {
            cout << "Currently at generation number: " << generation << "(" << vectorIntToString(parent) << ")\n";
            }
            int parentPosInArray;
            auto posInArray = find(tree.begin(), tree.end(), parent);
            if (posInArray != tree.end()) {
                parentPosInArray = distance(tree.begin(), posInArray);
                if (doDebug) {
                cout << "Parent found in: " << parentPosInArray << "\n";
                }
            }
            else {
                if (doDebug) {
                cout << "Vector " << vectorIntToString(parent) << " not found";
                }
            }
            TagData parentTag = parsedArray[parentPosInArray];
            if (doDebug) {
            cout << "Tag that is that parent: " << vectorIntToString(parentTag.relation) << "\n";

            cout << "Formated text beforehand: " << currentText << "\n";
            }
            // Creates a temporary tag to stack text effects
            currentText = formatRules(parentTag, currentText);
            if (doDebug) {
            cout << "Formated text afterwards: " << currentText << "\n\n\n";
            }
        }
        string formattedText = currentText;
        formattedText += "\n";
        output += formattedText;
    } 
    return output;
}


string basicParse(string& webContents) {
    string preParsedText = webContents;
    
    // Removes the header, styles and scripts
    removeBrowserExclusiveElements(preParsedText);

    // Finds the positions of all tags
    vector<size_t> tagPositions = findCharPositions(preParsedText, '<');

    // Assigns characteristics to the tags
    for (int tagPos : tagPositions) {
        string type = getTagType(preParsedText, tagPos+1);
        string attributes = "";
        string text = "";
        // If the tag is empty it can't have no text nor attributes
        if (type[0] != '/') {
            int attributesPos = tagPos + type.size() + 1;
            attributes = getAttributes(preParsedText, attributesPos);
            int textPos = attributesPos + attributes.size() + 1;
            text = getText(preParsedText, textPos);
        }
        // Placeholder relation
        vector<int> placeholderRelation = {0};

        parsedArray.push_back({type, attributes, text, placeholderRelation});
    }

    // Places the relations into the parsedArray for better shipping
    vector<vector<int>> relations = getRelations(parsedArray);
    int rowPos = 0;

    for (auto& row : parsedArray) {
        row.relation = relations[rowPos];
        rowPos++;
        // For debugging puporses
        if (doDebug) {
        string type = row.type;
        string attributes = row.attributes;
        string text = row.text;
        vector<int> relation = row.relation;
        string relationStr;
        for (int level : relation) { relationStr += to_string(level) + " "; }
        string debug = "Type: " + type + "| Attributes: " + attributes + "|Text: " + text + "|Relation: " + relationStr;
        cout << debug << endl;
        }
        // End of debugging purposes
    }
    
    // Do something with formatContents(parsedArray, currentPos)

    string parsed = formatContents(parsedArray);

    return parsed;
}

int writeToFile(const string& path, const string& textToInsert) {
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << path << endl;
        return 1;
    }
    file << textToInsert;
    file.close();
    return 0;
}

int executeCurl() {
    if (system("cmd.exe /C curlExecute.bat") != 0) {
        cerr << "Failed to execute curl command." << endl;
        return 1;
    }
    return 0;
}

int main() {
    string url;
    cout << "Select the URL, Don't include https:// " << endl;
    cin >> url;  

    string urlToCurl = configureUrlRequest(url);

    // Reads the url from the requestConfig.txt file
    //executeCurl();

    string originalHtmlString = readTextFromFile("rawResponse.html");
    if (originalHtmlString.empty()) {
        cerr << "Failed to read from file or file is empty." << endl;
        return 1;
    }
    string output;
    output += urlToCurl + "\n";
    output += addHeader(originalHtmlString);
    output += profile("parse HTML", basicParse, originalHtmlString);

    writeToFile("output.md", output);

    return 0;
}
