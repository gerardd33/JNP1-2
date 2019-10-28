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

  // TOASK: don't know how to neatly group these getters and setters with the restriction on using classes \
  // Is it better this way or to just leave them as normal functions like those below (and call them \
  // set_poset_deleted (for set_deleted), poset::get_values_list etc?
  // Getters and setters for posets
  namespace poset {
    void set_deleted(unsigned long poset_id, bool value) {
        std::get<0>(posets[poset_id]) = value;
    }

    bool is_deleted(unsigned long poset_id) {
        return std::get<0>(posets[poset_id]);
    }

    Values_list* get_values_list(unsigned long poset_id) {
        return &std::get<1>(posets[poset_id]);
    }

    Values_ids* get_values_ids(unsigned long poset_id) {
        return &std::get<2>(posets[poset_id]);
    }

    Adjacency_list* get_adjacency_list(unsigned long poset_id) {
        return &std::get<3>(posets[poset_id]);
    }
  }

  bool poset_exists(unsigned long id) {
      return id < next_poset_id && !poset::is_deleted(id);
  }

  bool is_valid_value(char const* value) {
      return value != nullptr; // value can't be null, and that's it (?)
  }

  bool is_value_in_poset(unsigned long id, char const* value) {

      assert(poset_exists(id));

      Values_ids* values_ids = poset::get_values_ids(id);
      return values_ids->find(value) != values_ids->end();
  }

  void add_edge(unsigned long id, unsigned long from_value_id, unsigned long to_value_id) {

      Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
      adjacency_list->at(from_value_id).push_back(to_value_id);
  }

  // Adds all the possible edges between elements from from_list and to_list.
  void add_all_edges_between(unsigned long id, const std::vector<unsigned long>& from_list, const std::vector<unsigned long>& to_list) {

      for (unsigned long from_value_id : from_list) {
          for (unsigned long to_value_id : to_list) {
              add_edge(id, from_value_id, to_value_id);
          }
      }
  }

  // Removes an edge from from_value_id to to_value_id.
  void delete_edge(unsigned long id, unsigned long from_value_id, unsigned long to_value_id) {

      std::vector<unsigned long>* neighbours = &poset::get_adjacency_list(id)->at(from_value_id);

      //TODO: do it better, this might not work properly
      for (size_t i = 0; i < neighbours->size(); i++) {
          if (neighbours->at(i) == to_value_id) {
              std::swap(neighbours->at(i), neighbours->back());
              neighbours->pop_back();
              i--;
          }
      }
  }

  // Removes all edges from nodes in vector from_list to the node to_value_id.
  void delete_all_edges_from(unsigned long id, const std::vector<unsigned long>& from_list, unsigned long to_value_id) {

      for (unsigned long curr_value_id : from_list) {
          delete_edge(id, curr_value_id, to_value_id);
      }
  }

  bool bfs(unsigned long id, unsigned long start_value_id, unsigned long destination_value_id) {

      Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
      std::queue<unsigned long> queue;
      std::vector<bool> visited(adjacency_list->size(), false);
      queue.push(start_value_id);

      while (!queue.empty()) {

          unsigned long curr_value_id = queue.front();
          queue.pop();

          if (curr_value_id == destination_value_id)
              return true;

          for (unsigned long neighbour : adjacency_list->at(curr_value_id)) {
              if (!visited[neighbour]) {
                  visited[neighbour] = true;
                  queue.push(neighbour);
              }
          }
      }

      return false;
  }

  bool find_path(unsigned long id, char const* value1, char const* value2) {

      Values_ids* values_ids = poset::get_values_ids(id);
      unsigned long start_value_id = values_ids->at(value1);
      unsigned long destination_value_id = values_ids->at(value2);

      return bfs(id, start_value_id, destination_value_id);
  }

  // Returns a vector of all nodes from which there exists an edge to the node corresponding to the given value.
  std::vector<unsigned long> find_all_with_edge_to(unsigned long id, unsigned long value_id) {

      Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
      std::vector<unsigned long> result;

      for (size_t curr_value_id = 0; curr_value_id < adjacency_list->size(); curr_value_id++) {
          for (unsigned long neighbour_value_id : adjacency_list->at(curr_value_id)) {
              if (neighbour_value_id == value_id) {
                  result.push_back(curr_value_id);
                  break;
              }
          }
      }

      return result;
  }

  bool can_delete_relation(unsigned long id, char const* value1, char const* value2) {

      Values_ids* values_ids = poset::get_values_ids(id);
      unsigned long value1_id = values_ids->at(value1);
      unsigned long value2_id = values_ids->at(value2);

      delete_edge(id, value1_id, value2_id);
      bool result = !find_path(id, value1, value2);
      add_edge(id, value1_id, value2_id);

      return result;
  }
}


unsigned long poset_new() {

    Poset poset;
    posets.push_back(poset);
    poset::set_deleted(next_poset_id, false);
    return next_poset_id++;
}

void poset_delete(unsigned long id) {

    if (id >= next_poset_id || poset::is_deleted(id))
        return;

    poset_clear(id);
    poset::set_deleted(id, true);
}

size_t poset_size(unsigned long id) {

    if (!poset_exists(id))
        return 0;

    return poset::get_values_ids(id)->size();
}

bool poset_insert(unsigned long id, char const* value) {

    if (!poset_exists(id) || !is_valid_value(value) || is_value_in_poset(id, value))
        return false;

    std::string new_value(value);
    unsigned long new_value_id = poset::get_adjacency_list(id)->size();
    std::vector<unsigned long> empty_vec;

    poset::get_values_list(id)->push_back(new_value);
    poset::get_values_ids(id)->insert(std::make_pair(new_value, new_value_id));
    poset::get_adjacency_list(id)->push_back(empty_vec);

#ifndef NDEBUG
    std::cerr << new_value << " " << new_id << "\n";
#endif

    return true;
}

bool poset_remove(unsigned long id, char const* value) {

    if (!poset_exists(id) || !is_valid_value(value) || !is_value_in_poset(id, value))
        return false;

    //TODO: improve this algorithm

    // for now this slow algorithm, just to make sure it works
    // 0. delete removed node from everywhere
    // 1. find nodes that have an edge to removed node and put them in a vector1
    // 2. put nodes to which the removed node has an edge in a vector2
    // 3. create new edges between all nodes in vector1 and vector2

    Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
    unsigned long value_id = poset::get_values_ids(id)->at(value);

    poset::get_values_ids(id)->erase(value);
    poset::get_values_list(id)->at(value_id).clear();

    std::vector<unsigned long> edges_from_value = find_all_with_edge_to(id, value_id);
    std::vector<unsigned long>* edges_to_value = &adjacency_list->at(value_id);

    delete_all_edges_from(id, edges_from_value, value_id);
    add_all_edges_between(id, edges_from_value, *edges_to_value);

    adjacency_list->at(value_id).clear();
    return true;
}

bool poset_add(unsigned long id, char const* value1, char const* value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2)
        || !is_value_in_poset(id, value1) || !is_value_in_poset(id, value2))
        return false;

    if (find_path(id, value1, value2) || find_path(id, value2, value1))
        return false;

    Values_ids* values_ids = poset::get_values_ids(id);
    unsigned long value1_id = values_ids->at(value1);
    unsigned long value2_id = values_ids->at(value2);
    poset::get_adjacency_list(id)->at(value1_id).push_back(value2_id);

    return true;
}

bool poset_del(unsigned long id, char const* value1, char const* value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2) ||
        !is_value_in_poset(id, value1) || !is_value_in_poset(id, value2))
        return false;

    if (!find_path(id, value1, value2) || !can_delete_relation(id, value1, value2))
        return false;

    Values_ids* values_ids = poset::get_values_ids(id);
    unsigned long value1_id = values_ids->at(value1);
    unsigned long value2_id = values_ids->at(value2);
    delete_edge(id, value1_id, value2_id);

    std::vector<unsigned long> from_list1 = find_all_with_edge_to(id, value1_id);
    std::vector<unsigned long> to_list1 = {value2_id};

    std::vector<unsigned long> from_list2 = {value1_id};
    std::vector<unsigned long>* to_list2 = &poset::get_adjacency_list(id)->at(value2_id);

    add_all_edges_between(id, from_list1, to_list1);
    add_all_edges_between(id, from_list2, *to_list2);

    return true;
}

bool poset_test(unsigned long id, char const* value1, char const* value2) {

    if (!poset_exists(id) || !is_valid_value(value1) || !is_valid_value(value2) || !is_value_in_poset(id, value1) || !is_value_in_poset(id, value2))
        return false;
    return find_path(id, value1, value2);
}

void poset_clear(unsigned long id) {

    if (!poset_exists(id))
        return;

    poset::get_values_list(id)->clear(); // TOASK: dk if we have to additionally delete these strings by hand
    poset::get_values_ids(id)->clear();
    poset::get_adjacency_list(id)->clear();
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