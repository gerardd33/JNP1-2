#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include "poset.h"

#define NDEBUG

//using Id = unsigned long;

using Values = std::vector<std::string>;
using Values_ids = std::unordered_map<std::string, unsigned long>;
using Adjacency_list = std::vector<std::vector<unsigned long>>;

using Poset = std::pair<Values, std::pair<Values_ids, Adjacency_list>>;

unsigned long next_poset_id = 0;
std::vector<Poset> posets;
std::vector<bool> deleted_posets;

unsigned long poset_new() {

    Poset poset;
    posets.push_back(poset);
    deleted_posets.push_back(false);

    return next_poset_id++;
}


void poset_delete(unsigned long id) {

    if (id < next_poset_id && !deleted_posets[id]) {
        poset_clear(id);
        deleted_posets[id] = true;
    }
}

bool poset_exists(unsigned long id) {
    return id < next_poset_id && !deleted_posets[id];
}

bool value_in_poset(unsigned long id, char const *value) {

    assert(poset_exists(id));

    Values_ids* curr_poset = &posets[id].second.first;
    return (*curr_poset).find(value) != (*curr_poset).end();
}

bool is_valid_value(char const *value) {
    return value != nullptr; // value cant be null, and thats it (?)
}

size_t poset_size(unsigned long id) {

    if (poset_exists(id)) {
        return posets[id].second.first.size();
    }

    return 0;
}

void poset_clear(unsigned long id) {

    if (poset_exists(id)) {
        posets[id].first.clear(); // dk if we have to additionally delete these strings by hand
        posets[id].second.first.clear();
        posets[id].second.second.clear();
    }
}

bool poset_insert(unsigned long id, char const *value) {

    if (poset_exists(id) && is_valid_value(value) && !value_in_poset(id, value)) {

        std::string new_value = value;
        unsigned long new_id = posets[id].second.second.size();

        std::vector<unsigned long> empty_vec;

        posets[id].first.push_back(new_value);
        posets[id].second.first.insert(std::make_pair(new_value, new_id));
        posets[id].second.second.push_back(empty_vec);

        #ifndef NDEBUG
        std::cerr << new_value << " " << new_id << "\n";
        #endif
        return true;
    }

    return false;
}

bool bfs_search(unsigned long id, unsigned long start_id, unsigned long destination_id) {

    std::queue<unsigned long> queue;
    std::vector<bool> visited(posets[id].second.second.size(), false);

    queue.push(start_id);

    while (!queue.empty()) {
        unsigned long curr_id = queue.front();
        queue.pop();

        if (curr_id == destination_id) {
            return true;
        }

        std::string* value = &posets[id].first[curr_id];
        unsigned long value_id = posets[id].second.first[*value];
        std::vector<unsigned long>* neighbours = &posets[id].second.second[value_id];

        for (unsigned long neighbour : *neighbours) {
            if (!visited[neighbour]) {
                visited[neighbour] = true;
                queue.push(neighbour);
            }
        }
    }

    return false;
}

bool find_path(unsigned long id, char const *value1, char const *value2) {

    Values_ids* values_ids = &posets[id].second.first;

    unsigned long start_id = (*values_ids)[value1];
    unsigned long destination_id = (*values_ids)[value2];

    return bfs_search(id, start_id, destination_id);
}


bool poset_add(unsigned long id, char const *value1, char const *value2) {

    if (poset_exists(id) && is_valid_value(value1) && is_valid_value(value2) && value_in_poset(id, value1) && value_in_poset(id, value2)) {

        if (find_path(id, value1, value2) || find_path(id, value2, value1)) {
            return false;
        }

        Adjacency_list* adjacency_list = &posets[id].second.second;
        unsigned long value1_id = posets[id].second.first[value1];
        unsigned long value2_id = posets[id].second.first[value2];

        (*adjacency_list)[value1_id].push_back(value2_id);
        return true;
    }

    return false;
}

std::vector<unsigned long> find_nodes_with_edge_to(unsigned long id, unsigned long value_id) {

    Adjacency_list adjacency_list = posets[id].second.second;

    std::vector<unsigned long> result;

    for (size_t i = 0; i < adjacency_list.size(); i++) {

        unsigned long curr_value_id = i;
        std::vector<unsigned long>* neighbours = &adjacency_list[i];

        for (unsigned long neighbour_value_id : (*neighbours)) {
            if (neighbour_value_id == value_id) {
                result.push_back(curr_value_id);
                break;
            }
        }
    }

    return result;
}

void delete_edge_to_neighbour(unsigned long id, unsigned long value1_id, unsigned long value2_id) {

    std::vector<unsigned long>* neighbours = &posets[id].second.second[value1_id];

    //TODO: do it better, this might not work properly
    for (size_t i = 0; i < (*neighbours).size(); i++) {
        unsigned long neighbour_value_id = (*neighbours)[i];

        if (neighbour_value_id == value2_id) {
            std::swap((*neighbours)[i], (*neighbours).back());
            (*neighbours).pop_back();
            i--;
        }
    }
}

void delete_edges_from_to(unsigned long id, const std::vector<unsigned long>& from, unsigned long value_id) {

    for (unsigned long curr_value_id : from) {
        delete_edge_to_neighbour(id, curr_value_id, value_id);
    }
}

void poset_add_edge_between(unsigned long id, unsigned long value1_id, unsigned long value2_id) {

    Adjacency_list* adjacency_list = &posets[id].second.second;

    (*adjacency_list)[value1_id].push_back(value2_id);
}

void poset_add_edges(unsigned long id, const std::vector<unsigned long>& from, const std::vector<unsigned long>& to) {

    if (from.empty() || to.empty()) { // dk if necessary
        return;
    }

    for (unsigned long value1_id: from) {
        for (unsigned long value2_id : to) {
            poset_add_edge_between(id, value1_id, value2_id);
        }
    }
}

bool poset_remove(unsigned long id, char const *value) {

    if (poset_exists(id) && is_valid_value(value) && value_in_poset(id, value)) {
        //TODO: imporve this algorithm

        // for now this slow algorithm, just to make sure it works

        // 0. delete removed node from everywhere
        // 1. find nodes that have an edge to removed node and put them in a vector1
        // 2. put nodes to which the removed node has an edge in a vector2
        // 3. create new edges between all nodes in vector1 and vector2

        unsigned long value_id = posets[id].second.first[value];

        posets[id].second.first.erase(value);
        posets[id].second.second[value_id].clear();
        posets[id].first[value_id] = "";

        std::vector<unsigned long> from = find_nodes_with_edge_to(id, value_id);
        std::vector<unsigned long>* to = &posets[id].second.second[value_id];

        delete_edges_from_to(id, from, value_id);

        poset_add_edges(id, from, *to);

        return true;
    }

    return false;
}

bool can_delete_relation(unsigned long id, char const *value1, char const *value2) {

    unsigned long value1_id = posets[id].second.first[value1];
    unsigned long value2_id = posets[id].second.first[value2];

    delete_edge_to_neighbour(id, value1_id, value2_id);

    bool result = !find_path(id, value1, value2);

    poset_add_edge_between(id, value1_id, value2_id);

    return result;
}

bool poset_del(unsigned long id, char const *value1, char const *value2) {

    if (poset_exists(id) && is_valid_value(value1) && is_valid_value(value2) && value_in_poset(id, value1) && value_in_poset(id, value2) ) {

        if (find_path(id, value1, value2) && can_delete_relation(id, value1, value2)) {

            unsigned long value1_id = posets[id].second.first[value1];
            unsigned long value2_id = posets[id].second.first[value2];
            delete_edge_to_neighbour(id, value1_id, value2_id);

            std::vector<unsigned long> from = find_nodes_with_edge_to(id, value1_id);
            std::vector<unsigned long> to = {value2_id};

            poset_add_edges(id, from, to);

            return true;
        }
    }
    return false;
}

bool poset_test(unsigned long id, char const *value1, char const *value2) {

    if (poset_exists(id) && is_valid_value(value1) && is_valid_value(value2) && value_in_poset(id, value1) && value_in_poset(id, value2)) {
        return find_path(id, value1, value2);
    }
    return false;
}

int main() {
    unsigned long p1;

    p1 = poset_new();
    assert(poset_size(p1) == 0);
    assert(poset_size(p1 + 1) == 0);
    assert(!poset_insert(p1, NULL));
    assert(poset_insert(p1, "A"));
    assert(poset_test(p1, "A", "A"));
    assert(!poset_insert(p1, "A"));
    assert(!poset_insert(p1 + 1, "B"));
    assert(poset_size(p1) == 1);
    assert(!poset_remove(p1 + 1, "A"));
    assert(poset_remove(p1, "A"));
    assert(!poset_remove(p1, "A"));
    assert(poset_insert(p1, "B"));
    assert(poset_insert(p1, "C"));
    assert(poset_add(p1, "B", "C"));
    assert(!poset_remove(p1, "A"));
    assert(!poset_add(p1, NULL, "X"));
    assert(!poset_del(p1, NULL, "X"));
    assert(!poset_test(p1, NULL, "X"));
    assert(!poset_add(p1, "X", NULL));
    assert(!poset_del(p1, "X", NULL));
    assert(!poset_test(p1, "X", NULL));
    assert(!poset_add(p1, NULL, NULL));
    assert(!poset_del(p1, NULL, NULL));
    assert(!poset_test(p1, NULL, NULL));
    assert(!poset_add(p1, "C", "D"));
    assert(!poset_add(p1, "D", "C"));
    assert(!poset_del(p1, "C", "D"));
    assert(!poset_del(p1, "D", "C"));
    assert(!poset_test(p1, "C", "D"));
    assert(!poset_test(p1, "D", "C"));
    assert(!poset_add(p1 + 1, "C", "D"));
    assert(!poset_del(p1 + 1, "C", "D"));
    assert(!poset_test(p1 + 1, "C", "D"));
    poset_clear(p1);
    poset_clear(p1 + 1);
    assert(poset_insert(p1, "E"));
    assert(poset_insert(p1, "F"));
    assert(poset_insert(p1, "G"));
    assert(poset_add(p1, "E", "F"));
    assert(!poset_add(p1, "E", "F"));
    assert(!poset_add(p1, "F", "E"));
    assert(poset_test(p1, "E", "F"));
    assert(!poset_test(p1, "F", "E"));
    assert(poset_add(p1, "F", "G"));
    assert(poset_test(p1, "E", "G"));
    assert(!poset_del(p1, "E", "G"));
    assert(poset_del(p1, "E", "F"));
    assert(!poset_del(p1, "E", "F"));
    assert(!poset_del(p1, "G", "F"));
    assert(!poset_del(p1, "G", "G"));
    assert(poset_size(p1) == 3);
    poset_delete(p1);
    poset_delete(p1);
    poset_delete(p1 + 1);

    // harder cases

    p1 = poset_new();
    assert(poset_size(p1) == 0);
    assert(poset_size(p1 + 1) == 0);
    assert(!poset_insert(p1, NULL));

    // 1
    assert(poset_insert(p1, "A"));
    assert(poset_insert(p1, "B"));
    assert(poset_insert(p1, "C"));
    assert(poset_add(p1, "A", "B"));
    assert(poset_test(p1, "A", "B"));
    assert(poset_add(p1, "B", "C"));
    assert(poset_test(p1, "B", "C"));

    assert(poset_test(p1, "A", "C"));
    assert(!poset_add(p1, "A", "C"));
    assert(!poset_del(p1, "A", "C"));
    assert(poset_test(p1, "A", "C"));

    assert(poset_del(p1, "B", "C"));
    assert(poset_test(p1, "A", "C"));
    assert(!poset_del(p1, "B", "C"));
    assert(poset_test(p1, "A", "B"));
    assert(!poset_test(p1, "B", "C"));
    assert(poset_del(p1, "A", "B"));
    assert(poset_test(p1, "A", "C"));
    assert(!poset_test(p1, "A", "B"));

    assert(poset_size(p1) == 3);
    poset_clear(p1);
    assert(poset_size(p1) == 0);
    poset_delete(p1);
    poset_delete(p1);


    // 2
    p1 = poset_new();
    assert(poset_insert(p1, "C"));
    assert(poset_insert(p1, "B"));
    assert(poset_insert(p1, "A"));
    assert(poset_add(p1, "A", "B"));
    assert(poset_add(p1, "A", "C"));
    assert(poset_add(p1, "B", "C"));
    assert(poset_test(p1, "A", "C"));
    assert(poset_test(p1, "B", "C"));
    assert(poset_del(p1, "B", "C"));
    assert(poset_test(p1, "A", "C"));
    assert(poset_add(p1, "C", "B"));
    assert(poset_test(p1, "C", "B"));
    assert(poset_remove(p1, "B"));
    assert(poset_test(p1, "A", "C"));
    assert(!poset_test(p1, "C", "B"));

    assert(poset_size(p1) == 2);

    assert(!poset_remove(p1, "B"));
    assert(poset_remove(p1, "C"));
    assert(poset_size(p1) == 1);
    assert(!poset_test(p1, "A", "C"));
    assert(!poset_test(p1, "C", "A"));
    assert(poset_test(p1, "A", "A"));
    assert(!poset_test(p1, "B", "B"));
    assert(!poset_test(p1, "C", "C"));

    poset_delete(p1);






    return 0;
}