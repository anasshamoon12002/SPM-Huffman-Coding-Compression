#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include "utimer.cpp"

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

// Function to get the file size

streampos getFileSize(string filename)
{
    ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        std::streampos filesize = file.tellg();
        // std::cout << "File size: " << filesize << " bytes" << std::endl;
        file.close();
        return filesize;
    } else {
        // std::cout << "File not found." << std::endl;
        return -1;
    }
}

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


void writeBinaryStringToFile(const std::string& binaryString, const std::string& outputFile) {
    std::ofstream outFile(outputFile, std::ios::binary);

    if (!outFile.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return;
    }

    std::bitset<8> byte;
    size_t bitCount = 0;

    for (size_t i = 0; i < binaryString.length(); i++) {
        byte[7 - (bitCount % 8)] = (binaryString[i] == '1');
        bitCount++;

        if (bitCount % 8 == 0 || i == binaryString.length() - 1) {
            char byteChar = static_cast<char>(byte.to_ulong());
            outFile.write(&byteChar, 1);
            byte.reset();
        }
    }

    outFile.close();
}


// Write encoded data to output file
void writeEncodedData(const string& encodedData, const string& outputFile, const unordered_map<char, string>& huffmanCodes) {
    // ofstream file;
    // file.open(outputFile, ios::binary | ios::out);

    // if (!file.is_open()) {
    //     cerr << "Error opening output file" << endl;
    //     return;
    // }

    // file << huffmanCodes.size() << endl;
    // for (const auto& pair : huffmanCodes) {
    //     file << pair.first << " " << pair.second << endl;
    // }

    // file.write(encodedData.data(), encodedData.size());

    // file << encodedData;

    // file.close();

    // writeEncodedStringToFile(encodedData, outputFile);

    writeBinaryStringToFile(encodedData, outputFile);


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


bool compressFile(string inputFilePath, string outputFilePath)
{
    try
    {
        unordered_map<char, int> freqMap;

        {
            utimer tFreqMap("Frequency Map");

            freqMap = buildFreqMap(inputFilePath);
        }

        Node* root;

        {
            utimer tHuffmanTree("Huffman Tree");

            root = buildHuffmanTree(freqMap);
        }

        unordered_map<char, string> huffmanCodes;

        {
            utimer tHuffmanCodes("Huffman Codes");

            buildHuffmanCodes(root, "", huffmanCodes);
        }

        

        string encodedData;

        {
            utimer tEncodedContent("Encoded Content");

            encodedData = encodeFile(inputFilePath, huffmanCodes);

            // cout << endl << encodedData << endl << endl;
        }

        {
            utimer tWriteCompressed("Writing to file");

            writeEncodedData(encodedData, outputFilePath, huffmanCodes);
        }

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

    utimer t_full("Full Program");

    try
    {
        // cout << "1" << endl;
        if (argc < 2)
        {
            cout << "Expecting the file path as the command line argument." << endl;
            return -1;
        }

        string inputFilePath = argv[1];
        string outputFilepath = "outputs/compressed.bin";

        if (argc == 3)
        {
            outputFilepath = argv[2];
        }

        streampos inputFilesize = getFileSize(inputFilePath);

        cout << "File size of the uncompressed file is " << inputFilesize << " bytes." << endl;

        // string content = readFile(inputFilePath);
        
        // unordered_map freqMap = buildFreqMap(inputFilePath);

        utimer tCompressedFile("Actual Functionality");

        bool compressed = compressFile(inputFilePath, outputFilepath);

        streampos outputFilesize = getFileSize(outputFilepath);

        cout << "File size of the compressed file is " << outputFilesize << " bytes." << endl;

        float compressedPercentage = (1.0 - ((float) outputFilesize / (float)inputFilesize)) * 100;

        cout << "File compressed by " << compressedPercentage << "%" << endl;


    }
    catch(const char* exception)
    {
        cout << exception << endl;
    }
    

    return 0;
}