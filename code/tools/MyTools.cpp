//
// Created by skywa on 2024/5/26.
//
#include "MyTools.hpp"

Command_Name get_Command_Name(const string& str){
    if (str == "add_user") return Command_Name::add_user;
    else if (str == "login") return Command_Name::login;
    else if (str == "logout") return Command_Name::logout;
    else if (str == "query_profile") return Command_Name::query_profile;
    else if (str == "modify_profile") return Command_Name::modify_profile;
    else if (str == "add_train") return Command_Name::add_train;
    else if (str == "delete_train") return Command_Name::delete_train;
    else if (str == "release_train") return Command_Name::release_train;
    else if (str == "query_train") return Command_Name::query_train;
    else if (str == "query_ticket") return Command_Name::query_ticket;
    else if (str == "query_transfer") return Command_Name::query_transfer;
    else if (str == "buy_ticket") return Command_Name::buy_ticket;
    else if (str == "query_order") return Command_Name::query_order;
    else if (str == "refund_ticket") return Command_Name::refund_ticket;
    else if (str == "clean") return Command_Name::clean;
    else if (str == "exit") return Command_Name::exit;
    else return Command_Name::unknown;
}

void output_ReturnMode(ReturnMode ret, int timestamp, string extra_info){
//    return;
    std::cout<<'['<<timestamp<<"] ";
    if (ret == ReturnMode::Correct) std::cout<<"Correct!"<<std::endl;
    if (ret != ReturnMode::Correct) std::cout<<"Invalid:";
    if (1){
        if (ret == ReturnMode::Invalid_Format) std::cout<<" Invalid_Format";
        if (ret == ReturnMode::Wrong_Value) std::cout<<" Wrong_Value";
        if (ret == ReturnMode::Invalid_Operation) std::cout<<" Invalid_Operation";
        if (ret == ReturnMode::Lack_Permission) std::cout<<" Lack_Permission";
        if (ret == ReturnMode::Out_Of_Range) std::cout<<" Out_Of_Range";
        if (ret == ReturnMode::Other_Error) std::cout<<" Other_Error";
        if (ret != ReturnMode::Correct) std::cout<<" "<<extra_info;
    }
    if (ret != ReturnMode::Correct) std::cout<<std::endl;
}

string empty_time_string(){
    return string("xx-xx xx:xx");
}

type_time setOffDate(type_time arriveDate, type_time startTime, type_time stopTime, type_time costTime){
    return type_time( (arriveDate.Days() -  (startTime + stopTime + costTime).Days()) * 1440  );
    //startdate + date(starttime + cur_route.arrive[posStart] + cur_route.stop[posStart]) ==  date(arriveDate)
}

string to_index(type_time time_, type_trainID id_){
    return std::to_string(time_.standard) + '|' + id_.to_string();
}

ReturnMode get_arguments(std::string (&arguments)[26], const vector<string>& cur_tokens){
    for (int i = 0; i < 26; i++)
        arguments[i].clear();
    if (cur_tokens.size() % 2 != 0) return ReturnMode::Invalid_Format;
    for (int i = 2; i < cur_tokens.size(); i += 2){
        if (cur_tokens[i][0] != '-' ||cur_tokens[i].size() != 2) return ReturnMode::Invalid_Format;
        if (cur_tokens[i][1] - 'a' < 0 || cur_tokens[i][1] - 'a' >= 26) return ReturnMode::Invalid_Format;
        arguments[cur_tokens[i][1] - 'a'] = cur_tokens[i+1];
    }
    return ReturnMode::Correct;
}

ReturnMode get_Command_Head(Command_Head& ret, const vector<string>& tokens){
    //检查时间戳和命令是否存在
    if (tokens.size() < 2) return ReturnMode::Invalid_Format;
    string str = tokens[0];
    if (str[0] != '[' || str[str.size() - 1] != ']') return ReturnMode::Invalid_Format;
    for (int i = 1; i < str.size() - 1; i++)
        if (!isdigit(str[i])) return ReturnMode::Invalid_Format;
    try{
        ret.first = std::stoi(str.substr(1, str.size() - 2));
    } catch (...){
        return ReturnMode::Invalid_Format;
    }
    ret.second = get_Command_Name(tokens[1]);
    if (ret.second == Command_Name::unknown) return ReturnMode::Invalid_Format;
    return ReturnMode::Correct;
}

vector<string> get_tokens() {
    vector<string> tokens;
    if (std::cin.eof()) { tokens.push_back("exit"); return tokens; }
    string str;
    std::getline(std::cin, str);
    std::istringstream iss(str); // 创建字符串流
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void output_tokens(const vector<string>& tokens){
    for (int i = 0; i < tokens.size(); i++){
        std::cout<<tokens[i]<<',';
    }
    std::cout<<std::endl;
}


vector<string> split_by_vertical_bar(const std::string& _str){
    vector<string> result;
    std::stringstream ss(_str);
    string item;
    while (std::getline(ss, item, '|'))
        result.push_back(item);
    return result;
}




