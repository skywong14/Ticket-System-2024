//
// Created by skywa on 2023/12/24.
//

#ifndef BOOKSTORE_2023_LINER_MEMORYIO_H
#define BOOKSTORE_2023_LINER_MEMORYIO_H

#include <fstream>
#include <ostream>
#include <cassert>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class LinerMemory {
private:
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
    int sizeofint = sizeof(int);

    int T_pos(int pos){
        return sizeofint * info_len + (pos - 1) * sizeofT;
    }

    void initialise_file(){
        int tmp = 0;
        file.open(file_name, std::ios::out | std::ios::binary);
        file.write(reinterpret_cast<char *>(&tmp), sizeofint);
        file.close();
    }

    int check_File_Exists(const string& FN){
        fstream file_tmp(FN);
        if (file_tmp.is_open()){
            file_tmp.close();
            return 1;
        } else
            return 0;
    }
public:
    LinerMemory() = default;
    LinerMemory(const string& file_name) : file_name(file_name) {}

    int initialise(string FN = "", int clear_file = 0) {
        if (!FN.empty())
            file_name = FN +"_file.lne";

        if (check_File_Exists(file_name) && !clear_file) return 0;

        initialise_file();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        assert(file.is_open());
        file.close();
        file.seekp(std::ios::beg);
        int tmp = 0;
        for (int i = 1; i <= info_len; i++)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
        return 1;
    }

    int read_info(int n) {
        if (n > info_len) return -1;
        file.open(file_name, std::ios::in | std::ios::binary);
        int tmp;
        file.seekg(sizeof(int) * (n-1), std::ios::beg);
        file.read((char*)&tmp, sizeof(int));
        file.close();
        return tmp;
    }

    void write_info(int pos, int tmp) {
        if (pos > info_len) return;
        file.open(file_name, std::ios::out | std::ios::in | std::ios::binary);
        int det = sizeofint * (pos - 1);
        file.seekp(det, std::ios::beg);
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    void write_T(int pos, T t_) {
        file.open(file_name, std::ios::out | std::ios::in | std::ios::binary);
        file.seekp(T_pos(pos), std::ios::beg);
        file.write(reinterpret_cast<char *>(&t_), sizeofT);
        file.close();
    }

    T read_T(int pos) {
        file.open(file_name, std::ios::out | std::ios::in | std::ios::binary);
        file.seekg(T_pos(pos), std::ios::beg);
        T t;
        file.read((char*)&t, sizeofT);
        file.close();
        return t;
    }
};

#endif //BOOKSTORE_2023_LINER_MEMORYIO_H
