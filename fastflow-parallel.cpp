#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <thread>
#include <bitset>
#include <condition_variable>
#include "utimer.cpp"
#include <unistd.h>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>

using namespace std;
using namespace ff;

string encodeContent(const string data, const unordered_map<char, string>& huffmanCodes);

class CompressTask : public ff_node {
private:
    string inputFile;
    size_t start, end;
    const unordered_map<char, string>& huffmanCodes;
    int threadNum;
    unordered_map<int, string>& encodedChunks;

public:
    CompressTask(const string& inputFile, size_t start, size_t end, const unordered_map<char, string>& huffmanCodes,
                 int threadNum, unordered_map<int, string>& encodedChunks)
        : inputFile(inputFile), start(start), end(end), huffmanCodes(huffmanCodes),
          threadNum(threadNum), encodedChunks(encodedChunks) {}

    void* svc(void* taskData) {
        // utimer svc("Worker Node Computation " + to_string(threadNum));
        ifstream inFile(inputFile);

        // cout << "Input File: " << inputFile << endl;

        if (!inFile.is_open()) {
            cerr << "Error opening input file in threads." << endl;
            perror("open failure");
            return nullptr;
        }

        inFile.seekg(start);

        string fileContent = "";
        char ch;

        for (int i = start; i < end; i++) {
            inFile.get(ch);
            fileContent += ch;
        }

        inFile.close();

        // cout << "Thread " << threadNum << endl;

        string encodedData = encodeContent(fileContent, huffmanCodes);

        encodedChunks[threadNum] = encodedData;

        return GO_ON;
    }
};

// Function to get the file size

streampos getFileSize(string filename)
{
    ifstream file(filename, ios::binary | ios::ate);

    if (file.is_open()) {
        streampos filesize = file.tellg();
        // cout << "File size: " << filesize << " bytes" << endl;
        file.close();
        return filesize;
    } else {
        // cout << "File not found." << endl;
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

void writeBinaryStringToFile(const string& binaryString, const string& outputFile) {
    ofstream outFile(outputFile, ios::binary);

    // cout << "Output file path: " << outputFile << endl;

    if (!outFile.is_open()) {
        cerr << "Error opening output file" << endl;
        return;
    }

    bitset<8> byte;
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


unordered_map<char, int> buildFreqMap(string inputFile)
{
    ifstream file;

    // char* name = get_current_dir_name();

    // cout << name << endl;

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

string encodeContent(const string data, const unordered_map<char, string>& huffmanCodes)
{
    string encodedData = "";

    for (char ch : data)
    {
        encodedData += huffmanCodes.at(ch);
    }

    return encodedData;

}

void compressChunk(const string& inputFile, size_t start, size_t end, unordered_map<char, string>& huffmanCodes, int threadNum, unordered_map<int, string>& encodedChunks) {

    try
    {
        ifstream inFile(inputFile);

        // cout << "Input File: " << inputFile << endl;

        if (!inFile.is_open()) {
            cerr << "Error opening input file in threads." << endl;
            perror("open failure");
            return;
        }

        // Move file pointers to the start and end of the chunk
        inFile.seekg(start);

        string fileContent = "";
        char ch;

        for (int i = start; i < end; i++)
        {
            inFile.get(ch);
            fileContent += ch;
        }

        inFile.close();

        cout << "Thread " << threadNum << endl;

        string encodedData = encodeContent(fileContent, huffmanCodes);

        encodedChunks[threadNum] = encodedData;

    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
    }
    
}

int main(int argc, char* argv[]) {

    utimer t_full("Full Program");

    try
    {
        if (argc < 2) {
            cout << "Expecting the file path as the command line argument." << endl;
            return -1;
        }

        string inputFilePath = argv[1];
        string outputFilepath = "outputs/compressed.bin";
        // Define the number of threads you want to use for different stages
        int numThreads;

        if (argc > 2) {
            outputFilepath = argv[2];
            numThreads = argc == 4 ? stoi(argv[3]) : 1;
        }

        streampos inputFilesize = getFileSize(inputFilePath);

        cout << "File size of the uncompressed file is " << inputFilesize << " bytes." << endl;

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

        unordered_map<int, string> encodedChunks;

        for (int i = 0; i < numThreads; i++)
        {
            encodedChunks[i] = "";
        }

        size_t chunkSize = inputFilesize / numThreads;

        // cout << "After chunksize" << endl;


        vector<ff_node*> workers;

        {
            utimer tWorkerNodeCreation("Worker Node Creation");
            for (int i = 0; i < numThreads; ++i) {
                workers.push_back(new CompressTask(inputFilePath, i * chunkSize,
                                                (i == numThreads - 1) ? (unsigned long) inputFilesize : (i + 1) * chunkSize,
                                                huffmanCodes, i, encodedChunks));
                // cout << "Worker: " << i << endl;
            }
        }

        ff_farm farm(workers);

        {
            utimer tCompressedFile("Parallel Compression");
            if (farm.run_and_wait_end() < 0) {
                cerr << "Farm execution error." << endl;
                return -1;
            }
        }

        string encodedData = "";

        for (int i = 0; i < numThreads; i++)
        {
            encodedData += encodedChunks[i];
        }

        writeBinaryStringToFile(encodedData, outputFilepath);

        streampos outputFilesize = getFileSize(outputFilepath);

        cout << "File size of the compressed file is " << outputFilesize << " bytes." << endl;

        float compressedPercentage = (1.0 - ((float) outputFilesize / (float)inputFilesize)) * 100;

        cout << "File compressed by " << compressedPercentage << "%" << endl;

        return 0;
    }
    catch(const char* exception)
    {
        cout << exception << endl;
    }
    
    return -1;
}
