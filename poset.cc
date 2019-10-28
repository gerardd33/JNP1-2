#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <cassert>
#include "poset.h"

#define NDEBUG

using Values_list = std::vector<std::string>;
// A list of values present in the poset.
using Values_ids = std::unordered_map<std::string, unsigned long>;
// A map matching the values to their ids in the poset
using Adjacency_list = std::vector<std::vector<unsigned long>>;
// An adjacency list storing the edges in the graph representation of the poset
using Poset = std::tuple<bool, Values_list, Values_ids, Adjacency_list>;
// The bool indicates whether the poset is deleted.

namespace {
  // TOASK: not sure where these should be
  unsigned long next_poset_id = 0;
  std::vector<Poset> posets;

  // TOASK: don't know how to neatly group these getters and setters with the restriction on using classes
  // Getters and setters for posets
  namespace {
    void set_poset_deleted(unsigned long poset_id, bool value) {
        std::get<0>(posets[poset_id]) = value;
    }

    bool is_poset_deleted(unsigned long poset_id) {
        return std::get<0>(posets[poset_id]);
    }

    Values_list* get_poset_values_list(unsigned long poset_id) {
        return &std::get<1>(posets[poset_id]);
    }

    Values_ids* get_poset_values_ids(unsigned long poset_id) {
        return &std::get<2>(posets[poset_id]);
    }

    Adjacency_list* get_poset_adjacency_list(unsigned long poset_id) {
        return &std::get<3>(posets[poset_id]);
    }
  }

  bool poset_exists(unsigned long id) {
      return id < next_poset_id && !is_poset_deleted(id);
  }

  bool value_in_poset(unsigned long id, char const *value) {

      assert(poset_exists(id));

      Values_ids* values_ids = get_poset_values_ids(id);
      return values_ids->find(value) != values_ids->end();
  }

  bool is_valid_value(char const *value) {
      return value != nullptr; // value can't be null, and that's it (?)
  }

  bool bfs_search(unsigned long id, unsigned long start_id, unsigned long destination_id) {

      Adjacency_list* adjacency_list = get_poset_adjacency_list(id);
      std::queue<unsigned long> queue;
      std::vector<bool> visited(adjacency_list->size(), false);

      queue.push(start_id);

      while (!queue.empty()) {
          unsigned long curr_id = queue.front();
          queue.pop();

          if (curr_id == destination_id)
              return true;

          std::vector<unsigned long>* neighbours = &adjacency_list->at(curr_id);
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

      Values_ids* values_ids = get_poset_values_ids(id);

      unsigned long start_id = values_ids->at(value1);
      unsigned long destination_id = values_ids->at(value2);

      return bfs_search(id, start_id, destination_id);
  }

  std::vector<unsigned long> find_nodes_with_edge_to(unsigned long id, unsigned long value_id) {

      Adjacency_list* adjacency_list = get_poset_adjacency_list(id);
      std::vector<unsigned long> result;

      for (size_t i = 0; i < adjacency_list->size(); i++) {

          unsigned long curr_value_id = i;
          std::vector<unsigned long>* neighbours = &adjacency_list->at(i);

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

      std::vector<unsigned long>* neighbours = &get_poset_adjacency_list(id)->at(value1_id);

      //TODO: do it better, this might not work properly
      for (size_t i = 0; i < neighbours->size(); i++) {
          unsigned long neighbour_value_id = neighbours->at(i);

          if (neighbour_value_id == value2_id) {
              std::swap(neighbours->at(i), neighbours->back());
              neighbours->pop_back();
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

      Adjacency_list* adjacency_list = get_poset_adjacency_list(id);
      adjacency_list->at(value1_id).push_back(value2_id);
  }

  void poset_add_edges(unsigned long id, const std::vector<unsigned long>& from, const std::vector<unsigned long>& to) {

      for (unsigned long value1_id : from) {
          for (unsigned long value2_id : to) {
              poset_add_edge_between(id, value1_id, value2_id);
          }
      }
  }

  bool can_delete_relation(unsigned long id, char const *value1, char const *value2) {

      Values_ids* values_ids = get_poset_values_ids(id);
      unsigned long value1_id = values_ids->at(value1);
      unsigned long value2_id = values_ids->at(value2);

      delete_edge_to_neighbour(id, value1_id, value2_id);

      bool result = !find_path(id, value1, value2);

      poset_add_edge_between(id, value1_id, value2_id);

      return result;
  }
}


unsigned long poset_new() {

    Poset poset;
    set_poset_deleted(next_poset_id, false);
    posets.push_back(poset);
    return next_poset_id++;
}

void poset_delete(unsigned long id) {

    if (id >= next_poset_id || is_poset_deleted(id))
        return;

    poset_clear(id);
    set_poset_deleted(id, true);
}

size_t poset_size(unsigned long id) {

    if (!poset_exists(id))
        return 0;

    return get_poset_values_ids(id)->size();
}

bool poset_insert(unsigned long id, char const *value) {

    if (!poset_exists(id) || !is_valid_value(value) || value_in_poset(id, value))
        return false;

    std::string new_value(value);
    unsigned long new_value_id = get_poset_adjacency_list(id)->size();
    std::vector<unsigned long> empty_vec;

    get_poset_values_list(id)->push_back(new_value);
    get_poset_values_ids(id)->insert(std::make_pair(new_value, new_value_id));
    get_poset_adjacency_list(id)->push_back(empty_vec);

#ifndef NDEBUG
    std::cerr << new_value << " " << new_id << "\n";
#endif

    return true;
}

bool poset_remove(unsigned long id, char const *value) {

    if (!poset_exists(id) || !is_valid_value(value) || !value_in_poset(id, value))
        return false;

    //TODO: improve this algorithm

    // for now this slow algorithm, just to make sure it works
    // 0. delete removed node from everywhere
    // 1. find nodes that have an edge to removed node and put them in a vector1
    // 2. put nodes to which the removed node has an edge in a vector2
    // 3. create new edges between all nodes in vector1 and vector2

    Adjacency_list* adjacency_list = get_poset_adjacency_list(id);
    unsigned long value_id = get_poset_values_ids(id)->at(value);

    get_poset_values_ids(id)->erase(value);
    get_poset_values_list(id)->at(value_id).clear();

    std::vector<unsigned long> from = find_nodes_with_edge_to(id, value_id);
    std::vector<unsigned long>* to = &adjacency_list->at(value_id);

    delete_edges_from_to(id, from, value_id);
    poset_add_edges(id, from, *to);

    adjacency_list->at(value_id).clear();
    return true;
}

bool poset_add(unsigned long id, char const *value1, char const *value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2)
        || !value_in_poset(id, value1) || !value_in_poset(id, value2))
        return false;

    if (find_path(id, value1, value2) || find_path(id, value2, value1))
        return false;

    Adjacency_list* adjacency_list = get_poset_adjacency_list(id);
    Values_ids* values_ids = get_poset_values_ids(id);

    unsigned long value1_id = values_ids->at(value1);
    unsigned long value2_id = values_ids->at(value2);
    adjacency_list->at(value1_id).push_back(value2_id);

    return true;
}

bool poset_del(unsigned long id, char const *value1, char const *value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2) ||
        !value_in_poset(id, value1) || !value_in_poset(id, value2))
        return false;

    if (!find_path(id, value1, value2) || !can_delete_relation(id, value1, value2))
        return false;

    Values_ids* values_ids = get_poset_values_ids(id);
    unsigned long value1_id = values_ids->at(value1);
    unsigned long value2_id = values_ids->at(value2);
    delete_edge_to_neighbour(id, value1_id, value2_id);

    std::vector<unsigned long> from1 = find_nodes_with_edge_to(id, value1_id);
    std::vector<unsigned long> to1 = {value2_id};

    std::vector<unsigned long> from2 = {value1_id};
    std::vector<unsigned long>* to2 = &get_poset_adjacency_list(id)->at(value2_id);

    poset_add_edges(id, from1, to1);
    poset_add_edges(id, from2, *to2);

    return true;
}

bool poset_test(unsigned long id, char const *value1, char const *value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2) || !value_in_poset(id, value1) || !value_in_poset(id, value2))
        return false;
    return find_path(id, value1, value2);
}

void poset_clear(unsigned long id) {

    if (!poset_exists(id))
        return;

    get_poset_values_list(id)->clear(); // dk if we have to additionally delete these strings by hand
    get_poset_values_ids(id)->clear();
    get_poset_adjacency_list(id)->clear();
}


int main() {
    unsigned long p1;

    p1 = poset_new();
    assert(poset_size(p1) == 0);
    assert(poset_size(p1 + 1) == 0);
    assert(!poset_insert(p1, nullptr));
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
    assert(!poset_add(p1, nullptr, "X"));
    assert(!poset_del(p1, nullptr, "X"));
    assert(!poset_test(p1, nullptr, "X"));
    assert(!poset_add(p1, "X", nullptr));
    assert(!poset_del(p1, "X", nullptr));
    assert(!poset_test(p1, "X", nullptr));
    assert(!poset_add(p1, nullptr, nullptr));
    assert(!poset_del(p1, nullptr, nullptr));
    assert(!poset_test(p1, nullptr, nullptr));
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
    assert(!poset_insert(p1, nullptr));

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
    assert(poset_test(p1, "A", "C"));
    assert(!poset_test(p1, "C", "A"));
    assert(!poset_test(p1, "B", "A"));
    assert(!poset_test(p1, "C", "B"));
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


    p1 = poset_new();

    assert(poset_insert(p1, "A"));
    assert(poset_insert(p1, "B"));
    assert(poset_insert(p1, "C"));
    assert(poset_insert(p1, "D"));
    assert(poset_add(p1, "A", "B"));
    assert(poset_add(p1, "B", "C"));
    assert(poset_add(p1, "A", "D"));
    assert(poset_add(p1, "C", "D"));
    assert(poset_test(p1, "A", "D"));
    assert(!poset_del(p1, "D", "A"));
    assert(!poset_del(p1, "A", "D"));
    assert(poset_del(p1, "B", "C"));
    assert(poset_test(p1, "B", "D"));
    assert(poset_test(p1, "A", "D"));
    assert(poset_add(p1, "C", "B"));
    assert(poset_test(p1, "A", "B"));
    assert(poset_test(p1, "C", "B"));
    assert(!poset_del(p1, "A", "B"));

    return 0;
}