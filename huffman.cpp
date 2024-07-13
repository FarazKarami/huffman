#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <sstream>

using namespace std;

// Node structure for the Huffman Tree
struct Node {
    char ch;   // Character
    int freq;  // Frequency of the character
    Node *left, *right;  // Left and right children
    Node(char c = '$', int f = 0) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// Comparator to be used in the priority queue for building the Huffman Tree
struct compare {
    bool operator()(Node *l, Node *r) {
        return l->freq > r->freq;
    }
};

// Function prototypes
string readFile(const string &filePath);
map<char, int> calculateFrequency(const string &text);
Node* buildHuffmanTree(map<char, int> &frequency);
void storeCodes(Node* root, string str, map<char, string> &huffmanCode);
string encodeText(const string &text, map<char, string> &huffmanCode);
void writeToFile(const string &filePath, const string &content);
void writeHuffmanTable(const string &filePath, const map<char, string> &huffmanCode);
string decodeText(Node* root, const string &encodedText);
void decodeFromFile(const string &tablePath, const string &letterPath);
void initial(Node* root, const string &letterPath);
string trace(int i, const string &txt, Node* root, Node* current, string output);
void deleteTree(Node* root);

int main() {
    string filePath;
    cout << "Enter the path of the text file: ";
    cin >> filePath;

    // Read the input file
    string text = readFile(filePath);
    
    // Calculate the frequency of each character in the text
    map<char, int> frequency = calculateFrequency(text);
    
    // Build the Huffman Tree
    Node* root = buildHuffmanTree(frequency);

    // Store the Huffman Codes
    map<char, string> huffmanCode;
    storeCodes(root, "", huffmanCode);

    // Encode the text using Huffman Codes
    string encodedText = encodeText(text, huffmanCode);
    
    // Write the encoded text to a file
    writeToFile("encoded.txt", encodedText);
    
    // Write the Huffman table to a file
    writeHuffmanTable("huffman_table.txt", huffmanCode);

    // Decode the text from the encoded string
    string decodedText = decodeText(root, encodedText);
    cout << "Decoded Text: " << decodedText << endl;

    // Get paths for Huffman table and encoded letter files
    string tablePath, letterPath;
    cout << "Enter the path of the Huffman table file: ";
    cin >> tablePath;
    cout << "Enter the path of the encoded letter file: ";
    cin >> letterPath;

    // Decode from files
    decodeFromFile(tablePath, letterPath);

    // Delete the Huffman Tree to free memory
    deleteTree(root);

    return 0;
}

// Function to read the entire content of a file
string readFile(const string &filePath) {
    ifstream file(filePath);
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

// Function to calculate the frequency of each character in the text
map<char, int> calculateFrequency(const string &text) {
    map<char, int> frequency;
    for (char ch : text) {
        frequency[ch]++;
    }
    return frequency;
}

// Function to build the Huffman Tree from the frequency map
Node* buildHuffmanTree(map<char, int> &frequency) {
    priority_queue<Node*, vector<Node*>, compare> minHeap;
    
    // Create a leaf node for each character and add it to the priority queue
    for (auto &pair : frequency) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    // Iterate until the size of heap becomes 1
    while (minHeap.size() != 1) {
        // Extract the two nodes with the highest priority (lowest frequency)
        Node *left = minHeap.top(); minHeap.pop();
        Node *right = minHeap.top(); minHeap.pop();

        // Create a new internal node with a frequency equal to the sum of the two nodes
        Node *top = new Node('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        // Add the new node to the heap
        minHeap.push(top);
    }

    // The remaining node is the root of the Huffman Tree
    return minHeap.top();
}

// Function to store the Huffman Codes in a map
void storeCodes(Node* root, string str, map<char, string> &huffmanCode) {
    if (!root) return;

    // If it's a leaf node, store the code
    if (root->ch != '$') huffmanCode[root->ch] = str;

    // Recur for the left and right children
    storeCodes(root->left, str + "0", huffmanCode);
    storeCodes(root->right, str + "1", huffmanCode);
}

// Function to encode the text using Huffman Codes
string encodeText(const string &text, map<char, string> &huffmanCode) {
    stringstream encodedText;
    for (char ch : text) {
        encodedText << huffmanCode[ch];
    }
    return encodedText.str();
}

// Function to write content to a file
void writeToFile(const string &filePath, const string &content) {
    ofstream file(filePath);
    file << content;
}

// Function to write the Huffman table to a file
void writeHuffmanTable(const string &filePath, const map<char, string> &huffmanCode) {
    ofstream file(filePath);
    for (auto &pair : huffmanCode) {
        file << pair.first << ": " << pair.second << "\n";
    }
}

// Function to decode the encoded text using the Huffman Tree
string decodeText(Node* root, const string &encodedText) {
    string decodedText;
    Node* current = root;
    for (char bit : encodedText) {
        // Traverse the Huffman Tree according to the bit
        if (bit == '0') current = current->left;
        else current = current->right;

        // If a leaf node is reached, add the character to the result
        if (!current->left && !current->right) {
            decodedText += current->ch;
            current = root;
        }
    }
    return decodedText;
}

// Function to decode text from files containing the Huffman table and the encoded text
void decodeFromFile(const string &tablePath, const string &letterPath) {
    map<char, string> charTable;

    // Read the Huffman table from the file
    try {
        ifstream reader(tablePath);
        if (!reader.is_open()) {
            cerr << "Error opening file: " << tablePath << endl;
            return;
        }

        string temp;
        while (getline(reader, temp)) {
            size_t pos = temp.find(':');
            if (pos != string::npos) {
                charTable[temp[0]] = temp.substr(pos + 2);
            }
        }

        reader.close();
    }
    catch (const exception& error) {
        cerr << "An error occurred: " << error.what() << endl;
        return;
    }

    // Reconstruct the Huffman Tree from the table
    Node* root = new Node('$', 0);
    for (auto& pair : charTable) {
        string binValue = pair.second;
        Node* current = root;
        for (int i = 0; i < binValue.length(); ++i) {
            if (binValue[i] == '0') {
                if (current->left == nullptr) {
                    current->left = new Node('$', 0);
                }
                current = current->left;
            } else {
                if (current->right == nullptr) {
                    current->right = new Node('$', 0);
                }
                current = current->right;
            }
        }
        current->ch = pair.first;
    }

    // Decode the encoded text from the file
    initial(root, letterPath);

    // Delete the Huffman Tree to free memory
    deleteTree(root);
}

// Function to read the encoded text from a file and decode it using the Huffman Tree
void initial(Node* root, const string &letterPath) {
    string temp;
    try {
        ifstream reader(letterPath);
        if (!reader.is_open()) {
            cerr << "Error opening file: " << letterPath << endl;
            return;
        }

        getline(reader, temp);
        reader.close();
    }
    catch (const exception& error) {
        cerr << "An error occurred: " << error.what() << endl;
        return;
    }

    try {
        ofstream writer("Decode.txt");
        if (!writer.is_open()) {
            cerr << "Error opening output file: Decode.txt" << endl;
            return;
        }

        writer << trace(0, temp, root, root, "");
        writer.close();
    }
    catch (const exception& error) {
        cerr << "An error occurred while writing to the file: " << error.what() << endl;
        return;
    }
}

// Recursive function to decode the text
string trace(int i, const string &txt, Node* root, Node* current, string output) {
    // If a leaf node is reached, add the character to the output
    if (current->ch != '$') {
        output += current->ch;
        return trace(i, txt, root, root, output);
    }
    // If the end of the text is reached, return the output
    if (i == txt.length()) return output;

    if (current == nullptr) throw runtime_error("Failed To Decode!");

    // Traverse the Huffman Tree according to the bit
    if (txt[i] == '0')
        return trace(i + 1, txt, root, current->left, output);
    else
        return trace(i + 1, txt, root, current->right, output);
}

// Function to delete the Huffman Tree and free memory
void deleteTree(Node* root) {
    if (root) {
        deleteTree(root->left);
        deleteTree(root->right);
        delete root;
    }
}
