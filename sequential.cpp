#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

// Function to read the file

// string readFile(string filePath)
// {
//     ifstream file;
//     file.open(filePath);

//     if (file)
//     {
//         string fileContent, line;

//         while (getline(file, line))
//         {
//             fileContent += line + '\n';
//         }

//         // cout << fileContent << endl;

//         file.close();

//         return fileContent;

//     }
//     else
//     {
//         throw "Unable to open the file. Please make sure that the file is a valid .txt file.";
//     }

// }

// Node for Huffman tree
struct Node {
    char data;
    int freq;
    Node *left, *right;

    Node(char data, int freq) {
        this->data = data;
        this->freq = freq;
        left = right = nullptr;
    }
};

// Comparator for priority queue
struct CompareNodes {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

Node* buildHuffmanTree(const unordered_map<char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, CompareNodes> pq;

    for (const auto& pair : freqMap) {
        pq.push(new Node(pair.first, pair.second));
    }

    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();

        Node* newNode = new Node('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    return pq.top();
}

// Build Huffman codes
void buildHuffmanCodes(Node* root, string code, unordered_map<char, string>& huffmanCodes) {
    if (!root) return;

    if (root->data != '\0') {
        huffmanCodes[root->data] = code;
    }

    buildHuffmanCodes(root->left, code + "0", huffmanCodes);
    buildHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// Encode the input file using Huffman codes
string encodeFile(const string& inputFile, const unordered_map<char, string>& huffmanCodes) {
    string encodedData = "";
    ifstream file(inputFile);

    if (!file.is_open()) {
        cerr << "Error opening input file" << endl;
        return "";
    }

    char ch;
    while (file.get(ch)) {
        encodedData += huffmanCodes.at(ch);
    }

    file.close();
    return encodedData;
}

// Write encoded data to output file
void writeEncodedData(const string& encodedData, const string& outputFile, const unordered_map<char, string>& huffmanCodes) {
    ofstream file;
    file.open(outputFile, ios::binary | ios::out);

    if (!file.is_open()) {
        cerr << "Error opening output file" << endl;
        return;
    }

    // file << huffmanCodes.size() << endl;
    // for (const auto& pair : huffmanCodes) {
    //     file << pair.first << " " << pair.second << endl;
    // }

    file.write(encodedData.data(), encodedData.size());

    // file << encodedData;

    file.close();
}

unordered_map<char, int> buildFreqMap(string inputFile)
{
    ifstream file;
    file.open(inputFile);

    unordered_map<char, int> freqMap;

    if (file)
    {
        char ch;

        while (file.get(ch))
        {
            freqMap[ch]++;
        }

        // cout << fileContent << endl;

        file.close();

        return freqMap;

    }
    else
    {
        throw "Unable to open the file. Please make sure that the file is a valid .txt file.";
    }
}


bool compressFile(string inputFilePath, string outputFilePath="outputs/compressed.bin")
{
    try
    {
        unordered_map<char, int> freqMap = buildFreqMap(inputFilePath);
        Node* root = buildHuffmanTree(freqMap);

        unordered_map<char, string> huffmanCodes;

        buildHuffmanCodes(root, "", huffmanCodes);

        string encodedData = encodeFile(inputFilePath, huffmanCodes);

        writeEncodedData(encodedData, outputFilePath, huffmanCodes);

        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    
}

int main(int argc, char* argv[])
{

    try
    {
        // cout << "1" << endl;
        if (argc != 2)
        {
            cout << "Expecting the file path as the command line argument." << endl;
            return -1;
        }

        // cout << "2" << endl;

        string inputFilePath = argv[1];
        // string outputFilepath = argv[2];

        // string content = readFile(inputFilePath);
        
        // unordered_map freqMap = buildFreqMap(inputFilePath);

        bool compressed = compressFile(inputFilePath);


    }
    catch(const char* exception)
    {
        cout << exception << endl;
    }
    

    return 0;
}