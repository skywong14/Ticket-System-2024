//
// Created by skywa on 2024/5/25.
//
#include "user.hpp"

bool User_system::logged_in(User_info &ret, type_userid userid) {
    return false;
}

bool User_system::get_User_info(User_info &ret, const string &userid_) {
    return false;
}

bool User_system::login(const string &userid_, const string &password_) {
    return false;
}

bool User_system::logout(const string &userid_) {
    return false;
}

bool User_system::delete_User(const string &userid_) {
    return false;
}

bool User_system::add_User(type_userid userid_, User_info info_) {

    return false;
}

bool User_system::modify_User(const string &userid_, User_info new_info_) {
    return false;
}

User_system::User_system():user_online() {
    user_data.initialise("user");
}

User_system::~User_system() {
    user_data.pop_all_buffer();
    user_online.clear();
}

bool User_system::empty() {
    return user_data.empty();
}

