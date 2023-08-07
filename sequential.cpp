#include <iostream>
#include <fstream>

using namespace std;

// Function to read the file

string readFile(string filePath)
{
    ifstream file;
    file.open(filePath);

    if (file)
    {
        string fileContent, line;

        while (getline(file, line))
        {
            fileContent += line + '\n';
        }

        // cout << fileContent << endl;

        return fileContent;

    }
    else
    {
        throw "Unable to open the file. Please make sure that the file is a valid .txt file.";
    }

    return "";

}

int main(int argc, char* argv[])
{

    try
    {
        if (argc != 2)
        {
            cout << "Expecting the file path as the command line argument." << endl;
            return -1;
        }

        string content = readFile(argv[1]);

    }
    catch(const char* exception)
    {
        cout << exception << endl;
    }
    



    return 0;
}