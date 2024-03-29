#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <sstream>
#include "poset.h"

#ifndef NDEBUG
#include <iostream>
#endif

#define dbg_prefix __func__ << "("
#define dbg_suffix ")\n" << __func__ << ": "
#define dbg_value(value) "\"" << (value ? value : "NULL") << "\""
#define dbg_relation(value1, value2) "relation (\"" << value1 << "\", \""<< value2 << "\")"

// A map storing the values and matching them to their ids in the poset
using Values_map = std::unordered_map<std::string, unsigned long>;
// A graph representation of the poset, each node in the map has its neighbours in the set
using Neighbours = std::unordered_set<unsigned long>;
using Adjacency_list = std::unordered_map<unsigned long, Neighbours>;

using Poset = std::tuple<Values_map, Adjacency_list>;
using Posets = std::unordered_map<unsigned long, Poset>;


namespace {

  Posets& posets() {
    static Posets posets;
    return posets;
  }

  namespace dbg {

    std::stringstream& write() {
      static std::stringstream write;
      return write;
    }

    void print() {
#ifndef NDEBUG
      std::cerr << write().str() << "\n";
      write().str("");
#endif
    }

    void poset_does_not_exist(unsigned long id) {
      write() << "poset " << id << " does not exist";
    }

    void invalid_value(const std::string& name) {
      write() << "invalid " << name << " (NULL)";
    }
  }

  // Getters for posets
  namespace poset {

    Values_map* get_Values_map(unsigned long poset_id) {
      return &std::get<0>(posets()[poset_id]);
    }

    Adjacency_list* get_Adjacency_list(unsigned long poset_id) {
      return &std::get<1>(posets()[poset_id]);
    }

  }

  unsigned long get_next_value_id() {
    static unsigned long next_value_id = 0;
    return next_value_id++;
  }

  bool poset_exists(unsigned long id) {
    return posets().find(id) != posets().end();
  }

  bool is_valid_value(char const* value) {
    return value != nullptr;
  }

  bool is_value_in_poset(unsigned long id, char const* value) {

    Values_map* Values_map = poset::get_Values_map(id);
    return Values_map->find(value) != Values_map->end();
  }

  void add_edge(unsigned long id, unsigned long from_value_id,
                unsigned long to_value_id) {

    Adjacency_list* adjacency_list = poset::get_Adjacency_list(id);

    Neighbours* neighbours = &adjacency_list->at(from_value_id);

    neighbours->insert(to_value_id);
  }

  // Adds all the possible edges between elements from from_set and to_set.
  void add_all_edges_between(unsigned long id,
                             const std::unordered_set<unsigned long>& from_set,
                             const std::unordered_set<unsigned long>& to_set) {

    for (unsigned long from_value_id : from_set) {
      for (unsigned long to_value_id : to_set) {
        add_edge(id, from_value_id, to_value_id);
      }
    }
  }

  // Removes an edge from from_value_id to to_value_id.
  void delete_edge(unsigned long id, unsigned long from_value_id,
                   unsigned long to_value_id) {

    Neighbours* neighbours =
            &poset::get_Adjacency_list(id)->at(from_value_id);

    neighbours->erase(to_value_id);
  }

  // Removes all edges from nodes in set from_set to the node to_value_id.
  void delete_all_edges_from(unsigned long id,
                             const std::unordered_set<unsigned long>& from_set,
                             unsigned long to_value_id) {

    for (unsigned long curr_value_id : from_set) {
      delete_edge(id, curr_value_id, to_value_id);
    }
  }

  bool bfs(unsigned long id, unsigned long start_value_id,
           unsigned long destination_value_id) {

    Adjacency_list* adjacency_list = poset::get_Adjacency_list(id);
    std::unordered_set<unsigned long> visited;
    std::queue<unsigned long> queue;
    visited.insert(start_value_id);
    queue.push(start_value_id);

    while (!queue.empty()) {

      unsigned long curr_value_id = queue.front();
      queue.pop();

      if (curr_value_id == destination_value_id) {
        return true;
      }

      for (unsigned long neighbour : adjacency_list->at(curr_value_id)) {
        if (visited.find(neighbour) == visited.end()) {
          visited.insert(neighbour);
          queue.push(neighbour);
        }
      }
    }

    return false;
  }

  bool find_path(unsigned long id, char const* value1, char const* value2) {

    Values_map* Values_map = poset::get_Values_map(id);
    unsigned long start_value_id = Values_map->at(value1);
    unsigned long destination_value_id = Values_map->at(value2);

    return bfs(id, start_value_id, destination_value_id);
  }

  // Returns a set of all nodes from which there exists
  // an edge to the node corresponding to the given value.
  std::unordered_set<unsigned long> find_all_with_edge_to(unsigned long id,
                                                   unsigned long value_id) {

    Adjacency_list* adjacency_list = poset::get_Adjacency_list(id);
    std::unordered_set<unsigned long> result;

    for (Adjacency_list::iterator it = adjacency_list->begin(); it != adjacency_list->end(); it++) {
      unsigned long curr_value_id = it->first;
      for (unsigned long neighbour_value_id : adjacency_list->at(curr_value_id)) {
        if (neighbour_value_id == value_id) {
          result.insert(curr_value_id);
          break;
        }
      }
    }

    return result;
  }

  bool can_delete_relation(unsigned long id, char const* value1,
                           char const* value2) {

    Values_map* Values_map = poset::get_Values_map(id);
    unsigned long value1_id = Values_map->at(value1);
    unsigned long value2_id = Values_map->at(value2);

    if (value1_id == value2_id) {
      return false;
    }

    delete_edge(id, value1_id, value2_id);
    bool result = !find_path(id, value1, value2);
    add_edge(id, value1_id, value2_id);
    return result;
  }
}

namespace jnp1 {

  unsigned long poset_new() {

    static unsigned long next_poset_id = 0;

    dbg::write() << dbg_prefix << dbg_suffix;

    Poset poset;
    posets()[next_poset_id] = poset;

    dbg::write() << "poset " << next_poset_id << " created";
    dbg::print();
    return next_poset_id++;
  }

  void poset_delete(unsigned long id) {

    dbg::write() << dbg_prefix << id << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return;
    }

    poset::get_Values_map(id)->clear();
    poset::get_Adjacency_list(id)->clear();
    posets().erase(id);

    dbg::write() << "poset " << id << " deleted";
    dbg::print();
  }

  size_t poset_size(unsigned long id) {

    dbg::write() << dbg_prefix << id << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return 0;
    }

    size_t result = poset::get_Values_map(id)->size();

    dbg::write() << "poset " << id << " contains " << result << " element(s)";
    dbg::print();
    return result;
  }

  bool poset_insert(unsigned long id, char const *value) {

    dbg::write() << dbg_prefix << id << ", " << dbg_value(value) << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return false;
    }

    if (!is_valid_value(value)) {
      dbg::invalid_value("value");
      dbg::print();
      return false;
    }

    dbg::write() << "poset " << id << ", ";

    if (is_value_in_poset(id, value)) {
      dbg::write() << "element " << dbg_value(value) << " already exists";
      dbg::print();
      return false;
    }

    std::string new_value(value);
    unsigned long new_value_id = get_next_value_id();
    std::unordered_set<unsigned long> empty_set;

    poset::get_Values_map(id)->insert(std::make_pair(new_value, new_value_id));
    poset::get_Adjacency_list(id)->insert(std::make_pair(new_value_id, empty_set));

    dbg::write() << "element " << dbg_value(value) << " inserted";
    dbg::print();
    return true;
  }

  bool poset_remove(unsigned long id, char const *value) {

    dbg::write() << dbg_prefix << id << ", " << dbg_value(value) << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return false;
    }

    if (!is_valid_value(value)) {
      dbg::invalid_value("value");
      dbg::print();
      return false;
    }

    dbg::write() << "poset " << id << ", ";

    if (!is_value_in_poset(id, value)) {
      dbg::write() << "element " << dbg_value(value) << " does not exist";
      dbg::print();
      return false;
    }

    Adjacency_list *adjacency_list = poset::get_Adjacency_list(id);
    unsigned long value_id = poset::get_Values_map(id)->at(value);

    poset::get_Values_map(id)->erase(value);

    std::unordered_set<unsigned long> edges_from_value =
        find_all_with_edge_to(id, value_id);
    std::unordered_set<unsigned long> *edges_to_value =
        &adjacency_list->at(value_id);

    delete_all_edges_from(id, edges_from_value, value_id);
    add_all_edges_between(id, edges_from_value, *edges_to_value);
    adjacency_list->erase(value_id);

    dbg::write() << "element " << dbg_value(value) << " removed";
    dbg::print();
    return true;
  }

  bool poset_add(unsigned long id, char const *value1, char const *value2) {

    dbg::write() << dbg_prefix << id << ", " << dbg_value(value1) << ", "
                 << dbg_value(value2) << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return false;
    }

    if (!is_valid_value(value1)) {
      dbg::invalid_value("value1");
      dbg::print();
    }

    if (!is_valid_value(value2)) {
      if (!is_valid_value(value1)) {
        dbg::write() << __func__ << ": ";
      }
      dbg::invalid_value("value2");
      dbg::print();
    }

    if (!is_valid_value(value1) || !is_valid_value(value2)) {
      return false;
    }

    dbg::write() << "poset " << id << ", ";

    if (!is_value_in_poset(id, value1) || !is_value_in_poset(id, value2)) {
      dbg::write() << "element " << dbg_value(value1) << " or "
                   << dbg_value(value2) << " does not exist";
      dbg::print();
      return false;
    }

    dbg::write() << dbg_relation(value1, value2);
    if (find_path(id, value1, value2) || find_path(id, value2, value1)) {
      dbg::write() << " cannot be added";
      dbg::print();
      return false;
    }

    Values_map *Values_map = poset::get_Values_map(id);
    unsigned long value1_id = Values_map->at(value1);
    unsigned long value2_id = Values_map->at(value2);
    poset::get_Adjacency_list(id)->at(value1_id).insert(value2_id);

    dbg::write() << " added";
    dbg::print();
    return true;
  }

  bool poset_del(unsigned long id, char const *value1, char const *value2) {

    dbg::write() << dbg_prefix << id << ", " << dbg_value(value1) << ", "
                 << dbg_value(value2) << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return false;
    }

    if (!is_valid_value(value1)) {
      dbg::invalid_value("value1");
      dbg::print();
    }

    if (!is_valid_value(value2)) {
      if (!is_valid_value(value1)) {
        dbg::write() << __func__ << ": ";
      }
      dbg::invalid_value("value2");
      dbg::print();
    }

    if (!is_valid_value(value1) || !is_valid_value(value2)) {
      return false;
    }

    dbg::write() << "poset " << id << ", ";
    if (!is_value_in_poset(id, value1) || !is_value_in_poset(id, value2)) {
      dbg::write() << "element " << dbg_value(value1) << " or "
                   << dbg_value(value2) << " does not exist";
      dbg::print();
      return false;
    }

    dbg::write() << dbg_relation(value1, value2);
    if (!find_path(id, value1, value2)) {
      dbg::write() << " cannot be deleted";
      dbg::print();
      return false;
    }

    if (!can_delete_relation(id, value1, value2)) {
      dbg::write() << " cannot be deleted";
      dbg::print();
      return false;
    }

    Values_map *Values_map = poset::get_Values_map(id);
    unsigned long value1_id = Values_map->at(value1);
    unsigned long value2_id = Values_map->at(value2);
    delete_edge(id, value1_id, value2_id);

    std::unordered_set<unsigned long> from_set1 =
        find_all_with_edge_to(id, value1_id);
    std::unordered_set<unsigned long> to_set1 = {value2_id};

    std::unordered_set<unsigned long> from_set2 = {value1_id};
    std::unordered_set<unsigned long> *to_set2 =
        &poset::get_Adjacency_list(id)->at(value2_id);

    add_all_edges_between(id, from_set1, to_set1);
    add_all_edges_between(id, from_set2, *to_set2);

    dbg::write() << " deleted";
    dbg::print();
    return true;
  }

  bool poset_test(unsigned long id, char const *value1, char const *value2) {

    dbg::write() << dbg_prefix << id << ", " << dbg_value(value1) << ", "
                 << dbg_value(value2) << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return false;
    }

    if (!is_valid_value(value1)) {
      dbg::invalid_value("value1");
      dbg::print();
    }

    if (!is_valid_value(value2)) {
      if (!is_valid_value(value1)) {
        dbg::write() << __func__ << ": ";
      }
      dbg::invalid_value("value2");
      dbg::print();
    }

    if (!is_valid_value(value1) || !is_valid_value(value2)) {
      return false;
    }

    dbg::write() << "poset " << id << ", ";
    if (!is_value_in_poset(id, value1) || !is_value_in_poset(id, value2)) {
      dbg::write() << "element " << dbg_value(value1) << " or "
                   << dbg_value(value2) << " does not exist";
      dbg::print();
      return false;
    }

    dbg::write() << dbg_relation(value1, value2);
    if (find_path(id, value1, value2)) {
      dbg::write() << " exists";
      dbg::print();
      return true;
    } else {
      dbg::write() << " does not exist";
      dbg::print();
      return false;
    }
  }

  void poset_clear(unsigned long id) {

    dbg::write() << dbg_prefix << id << dbg_suffix;

    if (!poset_exists(id)) {
      dbg::poset_does_not_exist(id);
      dbg::print();
      return;
    }

    poset::get_Values_map(id)->clear();
    poset::get_Adjacency_list(id)->clear();

    dbg::write() << "poset " << id << " cleared";
    dbg::print();
  }
}
