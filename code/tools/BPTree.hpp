// Created by skywa on 2024/4/29.
#ifndef BPLUSTREE_BPTREE_HPP
#define BPLUSTREE_BPTREE_HPP
#pragma once
#include <cstdio>
#include <cstring>
#include "vector.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using sjtu::vector;

//each internal node with M keys and M+1 sons
template<class T, int Max_Nodes = 3000, int M = 300, int Buffer_Size = 500>
class BPTree{
private:
    fstream file, file_value;
    string index_filename, value_filename, filename;
    const int sizeofint = sizeof(int);
    const long long BASE = 197, MOD = 1e9+7;
    const int leaf_limit = (M + 1) / 2, internal_limit = M / 2;
    int sizeofBasicInformation = sizeof(Basic_Information);
    int sizeofNode = sizeof(Node);
    int sizeofNodeValue = sizeof(Node_Value);

    struct Basic_Information{
        int root_node_id = 0;
        int empty_node_id[Max_Nodes]{};
    };
    struct Node{
        int is_leaf = 0, size = 0, id = 0;
        int pre_node = 0, nxt_node = 0;
        long long index[M + 1]{};
        int sons[M + 2]{};
        Node() = default;
    };
    struct Node_Value{
        T values[M + 2]{};
    };

    Node root;
    Basic_Information basic_info;

    //-------directly on disk-------
    Basic_Information read_Basic_Information_disk(){
        file.open(index_filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(std::ios::beg);
        Basic_Information info;
        file.read((char*)&info, sizeofBasicInformation);
        file.close();
        return info;
    }
    void write_Basic_Information_disk(Basic_Information info_){
        file.open(index_filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(std::ios::beg);
        file.write(reinterpret_cast<char*>(&info_), sizeofBasicInformation);
        file.close();
    }
    Node read_Node_disk(int pos_){
        file.open(index_filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(sizeofBasicInformation + (pos_ - 1) * sizeofNode, std::ios::beg);
        Node node_;
        file.read((char*)&node_, sizeofNode);
        file.close();
        return node_;
    }

    void write_Node_disk(int pos_, Node node_){
        file.open(index_filename, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(sizeofBasicInformation + (pos_ - 1) * sizeofNode, std::ios::beg);
        file.write(reinterpret_cast<char*>(&node_), sizeofNode);
        file.close();
    }

    Node_Value read_Node_Value_disk(int pos_){
        file_value.open(value_filename, std::ios::in | std::ios::out | std::ios::binary);
        file_value.seekg((pos_ - 1) * sizeofNodeValue, std::ios::beg);
        Node_Value node_;
        file_value.read((char*)&node_, sizeofNodeValue);
        file_value.close();
        return std::move(node_);
    }
    void write_Node_Value_disk(int pos_, Node_Value node_){
        file_value.open(value_filename, std::ios::in | std::ios::out | std::ios::binary);
        file_value.seekp((pos_ - 1) * sizeofNodeValue, std::ios::beg);
        file_value.write(reinterpret_cast<char*>(&node_), sizeofNodeValue);
        file_value.close();
    }
    //-----------------------

    //--------Buffer Pool-------
    struct Buffer_Pool{
        Basic_Information basic_info;
        int node_size = 0, value_size = 0, info_flag = 0;
        int node_id[Buffer_Size]{}, value_id[Buffer_Size]{};
        int node_time[Buffer_Size]{}, value_time[Buffer_Size]{};
        int time_tag = 0;
        Node nodes[Buffer_Size];
        Node_Value values[Buffer_Size];
    }buffer;
    void refresh_basic_info(){
        write_Basic_Information_disk(buffer.basic_info);
    }
    void pop_node(int pos_){
        write_Node_disk(buffer.node_id[pos_], buffer.nodes[pos_]);
        buffer.node_size--;
        if (buffer.node_size > 0 && pos_ != buffer.node_size){
            buffer.node_id[pos_] = buffer.node_id[buffer.node_size];
            buffer.node_time[pos_] = buffer.node_time[buffer.node_size];
            buffer.nodes[pos_] = buffer.nodes[buffer.node_size];
        }
    }
    void pop_node_value(int pos_){
        write_Node_Value_disk(buffer.value_id[pos_], buffer.values[pos_]);
        buffer.value_size--;
        if (buffer.value_size > 0 && pos_ != buffer.value_size){
            buffer.value_id[pos_] = buffer.value_id[buffer.value_size];
            buffer.value_time[pos_] = buffer.value_time[buffer.value_size];
            buffer.values[pos_] = buffer.values[buffer.value_size];
        }
    }
    void add_node(int pos_, int id_, Node node_){
        buffer.node_id[pos_] = id_;
        buffer.node_time[pos_] = buffer.time_tag;
        buffer.nodes[pos_] = node_;
    }
    void add_node_value(int pos_, int id_, Node_Value val_){
        buffer.value_id[pos_] = id_;
        buffer.value_time[pos_] = buffer.time_tag;
        buffer.values[pos_] = val_;
    }
    bool Node_in_buffer(int id_){
        for (int i = 0; i < buffer.node_size; i++)
            if (buffer.node_id[i] == id_) return true;
        return false;
    }
    bool Node_Value_in_buffer(int id_){
        for (int i = 0; i < buffer.value_size; i++)
            if (buffer.value_id[i] == id_) return true;
        return false;
    }
    void push_node(int id_, Node node_){
        buffer.time_tag++;
        if (buffer.node_size < Buffer_Size){
            add_node(buffer.node_size, id_, node_);
            buffer.node_size++;
        }  else {
            int lst_time = buffer.time_tag, lst_id = -1;
            for (int i = 0; i < buffer.node_size; i++)
                if (buffer.node_time[i] < lst_time){
                    lst_time = buffer.node_time[i]; lst_id = i;
                }
            assert(lst_id >= 0);
            pop_node(lst_id);
            add_node(lst_id, id_, node_);
            buffer.node_size++;
        }
    }
    void push_node_value(int id_, Node_Value val_){
        buffer.time_tag++;
        if (buffer.value_size < Buffer_Size){
            add_node_value(buffer.value_size, id_, val_);
            buffer.value_size++;
        }  else {
            int lst_time = buffer.time_tag, lst_id = -1;
            for (int i = 0; i < buffer.value_size; i++)
                if (buffer.value_time[i] < lst_time){
                    lst_time = buffer.value_time[i]; lst_id = i;
                }
            pop_node_value(lst_id);
            add_node_value(lst_id, id_, val_);
            buffer.value_size++;
        }
    }
    void update_Node_in_buffer(int id_, Node node_){
        buffer.time_tag++;
        for (int i = 0; i < buffer.node_size; i++)
            if (buffer.node_id[i] == id_){
                buffer.node_time[i] = buffer.time_tag;
                buffer.nodes[i] = node_;
                return;
            }
    }
    void update_Node_Value_in_buffer(int id_, Node_Value val_){
        buffer.time_tag++;
        for (int i = 0; i < buffer.value_size; i++)
            if (buffer.value_id[i] == id_){
                buffer.value_time[i] = buffer.time_tag;
                buffer.values[i] = val_;
                return;
            }
    }
    Node get_Node(int id_){
        buffer.time_tag++;
        for (int i = 0; i < buffer.node_size; i++)
        if (buffer.node_id[i] == id_){
            buffer.node_time[i] = buffer.time_tag;
            return buffer.nodes[i];
        }
    }
    Node_Value get_Node_Value(int id_){
        buffer.time_tag++;
        for (int i = 0; i < buffer.value_size; i++)
            if (buffer.value_id[i] == id_){
                buffer.value_time[i] = buffer.time_tag;
                return buffer.values[i];
            }
    }
    //-----------------------------

    Basic_Information read_Basic_Information(){
        if (!buffer.info_flag){
            buffer.info_flag = 1;
            buffer.basic_info = read_Basic_Information_disk();
        }
        return buffer.basic_info;
    }
    void write_Basic_Information(Basic_Information info_){
        buffer.basic_info = info_;
    }
    Node read_Node(int pos_){
        if (Node_in_buffer(pos_)) return get_Node(pos_);
        else {
            Node node_ = read_Node_disk(pos_);
            push_node(pos_, node_);
            return node_;
        }
    }

    void write_Node(int pos_, Node node_){
        if (Node_in_buffer(pos_)){
            update_Node_in_buffer(pos_, node_);
        } else {
            push_node(pos_, node_);
        }
    }

    Node_Value read_Node_Value(int pos_){
        if (Node_Value_in_buffer(pos_)) return get_Node_Value(pos_);
        else {
            Node_Value val_ = read_Node_Value_disk(pos_);
            push_node_value(pos_, val_);
            return val_;
        }
    }
    void write_Node_Value(int pos_, Node_Value val_){
        if (Node_Value_in_buffer(pos_)){
            update_Node_Value_in_buffer(pos_, val_);
        } else {
            push_node_value(pos_, val_);
        }
    }

    int check_file_exists(const string& FN){
        fstream file_tmp(FN);
        if (file_tmp.is_open()){
            file_tmp.close();
            return 1;
        } else
            return 0;
    }
    void initialise_file(){
        int tmp = 0;
        file.open(index_filename, std::ios::out | std::ios::binary);
        file.write(reinterpret_cast<char *>(&tmp), sizeofint);
        file.close();

        file_value.open(value_filename, std::ios::out | std::ios::binary);
        file_value.write(reinterpret_cast<char *>(&tmp), sizeofint);
        file_value.close();
    }
    int allocate_node(){
        //使用后记得更新basic_info到文件，返回申请得到的node's id
        //如果返回0，说明出错
        int flag = 0;
        for (int i = 0; i < Max_Nodes; i++)
            if (basic_info.empty_node_id[i] > 0) {
                flag = basic_info.empty_node_id[i];
                basic_info.empty_node_id[i] = 0;
                break;
            }
        assert(flag != 0); //for debug only
        return flag;
    }
    void deallocate_node(int id_){
        basic_info.empty_node_id[id_ - 1] = id_;
    }
    void update_Node(int id_, Node node_){
        write_Node(id_, node_);
    }
    void update_Node_and_Values(int id_, Node node_, Node_Value val_){
        write_Node(id_, node_);
        write_Node_Value(id_, val_);
    }

    std::pair<vector<int>, Node> find_Node(long long index_hash, T value_){
        //vector end with -1 when the key&values is same
        vector<int> trace = {};
        int same_flag = 0;
        Node cur_node = read_Node(basic_info.root_node_id);
        trace.push_back(cur_node.id);

        while (!cur_node.is_leaf){
            int pos = cur_node.size;
            Node_Value node_values;
            int values_flag = 1;

            for (int i = 0; i < cur_node.size; i++){
                if (index_hash < cur_node.index[i]){
                    pos = i;
                    break;
                }
                if (index_hash == cur_node.index[i]){
                    if (values_flag){
                        node_values = read_Node_Value(cur_node.id);
                        values_flag = 0;
                    }
                    if (value_ < node_values.values[i]){
                        pos = i;
                        break;
                    }
                }
            }
            cur_node = read_Node(cur_node.sons[pos]);
            trace.push_back(cur_node.id);
        }
        //then cur_node is a leaf_node
        Node_Value values = read_Node_Value(cur_node.id);
        for (int i = 0; i < cur_node.size; i++){
            if (index_hash == cur_node.index[i] && value_ == values.values[i]) {
                same_flag = 1;
                break;
            }
        }
        if (same_flag) trace.push_back(-1);
        return std::make_pair(trace, cur_node);
    }

    void insert_node(Node cur_node, long long index_, T val_, int ptr_){
        //insert index_ and val_ inside cur_node
        Node_Value cur_values = read_Node_Value(cur_node.id);
        int pos = cur_node.size;
        for (int i = 0; i < cur_node.size; i++)
            if (cur_node.index[i] > index_ || cur_node.index[i] == index_ && cur_values.values[i] >val_){
                pos = i;
                break;
            }
        //then insert at position pos
        for (int i = cur_node.size; i > pos; i--){
            cur_node.index[i] = cur_node.index[i - 1];
            cur_node.sons[i + 1] =cur_node.sons[i - 1 + 1];
            cur_values.values[i] = cur_values.values[i - 1];
        }

        cur_node.index[pos] = index_;
        cur_node.sons[pos + 1] = ptr_;
        cur_values.values[pos] = val_;
        cur_node.size++;

        update_Node_and_Values(cur_node.id, cur_node, cur_values);
    }

    std::pair<int, int> get_siblings(int cur_id, Node parent_node){
        assert(!parent_node.is_leaf); //parent_node must be internal node
        int pos = -1;
        for (int i = 0; i <= parent_node.size; i++)
            if (cur_id == parent_node.sons[i]) { pos = i; break; }
        assert(pos >= 0);
        if (pos == 0) return std::make_pair(0, parent_node.sons[1]);
        if (pos == parent_node.size) return std::make_pair(parent_node.sons[parent_node.size - 1], 0);
        return std::make_pair(parent_node.sons[pos - 1], parent_node.sons[pos + 1]);
    }

    int borrow_from_pre_leaf(Node pre_node, Node cur_node, Node parent_node){
        if (pre_node.size <= leaf_limit) return 0;
        //return 1 if borrow successfully
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), pre_value = read_Node_Value(pre_node.id);

        long long ind_ = cur_node.index[0]; T val_ = cur_value.values[0];

        for (int i = cur_node.size; i >= 1; i--){
            cur_node.index[i] = cur_node.index[i - 1];
            cur_node.sons[i] = cur_node.sons[i - 1];
            cur_value.values[i] = cur_value.values[i - 1];
        }
        cur_node.size++;
        pre_node.size--;
        cur_node.index[0] = pre_node.index[pre_node.size];
        cur_node.sons[0] = pre_node.sons[pre_node.size];
        cur_value.values[0] = pre_value.values[pre_node.size];

        int pos = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.index[i] == ind_ && parent_value.values[i] == val_){
                pos = i;
                break;
            }
        assert(pos != -1); //for debug only
        parent_node.index[pos] = cur_node.index[0];
        parent_value.values[pos] = cur_value.values[0];

        update_Node_and_Values(cur_node.id, cur_node, cur_value);
        update_Node_and_Values(pre_node.id, pre_node, pre_value);
        update_Node_and_Values(parent_node.id, parent_node, parent_value);
        return 1;
    }
    int borrow_from_nxt_leaf(Node cur_node, Node nxt_node, Node parent_node){
        if (nxt_node.size <= leaf_limit) return 0;
        assert(cur_node.size == leaf_limit - 1); //for debug only
        //return 1 if borrow successfully
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), nxt_value = read_Node_Value(nxt_node.id);
        cur_node.index[cur_node.size] = nxt_node.index[0];
        cur_node.sons[cur_node.size] = nxt_node.sons[0];
        cur_value.values[cur_node.size] = nxt_value.values[0];
        cur_node.size++;
        nxt_node.size--;
        long long ind_ = nxt_node.index[0]; T val_ = nxt_value.values[0];
        for (int i = 0; i < nxt_node.size; i++){
            nxt_node.index[i] = nxt_node.index[i + 1];
            nxt_node.sons[i] = nxt_node.sons[i + 1];
            nxt_value.values[i] = nxt_value.values[i + 1];
        }
        int pos = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.index[i] == ind_ && parent_value.values[i] == val_){
                pos = i;
                break;
            }
        assert(pos != -1); //for debug only
        parent_node.index[pos] = nxt_node.index[0];
        parent_value.values[pos] = nxt_value.values[0];

        update_Node_and_Values(cur_node.id, cur_node, cur_value);
        update_Node_and_Values(nxt_node.id, nxt_node, nxt_value);
        update_Node_and_Values(parent_node.id, parent_node, parent_value);
        return 1;
    }

    int coalesce_pre_leaf(Node pre_node, Node cur_node, Node parent_node){
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), pre_value = read_Node_Value(pre_node.id);
        //merge
        int exist_nxt_node = cur_node.nxt_node;
        if (exist_nxt_node) {
            Node nxt_node = read_Node(exist_nxt_node);
            nxt_node.pre_node = pre_node.id;
            update_Node(nxt_node.id, nxt_node);
        }

        int pos = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.sons[i+1] == cur_node.id){
                pos = i;
                break;
            }
        assert(pos >= 0);

        pre_node.nxt_node = cur_node.nxt_node;
        for (int i = 0; i < cur_node.size; i++){
            pre_node.index[i + pre_node.size] = cur_node.index[i];
            pre_node.sons[i + pre_node.size] = cur_node.sons[i];
            pre_value.values[i + pre_node.size] = cur_value.values[i];
        }
        pre_node.size += cur_node.size;
        deallocate_node(cur_node.id);
        update_Node_and_Values(pre_node.id, pre_node, pre_value);

        return pos;
    }

    int borrow_from_pre_internal(Node pre_node, Node cur_node, Node parent_node){
        if (pre_node.size <= internal_limit) return 0;
        assert(cur_node.size == internal_limit - 1); //for debug only
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), pre_value = read_Node_Value(pre_node.id);

        int cur_in_parent = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.sons[i + 1] == cur_node.id){
                cur_in_parent = i;
                break;
            }
        assert(cur_in_parent > -1); //for debug only


        for (int i = cur_node.size; i >= 1; i--){
            cur_node.index[i] = cur_node.index[i - 1];
            cur_node.sons[i + 1] = cur_node.sons[i];
            cur_value.values[i] = cur_value.values[i - 1];
        }
        cur_node.sons[1] = cur_node.sons[0];
        cur_node.sons[0] = pre_node.sons[pre_node.size];
        cur_node.index[0] = parent_node.index[cur_in_parent];
        cur_value.values[0] = parent_value.values[cur_in_parent];
        cur_node.size++;
        pre_node.size--;
        parent_node.index[cur_in_parent] = pre_node.index[pre_node.size];
        parent_value.values[cur_in_parent]= pre_value.values[pre_node.size];

        update_Node_and_Values(cur_node.id, cur_node, cur_value);
        update_Node_and_Values(pre_node.id, pre_node, pre_value);
        update_Node_and_Values(parent_node.id, parent_node, parent_value);
        return 1;
    }
    int borrow_from_nxt_internal(Node cur_node, Node nxt_node, Node parent_node){
        if (nxt_node.size <= internal_limit) return 0;
        assert(cur_node.size == internal_limit - 1); //for debug only
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), nxt_value = read_Node_Value(nxt_node.id);

        int nxt_in_parent = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.sons[i + 1] == nxt_node.id){
                nxt_in_parent = i;
                break;
            }
        assert(nxt_in_parent > -1); //for debug only


        cur_node.index[cur_node.size] = parent_node.index[nxt_in_parent];
        cur_value.values[cur_node.size] = parent_value.values[nxt_in_parent];
        cur_node.sons[cur_node.size + 1] = nxt_node.sons[0];
        cur_node.size++;

        parent_node.index[nxt_in_parent] = nxt_node.index[0];
        parent_value.values[nxt_in_parent] = nxt_value.values[0];

        nxt_node.sons[0] = nxt_node.sons[1];
        for (int i = 0; i < nxt_node.size - 1; i++){
            nxt_node.index[i] = nxt_node.index[i + 1];
            nxt_node.sons[i + 1] = nxt_node.sons[i + 2];
            nxt_value.values[i] = nxt_value.values[i + 1];
        }
        nxt_node.size--;

        update_Node_and_Values(cur_node.id, cur_node, cur_value);
        update_Node_and_Values(nxt_node.id, nxt_node, nxt_value);
        update_Node_and_Values(parent_node.id, parent_node, parent_value);
        return 1;
    }

    int coalesce_pre_internal(Node pre_node, Node cur_node, Node parent_node){
        Node_Value parent_value = read_Node_Value(parent_node.id);
        Node_Value cur_value = read_Node_Value(cur_node.id), pre_value = read_Node_Value(pre_node.id);

        int cur_in_parent = -1;
        for (int i = 0; i < parent_node.size; i++)
            if (parent_node.sons[i + 1] == cur_node.id){
                cur_in_parent = i;
                break;
            }
        assert(cur_in_parent > -1); //for debug only

        int exist_nxt_node = cur_node.nxt_node;
        if (exist_nxt_node) {
            Node nxt_node = read_Node(exist_nxt_node);
            nxt_node.pre_node = pre_node.id;
            update_Node(nxt_node.id, nxt_node);
        }

        pre_node.index[pre_node.size] = parent_node.index[cur_in_parent];
        pre_node.sons[pre_node.size + 1] = cur_node.sons[0];
        pre_value.values[pre_node.size] = parent_value.values[cur_in_parent];
        pre_node.size++;
        pre_node.nxt_node = cur_node.nxt_node;

        for (int i = 0; i < cur_node.size; i++){
            pre_node.index[i + pre_node.size] = cur_node.index[i];
            pre_node.sons[i + pre_node.size + 1] = cur_node.sons[i + 1];
            pre_value.values[i + pre_node.size] = cur_value.values[i];
        }
        pre_node.size += cur_node.size;
        deallocate_node(cur_node.id);
        update_Node_and_Values(pre_node.id, pre_node, pre_value);

        return cur_in_parent;
    }

    //ATTENTION "const vector<int>& path"
    void erase_internal(Node cur_node, int pos, const vector<int>& path, int path_cnt){
        //delete the index&val at pos in cur_node(must be an internal node)
        //also delete the pos_ptr at pos+1
        //path[path_cnt] == cur_node.id

        Node_Value cur_value = read_Node_Value(cur_node.id);
        T deleted_val = cur_value.values[pos];


        for (int i = pos; i < cur_node.size - 1; i++){
            cur_node.index[i] = cur_node.index[i + 1];
            cur_node.sons[i + 1] = cur_node.sons[i + 2];
            cur_value.values[i] = cur_value.values[i + 1];
        }
        cur_node.size--;

        if (basic_info.root_node_id == cur_node.id){
            //this leaf node is root
            if (cur_node.size == 0){
                //move root
                basic_info.root_node_id = cur_node.sons[0];
                deallocate_node(cur_node.id);
                write_Basic_Information(basic_info);
                return;
            }
            //the tree has only one node
            update_Node_and_Values(cur_node.id, cur_node, cur_value);
            return;
        }
        update_Node_and_Values(cur_node.id, cur_node, cur_value);

        assert(path_cnt >= 1); //for debug only

        Node parent_node;
        Node sib_node;
        Node_Value parent_values;

        if (cur_node.size < internal_limit){
            //an internal node
            //unbalance (at least size >= internal_limit)
            parent_node = read_Node(path[path_cnt - 1]);

            std::pair<int, int> siblings = get_siblings(cur_node.id, parent_node); //pre and nxt

            int borrow_flag = 0;
            //1.1 try to borrow from pre_node
            if (siblings.first > 0){
                sib_node = read_Node(siblings.first);
                borrow_flag = borrow_from_pre_internal(sib_node, cur_node, parent_node);
            }
            //1.2 if 1.1 fails, borrow from nxt_node
            if (!borrow_flag && siblings.second > 0){
                sib_node = read_Node(siblings.second);
                borrow_flag = borrow_from_nxt_internal(cur_node, sib_node, parent_node);
            }
            //2.if fail, merge and delete on the internal nodes
            if (!borrow_flag){
                int internal_pos = -1;
                if (siblings.first > 0){
                    // Coalesce Left
                    sib_node = read_Node(siblings.first);
                    internal_pos = coalesce_pre_internal(sib_node, cur_node, parent_node);
                } else {
                    // Coalesce Right
                    assert(siblings.second > 0);
                    sib_node = read_Node(siblings.second);
                    internal_pos = coalesce_pre_internal(cur_node, sib_node, parent_node);
                }
                //then delete on parent_node
                erase_internal(parent_node, internal_pos, path, path_cnt - 1);
            }
        }
    }

    void output_dfs(int id, int space){
        Node n = read_Node_disk(id);
        Node_Value v = read_Node_Value_disk(id);
        for (int ii= 0; ii < space; ii++) std::cout<<' ';
        std::cout<<"Node_"<<id<<':'<<" size="<<n.size<<','<<','<<" is_leaf="<<n.is_leaf<<std::endl;
        if (n.is_leaf){
            for (int i = 0; i < n.size; i++){
                for (int ii= 0; ii < space; ii++) std::cout<<' ';
                std::cout<<"|son_"<<i<<':'<<n.index[i]<<','<<v.values[i]<<std::endl;
            }
        }else{
            for (int ii= 0; ii < space; ii++) std::cout<<' ';
            std::cout<<"|son_0:"<<std::endl;
            output_dfs(n.sons[0], space + 3);
            for (int i = 1; i <= n.size; i++){
                for (int ii= 0; ii < space; ii++) std::cout<<' ';
                std::cout<<"|son_"<<i<<": "<<n.index[i - 1]<<", "<<v.values[i - 1]<<std::endl;
                output_dfs(n.sons[i], space + 3);
            }
        }
    }

public:

    //buffer:
    void pop_all_buffer(){
        refresh_basic_info();
        for (int i = 0; i < buffer.node_size; i++){
            write_Node_disk(buffer.node_id[i], buffer.nodes[i]);
        }
        for (int i = 0; i < buffer.value_size; i++){
            write_Node_Value_disk(buffer.value_id[i], buffer.values[i]);
        }
        buffer.time_tag = 1;
        buffer.node_size = buffer.value_size = buffer.info_flag = 0;
    }

    long long get_Hash(const string& str1){
        long long ha = 0;
        for (int i = 0; i < str1.length(); i++)
            ha = (ha * BASE + (long long)(str1[i]) ) % MOD;
        return ha;
    }


    int initialise(string FN = "", int clear_file = 0){
        if (!FN.empty()) filename = FN;
        index_filename = filename + "_index.bpt";
        value_filename = filename + "_value.bpt";

        if (check_file_exists(index_filename) && check_file_exists(value_filename) && (!clear_file)) return 0; //文件已经存在就无需初始化

        initialise_file();
        file.open(index_filename, std::ios::in | std::ios::out | std::ios::binary);
        assert(file.is_open());
        file.close();
        Basic_Information info1;
        info1.root_node_id = 0; //0 means nullptr
        for (int i = 1; i <= Max_Nodes; i++)
            info1.empty_node_id[i-1] = i;
        write_Basic_Information_disk(info1);

        file_value.open(value_filename, std::ios::in | std::ios::out | std::ios::binary);
        assert(file_value.is_open());
        file_value.close();
        return 1;
    }


    int insert(const string& str1, T value_){
        //return 0 is key&value is same
        long long index_hash = get_Hash(str1);

        basic_info = read_Basic_Information();

        if (basic_info.root_node_id == 0){ //BPTree is empty
            basic_info.root_node_id = allocate_node();
            Node new_node;
            Node_Value new_node_value;
            new_node.id = basic_info.root_node_id;
            new_node.size = 1;
            new_node.is_leaf = 1;
            new_node.index[0] = index_hash;
            new_node.sons[0] = 0;
            new_node_value.values[0] = value_;
            update_Node_and_Values(new_node.id, new_node, new_node_value);
        } else {
            std::pair<vector<int>, Node> ret_ = find_Node(index_hash, value_);
            if (ret_.first.back() == -1) return 0;
            Node cur_node = ret_.second;
            vector<int> trace = ret_.first;
            int trace_cnt = int(trace.size()) - 1;
            //then insert in cur_node and not the head of cur_node is guaranteed
            //we need to insert index_hash&inserted_value to cur_node
            long long inserted_index = index_hash;
            int inserted_ptr = 0;
            T inserted_value = value_;

            while (true){
                if (cur_node.size < M){
                    insert_node(cur_node, inserted_index, inserted_value, inserted_ptr);
                    break;
                } else {
                    //split
                    //insert on node then split the node
                    //cur_node.size = M (attention: but has M + 1 sons)
                    int exist_nxt_node = cur_node.nxt_node;
                    Node new_node, nxt_node;
                    Node_Value new_values, cur_values = read_Node_Value(cur_node.id);

                    //insert first
                    int pos = cur_node.size;
                    for (int i = 0; i < cur_node.size; i++)
                        if (cur_node.index[i] > inserted_index || cur_node.index[i] == inserted_index && cur_values.values[i] >inserted_value){
                            pos = i;
                            break;
                        }
                    for (int i = cur_node.size; i > pos; i--){
                        cur_node.index[i] = cur_node.index[i - 1];
                        cur_node.sons[i + 1] =cur_node.sons[i - 1 + 1];
                        cur_values.values[i] = cur_values.values[i - 1];
                    }
                    cur_node.index[pos] = inserted_index;
                    cur_node.sons[pos + 1] = inserted_ptr;
                    cur_values.values[pos] = inserted_value;
                    cur_node.size++;

                    //then split, now cur_node.size = M + 1
                    new_node.id = allocate_node();
                    if (cur_node.is_leaf){
                        cur_node.size = (M + 1) / 2;
                        new_node.size = M + 1 - (M + 1) / 2;
                        for (int i = 0; i < new_node.size; i++){
                            new_node.index[i] = cur_node.index[i + (M + 1) / 2];
                            new_node.sons[i] = cur_node.sons[i + (M + 1) / 2 + 1];
                            new_values.values[i] = cur_values.values[i + (M + 1) / 2];
                        }
                        if (exist_nxt_node){
                            nxt_node = read_Node(cur_node.nxt_node);
                            nxt_node.pre_node = new_node.id;
                        }
                        new_node.is_leaf = cur_node.is_leaf;
                        new_node.nxt_node = cur_node.nxt_node; new_node.pre_node = cur_node.id;
                        cur_node.nxt_node = new_node.id;

                        inserted_index = new_node.index[0];
                        inserted_value = new_values.values[0];
                        inserted_ptr = new_node.id;
                    } else {
                        new_node.size = M - M / 2;
                        for (int i = 0; i < new_node.size; i++){
                            new_node.index[i] = cur_node.index[i + M / 2 + 1];
                            new_node.sons[i] = cur_node.sons[i + M / 2 + 1];
                            new_values.values[i] = cur_values.values[i + M / 2 + 1];
                        }
                        new_node.sons[new_node.size] = cur_node.sons[cur_node.size];
                        cur_node.size = M / 2;

                        if (exist_nxt_node){
                            nxt_node = read_Node(cur_node.nxt_node);
                            nxt_node.pre_node = new_node.id;
                        }
                        new_node.is_leaf = cur_node.is_leaf;
                        new_node.nxt_node = cur_node.nxt_node; new_node.pre_node = cur_node.id;
                        cur_node.nxt_node = new_node.id;

                        inserted_index = cur_node.index[M / 2];
                        inserted_value = cur_values.values[M / 2];
                        inserted_ptr = new_node.id;
                    }

                    if (cur_node.id == basic_info.root_node_id){
                        Node new_root;
                        Node_Value new_root_values;
                        new_root.id = allocate_node();
                        basic_info.root_node_id = new_root.id;
                        new_root.size = 1;
                        new_root.is_leaf = 0;
                        new_root.index[0] = inserted_index;
                        new_root_values.values[0] = inserted_value;
                        new_root.sons[0] = cur_node.id; new_root.sons[1] = inserted_ptr;

                        if (exist_nxt_node) update_Node(nxt_node.id, nxt_node);
                        update_Node_and_Values(new_root.id, new_root, new_root_values);
                        update_Node_and_Values(cur_node.id, cur_node, cur_values);
                        update_Node_and_Values(new_node.id, new_node, new_values);
                        break;
                    }
                    if (exist_nxt_node) update_Node(nxt_node.id, nxt_node);
                    update_Node_and_Values(cur_node.id, cur_node, cur_values);
                    update_Node_and_Values(new_node.id, new_node, new_values);

                    trace_cnt--;
                    cur_node = read_Node(trace[trace_cnt]);
                }
            }

        }
        write_Basic_Information(basic_info);
        return 1;
    }

    vector<T> search_values(const string& str_index){
        long long index_hash = get_Hash(str_index);
        vector<T> val = {};
        basic_info = read_Basic_Information();
        if (basic_info.root_node_id == 0){
            return val;
        }
        Node cur_node = read_Node(basic_info.root_node_id);

        while (!cur_node.is_leaf){
            int pos = cur_node.size;
            Node_Value node_values;

            for (int i = 0; i < cur_node.size; i++)
                if (index_hash <= cur_node.index[i]){
                    pos = i;
                    break;
                }
            if (cur_node.sons[pos] <= 0 || cur_node.sons[pos] >= Max_Nodes) throw std::runtime_error("out of range"); //for debug
            cur_node = read_Node(cur_node.sons[pos]);
        }
        //then cur_node is a leaf_node
        while (cur_node.index[cur_node.size - 1] < index_hash && cur_node.nxt_node > 0) {
            cur_node = read_Node(cur_node.nxt_node);
        }

        Node_Value values = read_Node_Value(cur_node.id);
        int flag = 1;
        while (flag){
            for (int i = 0; i < cur_node.size; i++)
                if (cur_node.index[i] == index_hash){
                    val.push_back(values.values[i]);
                } else if (cur_node.index[i] > index_hash) {flag = 0; break;}
            if (cur_node.nxt_node > 0) {
                cur_node = read_Node(cur_node.nxt_node);
                values = read_Node_Value(cur_node.id);
            }
            else break;
        }
        return val;
    }


    bool empty(){
        basic_info = read_Basic_Information();
        if (basic_info.root_node_id == 0) return true;
        return false;
    }

    int erase(const string& str1, T value_){
        basic_info = read_Basic_Information();
        long long index_hash = get_Hash(str1);
        std::pair<vector<int>, Node> ret = find_Node(index_hash, value_);
        vector<int> path = ret.first;
        Node cur_node = ret.second;
        Node_Value cur_value = read_Node_Value(cur_node.id);
        if (path.back() != -1) return 0; //cannot find index&val in the BPTree
        path.pop_back();
        //delete on leaf first
        int pos = -1;
        for (int i = 0; i < cur_node.size; i++)
            if (index_hash == cur_node.index[i] && value_ == cur_value.values[i]) { pos = i; break; }
        assert(pos >= 0); //for debug only
        for (int i = pos; i < cur_node.size - 1; i++){
            cur_node.index[i] = cur_node.index[i + 1];
            cur_node.sons[i] = cur_node.sons[i + 1];
            cur_value.values[i] = cur_value.values[i + 1];
        }
        cur_node.size--;

        if (basic_info.root_node_id == cur_node.id){
            //this leaf node is root
            if (cur_node.size == 0){
                //the whole tree is empty
                basic_info.root_node_id = 0;
                deallocate_node(cur_node.id);
                write_Basic_Information(basic_info);
                return 1;
            }
            //the tree has only one node
            update_Node_and_Values(cur_node.id, cur_node, cur_value);
            return 1;
        }
        update_Node_and_Values(cur_node.id, cur_node, cur_value);

        assert(path.size() >= 2); //for debug only

        if (pos == 0 && cur_node.pre_node != 0){
            //if delete on head, update the key
            long long substitution_index = cur_node.index[0];
            T substitution_val = cur_value.values[0];
            Node tmp_node; Node_Value tmp_value;
            int sub_flag = 0;
            for (int i = int(path.size()) - 2; i >= 0; i--){
                tmp_node = read_Node(path[i]);
                tmp_value = read_Node_Value(path[i]);
                for (int j = 0; j < tmp_node.size; j++)
                    if (tmp_node.index[j] == index_hash && tmp_value.values[j] == value_){
                        sub_flag = 1;
                        tmp_node.index[j] = substitution_index;
                        tmp_value.values[j] = substitution_val;
                        update_Node_and_Values(tmp_node.id, tmp_node, tmp_value);
                        break;
                    }
                if (sub_flag) break;
            }
            //if sub_flag == 0, then we erase at the first of all tree(but pre_node must be zero)
            assert(sub_flag); // for debug only
        }

        Node parent_node;
        Node sib_node;
        Node_Value parent_values;

        if (cur_node.size < leaf_limit){
            //a leaf node
            //unbalance (at least size >= M / 2)
            assert(path[path.size() - 2] > 0); //for debug only
            parent_node = read_Node(path[path.size() - 2]);

            std::pair<int, int> siblings = get_siblings(cur_node.id, parent_node); //pre and nxt
            int borrow_flag = 0;
            //1.1 try to borrow from pre_node
            if (siblings.first > 0){
                assert(cur_node.pre_node == siblings.first);
                sib_node = read_Node(siblings.first);
                borrow_flag = borrow_from_pre_leaf(sib_node, cur_node, parent_node);
            }
            //1.2 if 1.1 fails, borrow from nxt_node
            if (!borrow_flag && siblings.second > 0){
                assert(cur_node.nxt_node == siblings.second);
                sib_node = read_Node(siblings.second);
                borrow_flag = borrow_from_nxt_leaf(cur_node, sib_node, parent_node);
            }
            //2.if fail, merge and delete on the internal nodes
            if (!borrow_flag){
                int internal_pos = -1;
                if (siblings.first > 0){
                    // Coalesce Left
                    sib_node = read_Node(siblings.first);
                    internal_pos = coalesce_pre_leaf(sib_node, cur_node, parent_node);
                } else {
                    // Coalesce Right
                    sib_node = read_Node(siblings.second);
                    internal_pos = coalesce_pre_leaf(cur_node, sib_node, parent_node);
                }
                //then delete on parent_node
                erase_internal(parent_node, internal_pos, path, path.size() - 2);
            }
        }
        write_Basic_Information(basic_info);
        return 1;
    }

    void output_tree_structure(){
        std::cout<<">>>>>>>>>>>>>>>"<<std::endl;
        Basic_Information info_ = read_Basic_Information_disk();
        if (info_.root_node_id == 0) {
            std::cout<<"Empty!!!"<<std::endl;
        }else
        output_dfs(info_.root_node_id,0);
        std::cout<<"<<<<<<<<<<<<"<<std::endl;
    }

    BPTree() = default;
    explicit BPTree(const string& FN) : filename(FN) {}

    ~BPTree() {
        pop_all_buffer();
    }

};
#endif //BPLUSTREE_BPTREE_HPP
