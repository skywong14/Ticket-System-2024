//
// Created by skywa on 2024/5/25.
//

#ifndef TICKET_SYSTEM_2024_MYTOOLS_HPP
#define TICKET_SYSTEM_2024_MYTOOLS_HPP
#pragma once

#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdio>
#include "../My_Stl/vector.hpp"
#include "../My_Stl/map.hpp"
#include <vector> // for debug only

using std::string;
using sjtu::vector; // for debug only
using sjtu::map;

enum class ReturnMode{
    Correct, Lack_Permission, Invalid_Format, Out_Of_Range, Wrong_Value, Invalid_Operation, Other_Error
};
enum class Command_Name {
    add_user, login, logout, query_profile, modify_profile,
    add_train, delete_train, release_train, query_train, query_ticket,
    query_transfer, buy_ticket, query_order, refund_ticket,
    clean, exit, unknown
};
template <int LENGTH>
struct MyString{
    char str[LENGTH];
    MyString():str(){}
    MyString(const string& str_):str(){
        strcpy(str, str_.c_str());
    }
    MyString(const char* str_):str(){
        strcpy(str, str_);
    }
    char& operator [](int pos){
        return str[pos];
    }
    bool operator < (const MyString<LENGTH>& obj) const {
        return strcmp(str, obj.str) < 0;
    }
    bool operator == (const MyString<LENGTH>& obj) const {
        return strcmp(str, obj.str) == 0;
    }
    bool operator <= (const MyString<LENGTH>& obj) const {return !(obj < *this);}
    bool operator > (const MyString<LENGTH>& obj) const {return obj < *this;}
    bool operator >= (const MyString<LENGTH>& obj) const {return !(*this < obj);}
    bool operator != (const MyString<LENGTH>& obj) const {return !(*this == obj);}

    string to_string(){
        return string(str, str + LENGTH);
    }

    friend std::ostream& operator<<(std::ostream& os, const MyString& obj) {
        return os << obj.str;
    }
};


using Command_Head = std::pair<int, Command_Name>;

//User
using type_userid = MyString<21>;
using type_password = MyString<31>;
using type_realname = MyString<16>; // 3 * 5 + 1
using type_mailAddr = MyString<31>;
using type_privilege = int;
//Train
using type_trainID = MyString<21>;
using type_stationName_ptr = int;
using type_stationName = MyString<31>;
using type_stationNum = int;
using type_searNum = int;
using type_prices = int;

//定义01-01 00:00 为time0时刻
//2024年2月有29天
constexpr int MonthDays[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

class type_time{
private:
    int standard;
public:
    type_time():standard(0) {}
    explicit type_time(int sta_):standard(sta_) {}
    explicit type_time(const string& str){
        standard = 0;
        if (str[2] == '-') {
            standard = (MonthDays[std::stoi(str.substr(0, 2)) - 1]
                    + (std::stoi(str.substr(3, 2)) - 1)) * 1440;
        } else if (str[2] == ':'){
            standard = std::stoi(str.substr(0, 2)) * 60
                        + std::stoi(str.substr(3, 2));
        }
    }
    type_time(int Month_, int Day_, int Hour_ = 0, int Min_ = 0){
        standard =  (MonthDays[Month_ - 1] + (Day_ - 1)) * 1440 + Hour_ * 60 + Min_;
    }

    type_time operator+(const type_time& other) const {
        type_time result(standard + other.standard);
        return result;
    }
    type_time operator-(const type_time& other) const {
        type_time result(standard - other.standard);
        return result;
    }
    int Days() const{ return standard / 1440; }
    int Minutes() const{ return standard % 1440; }

    std::pair<int, int> Date() const{
        //01-01: days = 0; 01-31: days = 30;
        int days = standard / 1440;
        std::pair<int, int> ret;
        for (int i = 1; i <= 12; i++)
            if (days < MonthDays[i]) {
                ret.first = i;
                break;
            }
        ret.second = days - MonthDays[ret.first - 1] + 1;
        return ret;
    }
    std::pair<int, int> Time() const{
        std::pair<int, int> ret;
        int mins = standard % 1440;
        ret.first = mins / 60;
        ret.second = mins % 60;
        return ret;
    }
    void print(){
        std::pair<int, int> ret;
        ret = Date();
        if (ret.first < 10) std::cout << '0'; std::cout << ret.first << '-';
        if (ret.second < 10) std::cout << '0'; std::cout << ret.second << ' ';
        ret = Time();
        if (ret.first < 10) std::cout << '0'; std::cout << ret.first << ':';
        if (ret.second < 10) std::cout << '0'; std::cout << ret.second;
    }
};
void output_empty_time();

void output_ReturnMode(ReturnMode ret, int timestamp = -1, string extra_info = "");

Command_Name get_Command_Name(const string& str);

ReturnMode get_arguments(std::string (&arguments)[26], const vector<string>& cur_tokens);

ReturnMode get_Command_Head(Command_Head& ret, const vector<string>& tokens);

vector<string> get_tokens();

using ArgPair = std::pair<char, int>;

//按照“|”分割
vector<string> split_by_vertical_bar(const string& str);

void output_tokens(const vector<string>& tokens);



#endif //TICKET_SYSTEM_2024_MYTOOLS_HPP
