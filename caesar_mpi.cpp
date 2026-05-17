#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mpi.h>

using namespace std;

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

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string originalText;
    int totalLength = 0;
    int shift = 5;

    if (rank == 0) {
        ifstream inFile("input.txt");
        if (!inFile) {
            cerr << "Error: can`t open input.txt!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        stringstream buffer;
        buffer << inFile.rdbuf();
        originalText = buffer.str();
        totalLength = originalText.length();
        inFile.close();

        cout << "Proc count: " << size << ". Text size: " << totalLength << " characters." << endl;
    }

    MPI_Bcast(&totalLength, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (totalLength == 0) {
        MPI_Finalize();
        return 0;
    }

    vector<int> sendCounts(size);
    vector<int> displacements(size);
    int baseChunkSize = totalLength / size;
    int remainder = totalLength % size;

    int currentDisplacement = 0;
    for (int i = 0; i < size; ++i) {
        sendCounts[i] = baseChunkSize + (i < remainder ? 1 : 0);
        displacements[i] = currentDisplacement;
        currentDisplacement += sendCounts[i];
    }

    int localLength = sendCounts[rank];
    vector<char> localBuffer(localLength);

    string encryptedText;
    if (rank == 0) { encryptedText.resize(totalLength); }

    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    MPI_Scatterv(rank == 0 ? originalText.data() : nullptr, sendCounts.data(), displacements.data(), MPI_CHAR, localBuffer.data(), localLength, MPI_CHAR, 0, MPI_COMM_WORLD);

    encryptCaesar(localBuffer.data(), localLength, shift);

    MPI_Gatherv(localBuffer.data(), localLength, MPI_CHAR, rank == 0 ? &encryptedText[0] : nullptr, sendCounts.data(), displacements.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        cout << "Exec time: " << (end_time - start_time) << " sec." << endl;

        ofstream outFile("output_mpi.txt");
        if (outFile) {
            outFile << encryptedText;
            outFile.close();
        }
        else { cerr << "Error ehile writing in file!" << endl; }
    }

    MPI_Finalize();
    return 0;
}