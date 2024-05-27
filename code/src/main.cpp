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
    freopen("Mytest.txt","r",stdin);
    User_info cur_user_info, other_user_info, tmp_user_info;
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
            output_ReturnMode(ReturnMode::Correct, com_head.first,  " for_test");
            case Command_Name::add_user:
                //-c -u -p -n -m -g
//                if (!(check_arguments('c', 20) && check_arguments('u', 20) && check_arguments('p', 30) &&
//                    check_arguments('n', 15) && check_arguments('m', 30) && check_arguments('g', -1))) {
//                    continue;
//                }
                ret_mode = ReturnMode::Other_Error;

                if (the_first_user){
                    cur_user_info.privilege = 10;
                    cur_user_info.mailAddr = arguments['m' - 'a'];
                    cur_user_info.realname = arguments['n' - 'a'];
                    cur_user_info.password = arguments['p' - 'a'];
                    cur_user_info.userid = arguments['u' - 'a'];

                    user_system.add_User(cur_user_info.userid, cur_user_info);
                    the_first_user = false;
                    ret_mode = ReturnMode::Correct;

                } else if (user_system.logged_in(cur_user_info, type_userid(arguments['c' - 'a']))){ //已登陆
                    if (cur_user_info.password == arguments['p' - 'a'] //密码正确
                        && cur_user_info.privilege > std::stoi(arguments['g' - 'a']) //权限更高
                        && !user_system.User_exist(arguments['u' - 'a']) ) { // 新增用户不存在

                        other_user_info.privilege = std::stoi(arguments['g' - 'a']);
                        other_user_info.mailAddr = arguments['m' - 'a'];
                        other_user_info.realname = arguments['n' - 'a'];
                        other_user_info.password = arguments['p' - 'a'];
                        other_user_info.userid = arguments['u' - 'a'];
                        user_system.add_User(other_user_info.userid, other_user_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }

                output_ReturnMode(ret_mode, com_head.first);
                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::login:
                //-u -p
                ret_mode = ReturnMode::Other_Error;

                if (!user_system.logged_in(other_user_info, arguments['u' - 'a']) //未登录
                        && user_system.get_User_info(cur_user_info, arguments['u' - 'a'])){ //user exist
                    if (cur_user_info.password == arguments['p' - 'a']){ //密码正确
                        user_system.login(cur_user_info.userid, cur_user_info);
                        ret_mode = ReturnMode::Correct;
                    }
                }

                output_ReturnMode(ret_mode, com_head.first);
                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::logout:
                //-u
                ret_mode = ReturnMode::Other_Error;

                if (user_system.logged_in(other_user_info, arguments['u' - 'a'])){
                    user_system.logout(arguments['u' - 'a']);
                    ret_mode = ReturnMode::Correct;
                }

                output_ReturnMode(ret_mode, com_head.first);
                if (ret_mode == ReturnMode::Correct) std::cout<<0<<std::endl;
                else std::cout<<-1<<std::endl;
                break;
            case Command_Name::query_profile:
                // -c -u
                ret_mode = ReturnMode::Other_Error;
                if (user_system.logged_in(cur_user_info, arguments['c' - 'a'])
                    && user_system.get_User_info(other_user_info, arguments['u' - 'a'])){
                    if (cur_user_info.privilege >= other_user_info.privilege){
                        std::cout<<other_user_info.userid<<' '<<other_user_info.realname<<' '
                                 <<other_user_info.mailAddr<<' '<<other_user_info.privilege<<std::endl;
                        ret_mode = ReturnMode::Correct;
                    }
                }

                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
                break;
            case Command_Name::modify_profile:
                // -c -u (-p) (-n) (-m) (-g)
                ret_mode = ReturnMode::Other_Error;
                if (user_system.logged_in(cur_user_info, arguments['c' - 'a'])
                    && user_system.get_User_info(other_user_info, arguments['u' - 'a'])){
                    if (cur_user_info.userid == other_user_info.userid){ //修改自己
                        if (arguments['g' - 'a'].empty() || cur_user_info.privilege > std::stoi(arguments['g' - 'a'])){ //权限要求
                            if (!arguments['p' - 'a'].empty())
                                other_user_info.password = arguments['p' - 'a'];
                            if (!arguments['n' - 'a'].empty())
                                other_user_info.realname = arguments['n' - 'a'];
                            if (!arguments['m' - 'a'].empty())
                                other_user_info.mailAddr = arguments['m' - 'a'];
                            if (!arguments['g' - 'a'].empty())
                                other_user_info.privilege = std::stoi(arguments['g' - 'a']);

                            user_system.modify_User(cur_user_info.userid, cur_user_info, other_user_info);
                            user_system.update_user_logged_in(cur_user_info.userid, other_user_info);

                            ret_mode = ReturnMode::Correct;
                        }
                    } else { //修改别的用户
                        if (arguments['g' - 'a'].empty() || cur_user_info.privilege > std::stoi(arguments['g' - 'a'])){ //权限要求
                            tmp_user_info = other_user_info;
                            if (!arguments['p' - 'a'].empty())
                                other_user_info.password = arguments['p' - 'a'];
                            if (!arguments['n' - 'a'].empty())
                                other_user_info.realname = arguments['n' - 'a'];
                            if (!arguments['m' - 'a'].empty())
                                other_user_info.mailAddr = arguments['m' - 'a'];
                            if (!arguments['g' - 'a'].empty())
                                other_user_info.privilege = std::stoi(arguments['g' - 'a']);

                            user_system.modify_User(other_user_info.userid, tmp_user_info, other_user_info);
                            if (user_system.logged_in(tmp_user_info, other_user_info.userid)){
                                user_system.update_user_logged_in(other_user_info.userid, other_user_info);
                            }

                            ret_mode = ReturnMode::Correct;
                        }
                    }
                }

                output_ReturnMode(ret_mode, com_head.first);
                if (ret_mode != ReturnMode::Correct) std::cout<<-1<<std::endl;
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
                std::cout<<"bye"<<std::endl;
                system_open = false;
                break;
        }
    }
    return 0;
}