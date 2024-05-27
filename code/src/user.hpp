//
// Created by skywa on 2024/5/25.
//

#ifndef TICKET_SYSTEM_2024_USER_HPP
#define TICKET_SYSTEM_2024_USER_HPP

#include "../tools/BPTree.hpp"
#include "../tools/MyTools.hpp"

struct User_info{
    type_userid userid; // username
    type_password password;
    type_realname realname;
    type_mailAddr mailAddr;
    type_privilege privilege{};

    bool operator<(const User_info& other) const { return privilege < other.privilege; }
    bool operator==(const User_info& other) const { return privilege == other.privilege; }
    bool operator>(const User_info& other) const { return privilege > other.privilege; }
    bool operator!=(const User_info& other) const { return privilege != other.privilege; }
    bool operator<=(const User_info& other) const { return privilege <= other.privilege; }
    bool operator>=(const User_info& other) const { return privilege >= other.privilege; }

    void output(){
        std::cout<<userid<<' '<<realname<<' '
                 <<mailAddr<<' '<<privilege<<std::endl;
    }
};

class User_system{
    BPTree<User_info> user_data; //数据库
    map<type_userid, User_info> user_online;  //登录用户列表
public:
    User_system();
    ~User_system();

    //---------------
    //在已登录用户中查找
    bool logged_in(User_info &ret, type_userid userid_);

    //登录
    bool login(type_userid userid_, User_info info_);

    void update_user_logged_in(type_userid userid_, User_info new_info_);

    //登出
    bool logout(type_userid userid_);

    //在数据库中查找
    bool User_exist(type_userid userid_);

    bool delete_User(type_userid userid_, User_info info_);

    bool get_User_info(User_info &ret, type_userid userid_); //return false is not exist


    bool add_User(type_userid userid_, User_info info_);

    bool modify_User(type_userid userid_, User_info pre_info_, User_info new_info_);

    bool empty();
};

#endif //TICKET_SYSTEM_2024_USER_HPP
