// Created by skywa on 2024/5/25.
#include <iostream>
#include <cstdio>
#include "user.hpp"
#include "../tools/MyTools.hpp"

vector<string> cur_tokens;
string arguments[26];
Command_Head com_head;
User_system user_system;

bool check_arguments(char ch, int sz){
    if (sz == -1){
        try{
            std::stoi(arguments[ch - 'a']);
        } catch (...){
            return false;
        }
    } else {
        if (arguments[ch - 'a'].size() > sz) return false;
    }
    return true;
}

int main(){
    User_info cur_user_info, other_user_info;
    type_trainID train_id;
    bool ret_bool;
    bool the_first_user = user_system.empty();
    ReturnMode ret_mode;

    bool system_open = true;
    while (system_open){
        cur_tokens = get_tokens();
        if (cur_tokens.empty()) continue;
        if (get_Command_Head(com_head, cur_tokens) != ReturnMode::Correct){
             continue;
        }
        if (get_arguments(arguments, cur_tokens) != ReturnMode::Correct){
            continue;
        }

        //time_stamp: com_head.first; command: com_head.second;
        //token_pair: -<key> and <argument>

        //保证所有指令输入格式均合法
        switch (com_head.second) {
            case Command_Name::add_user:
                //-c -u -p -n -m -g
//                if (!(check_arguments('c', 20) && check_arguments('u', 20) && check_arguments('p', 30) &&
//                    check_arguments('n', 15) && check_arguments('m', 30) && check_arguments('g', -1))) {
//                    continue;
//                }

                ret_mode = ReturnMode::Correct;
                if (the_first_user){
                    cur_user_info.privilege = 10;
                    cur_user_info.mailAddr = arguments['m' - 'a'];
                    cur_user_info.realname = arguments['n' - 'a'];
                    cur_user_info.password = arguments['p' - 'a'];
                    cur_user_info.userid = arguments['u' - 'a'];

                    user_system.add_User(cur_user_info.userid, cur_user_info);
                    the_first_user = false;

                } else if (user_system.logged_in(cur_user_info, type_userid(arguments['c' - 'a']))){


                }
                output_ReturnMode(ret_mode);
                break;
            case Command_Name::login:
                //-u -p
                break;
            case Command_Name::logout:
                break;
            case Command_Name::query_profile:
                break;
            case Command_Name::modify_profile:
                break;
            case Command_Name::delete_train:
                break;
            case Command_Name::release_train:
                break;
            case Command_Name::query_train:
                break;
            case Command_Name::query_ticket:
                break;
            case Command_Name::query_transfer:
                break;
            case Command_Name::buy_ticket:
                break;
            case Command_Name::query_order:
                break;
            case Command_Name::refund_ticket:
                break;
            case Command_Name::clean:
                break;
            case Command_Name::exit:
                system_open = false;
                break;
        }
    }
    return 0;
}