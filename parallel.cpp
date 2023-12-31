#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <thread>
#include <bitset>
#include "utimer.cpp"
#include <unistd.h>

using namespace std;

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


        // char* name = get_current_dir_name();

        // cout << name << endl;

        string inputFilePath = argv[1];
        string outputFilepath = "outputs/compressed.bin";

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

        auto start_thread_creation = chrono::high_resolution_clock::now();
    
        // Create and join a single dummy thread
        thread dummy_thread([]() {});
        dummy_thread.join();

        // Measure the time taken for thread joining overhead
        auto end_thread_creation = chrono::high_resolution_clock::now();
        chrono::duration<double> thread_creation_time = end_thread_creation - start_thread_creation;

        // Print the thread creation overhead time
        cout << "Thread Creation Overhead Time: " << thread_creation_time.count() * 1000000 << " usec" << endl;

        {

            for (int i = 0; i < numThreads; i++)
            {
                encodedChunks[i] = "";
            }

            size_t chunkSize = inputFilesize / numThreads;

            vector<thread> threads(numThreads);

            utimer tCompressedFile("Parallel Compression");

            for (int i = 0; i < numThreads; ++i) {
                size_t start = i * chunkSize;
                size_t end = (i == numThreads - 1) ? (unsigned long) inputFilesize : (start + chunkSize);
                threads[i] = thread(compressChunk, ref(inputFilePath), start, end, ref(huffmanCodes), i, ref(encodedChunks));
                // threads[i].detach();
            }

            for (int i = 0; i < numThreads; i++)
            {
                threads[i].join();
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
