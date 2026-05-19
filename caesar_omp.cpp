#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <omp.h> 

using namespace std;

void encryptCaesarOpenMP(string& text, int shift, int num_threads) {
    shift = shift % 26;
    if (shift < 0) shift += 26;

    int length = text.length();

    omp_set_num_threads(num_threads);

#pragma omp parallel for
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
    int num_threads = 4; 
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
    cout << "Thread count: " << num_threads << endl;

    double start_time = omp_get_wtime();

    encryptCaesarOpenMP(text, shift, num_threads);

    double end_time = omp_get_wtime();

    cout << "exec time: " << (end_time - start_time) << " sec." << endl;

    ofstream outFile("output_omp.txt");
    if (outFile) {
        outFile << text;
        outFile.close();
        cout << "Result saved to output_omp.txt" << endl;
    }
    else {
        cerr << "Error while writing to file!" << endl;
    }

    return 0;
}