#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mpi.h>

using namespace std;

// Функція для локального шифрування фрагмента тексту
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
    int shift = 5; // Крок зсуву

    // Головний процес читає файл
    if (rank == 0) {
        ifstream inFile("input.txt");
        if (!inFile) {
            cerr << "Помилка: не вдалося відкрити файл input.txt!" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1); // Перериваємо виконання всіх процесів
        }

        // Зчитуємо весь вміст файлу у рядок
        stringstream buffer;
        buffer << inFile.rdbuf();
        originalText = buffer.str();
        totalLength = originalText.length();
        inFile.close();

        cout << "Прочитано " << totalLength << " символів з input.txt." << endl;
        cout << "Кількість MPI процесів: " << size << endl;
    }

    // Передаємо загальну довжину тексту всім процесам
    MPI_Bcast(&totalLength, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Якщо файл порожній, просто завершуємо програму
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

    // Розподіл тексту між усіма процесами
    MPI_Scatterv(rank == 0 ? originalText.data() : nullptr,
        sendCounts.data(), displacements.data(), MPI_CHAR,
        localBuffer.data(), localLength, MPI_CHAR,
        0, MPI_COMM_WORLD);

    // Паралельне виконання
    encryptCaesar(localBuffer.data(), localLength, shift);

    string encryptedText;
    if (rank == 0) {
        encryptedText.resize(totalLength);
    }

    // Збір результатів
    MPI_Gatherv(localBuffer.data(), localLength, MPI_CHAR,
        rank == 0 ? &encryptedText[0] : nullptr,
        sendCounts.data(), displacements.data(), MPI_CHAR,
        0, MPI_COMM_WORLD);

    // Головний процес записує результат у новий файл
    if (rank == 0) {
        ofstream outFile("output.txt");
        if (outFile) {
            outFile << encryptedText;
            outFile.close();
            cout << "Результат успішно збережено у output.txt." << endl;
        }
        else {
            cerr << "Помилка: не вдалося створити output.txt!" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}