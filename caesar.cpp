#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

void encryptCaesar(char* text, int length, int shift) {
    shift = shift % 26;
    if (shift < 0) shift += 26;

    for (int i = 0; i < length; ++i) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') {
            text[i] = (c - 'a' + shift) % 26 + 'a';
        }
        else if (c >= 'A' && c <= 'Z') {
            text[i] = (c - 'A' + shift) % 26 + 'A';
        }
    }
}

int main() {
    int shift = 5;
    string text;

    ifstream inFile("input.txt");
    if (!inFile) {
        cerr << "Error: can`t open input.txt!" << endl;
        return 1;
    }
    stringstream buffer;
    buffer << inFile.rdbuf();
    text = buffer.str();
    inFile.close();

    int length = text.length();
    if (length == 0) return 0;

    cout << "Text size: " << length << " characters." << endl;

    auto start = high_resolution_clock::now();

    encryptCaesar(&text[0], length, shift);

    auto stop = high_resolution_clock::now();
    duration<double> time_taken = stop - start;

    cout << "Exec time: " << time_taken.count() << " sec." << endl;

    ofstream outFile("output_seq.txt");
    if (outFile) {
        outFile << text;
        outFile.close();
    }
    else { cerr << "Error while writing in file!" << endl; }

    return 0;
}