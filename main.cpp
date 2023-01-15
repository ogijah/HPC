#include <iostream>
#include <charconv>
#include <cstring>
#include <chrono>
#include "dinstring.cpp"

using namespace std::chrono;

void sequential_execution();
void parallel_execution();
void quicksort(int*, int, int);

int main(){
    cout << "----------------------------SEKVENCIJALNO------------------------\n" << endl;

    auto start_seq = high_resolution_clock::now();
    sequential_execution();
    cout << endl;
    auto stop_seq = high_resolution_clock::now();
    auto dur_seq = duration_cast<microseconds>(stop_seq - start_seq);

    cout << "-----------------------------------------------------------------\n" << endl;

    cout << "----------------------------PARALELNO----------------------------\n" << endl;

    auto start_par = high_resolution_clock::now();
    parallel_execution();
    auto stop_par = high_resolution_clock::now();
    auto dur_par = duration_cast<microseconds>(stop_par - start_par);

    cout << "-----------------------------------------------------------------\n" << endl;


    cout << "Vreme za sekvencijalnu obradu: " << dur_seq.count() << "ms" << endl;
    cout << "Vreme za paralelnu obradu: " << dur_par.count() << "ms" << endl;

}

void sequential_execution(){

    DinString pixels = "eabhaeahacebeiefghdjhe";
    char temp;
    int i,j;
    int n = pixels.length();

    for (i = 0; i < n-1; i++){
        for (j = i+1; j < n; j++){
            if (pixels[i] > pixels[j]){
                temp = pixels[i];
                pixels[i] = pixels[j];
                pixels[j] = temp;
            }
        }
    }
    
    cout << "Sortirani pikseli:\t" << pixels << endl;


    char activeChar = pixels[0];
    int nrSameChars = 0;
    DinString compressedPixels;
    
    for (i = 0; i <= n; i++){
        if (activeChar == pixels[i] || nrSameChars == 0){
            nrSameChars ++;
        }    
        else{
            if (nrSameChars < 4){
                for(int j = 0; j < nrSameChars; j++){
                    char chars[] = {activeChar};
                    compressedPixels += chars;
                }  
                activeChar = pixels[i];
                nrSameChars = 1;
            }
            else{
                char chars[] = "";
                strncat(chars, &activeChar, 1);
                compressedPixels += chars;
                compressedPixels += "@";
                char num_array[1];
                to_chars(num_array, num_array + 3, nrSameChars - 4);
                compressedPixels += num_array;
                activeChar = pixels[i];
                nrSameChars = 1;
            }
        }
    }

    cout << "Kompresovani pikseli:\t" << compressedPixels << endl;   
}


void parallel_execution(){
    int ulaz[] = {8,7,5,9,8,9,6,8};
    quicksort(ulaz, 0, 7);
    cout << "Sortirani ulaz:\t\t";

    for(int i = 0; i < 8; i++){
        cout << ulaz[i] << " ";
    }
    int maskaUnazad[8];
    maskaUnazad[0] = 1;
    for (int i = 1; i < 8; i++){
        if (ulaz[i] == ulaz[i-1]) {
            maskaUnazad[i] = 0;
        }
        else {
            maskaUnazad[i] = 1;
        }
    }
    cout << endl << "Maska unazad:\t\t";

    for(int i = 0; i < 8; i++){
        cout << maskaUnazad[i] << " ";
    }
    int skeniranaMaskaUnazad[8] = {0,0,0,0,0,0,0,0};

    int scan_a = 0;
    #pragma omp simd reduction(inscan, +:scan_a)
    for(int i = 0; i < 8; i++){
            scan_a += maskaUnazad[i];
            #pragma omp scan inclusive(scan_a)
            skeniranaMaskaUnazad[i] = scan_a;
    }

    cout << endl << "Skenirana maska unazad: ";
    for(int i = 0; i < 8; i++){
        cout << skeniranaMaskaUnazad[i] << " ";
    }

    int *kompaktnaMaskaUnazad = new int();
    kompaktnaMaskaUnazad[0] = 0;
    int index = 1;
    
    #pragma omp parallel for 
    for(int i = 1; i <= 8; i++){
        if (skeniranaMaskaUnazad[i] != skeniranaMaskaUnazad[i-1]){
            #pragma omp critical
            {
                kompaktnaMaskaUnazad[index] = i;
                index++;
            }

        }
    }

    quicksort(kompaktnaMaskaUnazad, 0, index-1);

    cout << endl << "Kompaktna maska unazad: ";
    for (int i = 0; i < index; i++){
        cout << kompaktnaMaskaUnazad[i] << " ";
    }

    int *izlazZnakovi = new int();
    int *izlazPonavljanja = new int();
    int j = 0;
    int x,y;
    for (int i = 0; i < index-1; i++){
            #pragma omp parallel 
            {
                #pragma omp sections
                {
                    #pragma omp section
                         x = kompaktnaMaskaUnazad[i];
                    #pragma omp section
                         y = kompaktnaMaskaUnazad[i+1];
                }
                
            }
            izlazZnakovi[j] = ulaz[x];
            izlazPonavljanja[j] = y-x;
            j++;
                
    }
    cout << endl << "Izlazni znakovi:\t";
    for (int i = 0; i < j; i++){
        cout << izlazZnakovi[i] << " ";
    }

    cout << endl << "Izlaz ponavljanja:\t";
    for (int i = 0; i < j; i++){
        cout << izlazPonavljanja[i] << " ";
    }

    cout << endl;

}

void quicksort(int* array, int left, int right) {
    if (left < right) {
        int pivot = array[(left + right) / 2];
        int i = left;
        int j = right;

        while (i <= j) {
            while (array[i] < pivot) {
                i++;
            }
            while (array[j] > pivot) {
                j--;
            }
            if (i <= j) {
                std::swap(array[i], array[j]);
                i++;
                j--;
            }
        }

        #pragma omp parallel sections
        {
            #pragma omp section
            quicksort(array, left, j);
            #pragma omp section
            quicksort(array, i, right);
        }
    }
}

