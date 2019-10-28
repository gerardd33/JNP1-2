#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <cassert>
#include <sstream>
#include "poset.h"

#define fprefix __func__<<"("
#define fsuffix ")\n"<<__func__<<": "

// to uncomment
//#define NDEBUG

//TODO: Change this vector to a map or set
// A list of values present in the poset
using Values_list = std::vector<std::string>;
// A map matching the values to their ids in the poset
using Values_ids = std::unordered_map<std::string, unsigned long>; // zmienic na referencje
// An adjacency list storing the edges in the graph representation of the poset
using Adjacency_list = std::vector<std::vector<unsigned long>>;
// The bool indicates whether the poset is deleted
using Poset = std::tuple<bool, Values_list, Values_ids, Adjacency_list>;

namespace {

  std::vector<Poset>* posets() {
    static std::vector<Poset> posets;
    return &posets;
  }

  unsigned long next_poset_id = 0;

  namespace dbg {
    static std::stringstream write;

    void print() {
      #ifndef NDEBUG
      std::cerr << write.str() << "\n";
      write.str("");
      #endif
    }

    // TODO: better plan these functions, to make the write's in main functions clearer; tidy up a lot

    void poset_does_not_exist(unsigned long id) {
      write << "poset " << id << " does not exist";
    }

    void element_does_not_exist(char const* value) {
      write << "element \"" << value << "\" does not exist";
    }

    void element_already_exists(char const* value) {
      write << "element \"" << value << "\" already exists";
    }

    void invalid_value(const std::string& name) {
      write << "invalid " << name << " (NULL)";
    }

    void relation(char const* value1, char const* value2) {
      write << "relation (\"" << value1 << "\", \""<< value2 << "\")";
    }
  }

  // Getters and setters for posets
  namespace poset {

    void set_deleted(unsigned long poset_id, bool value) {
      std::get<0>((*(posets()))[poset_id]) = value;
    }

    bool is_deleted(unsigned long poset_id) {
      return std::get<0>((*(posets()))[poset_id]);
    }

    Values_list* get_values_list(unsigned long poset_id) {
      return &std::get<1>((*(posets()))[poset_id]);
    }

    Values_ids* get_values_ids(unsigned long poset_id) {
      return &std::get<2>((*(posets()))[poset_id]);
    }

    Adjacency_list* get_adjacency_list(unsigned long poset_id) {
      return &std::get<3>((*(posets()))[poset_id]);
    }
  }

  bool poset_exists(unsigned long id) {
    return id < next_poset_id && !poset::is_deleted(id);
  }

  bool is_valid_value(char const* value) {
    return value != nullptr;
  }

  bool is_value_in_poset(unsigned long id, char const* value) {

    Values_ids* values_ids = poset::get_values_ids(id);
    return values_ids->find(value) != values_ids->end();
  }

  void add_edge(unsigned long id, unsigned long from_value_id,
                unsigned long to_value_id) {

    Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
    adjacency_list->at(from_value_id).push_back(to_value_id);
  }

  // Adds all the possible edges between elements from from_list and to_list.
  void add_all_edges_between(unsigned long id,
                             const std::vector<unsigned long>& from_list,
                             const std::vector<unsigned long>& to_list) {

    for (unsigned long from_value_id : from_list) {
      for (unsigned long to_value_id : to_list) {
        add_edge(id, from_value_id, to_value_id);
      }
    }
  }

  // Removes an edge from from_value_id to to_value_id.
  void delete_edge(unsigned long id, unsigned long from_value_id,
                   unsigned long to_value_id) {

    std::vector<unsigned long>* neighbours =
            &poset::get_adjacency_list(id)->at(from_value_id);

    for (size_t i = 0; i < neighbours->size(); i++) {
      if (neighbours->at(i) == to_value_id) {
        std::swap(neighbours->at(i), neighbours->back());
        neighbours->pop_back();
        i--;
      }
    }
  }

  // Removes all edges from nodes in vector from_list to the node to_value_id.
  void delete_all_edges_from(unsigned long id,
                             const std::vector<unsigned long>& from_list,
                             unsigned long to_value_id) {

    for (unsigned long curr_value_id : from_list) {
      delete_edge(id, curr_value_id, to_value_id);
    }
  }

  bool bfs(unsigned long id, unsigned long start_value_id,
           unsigned long destination_value_id) {

    Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
    std::vector<bool> visited(adjacency_list->size(), false);
    std::queue<unsigned long> queue;
    queue.push(start_value_id);

    while (!queue.empty()) {

      unsigned long curr_value_id = queue.front();
      queue.pop();

      if (curr_value_id == destination_value_id) {
        return true;
      }

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

  // Returns a vector of all nodes from which there exists
  // an edge to the node corresponding to the given value.
  std::vector<unsigned long> find_all_with_edge_to(unsigned long id,
                                                   unsigned long value_id) {

    Adjacency_list* adjacency_list = poset::get_adjacency_list(id);
    std::vector<unsigned long> result;

    size_t adj_list_size = adjacency_list->size();
    for (size_t curr_value_id = 0; curr_value_id < adj_list_size; curr_value_id++) {
      for (unsigned long neighbour_value_id : adjacency_list->at(curr_value_id)) {
        if (neighbour_value_id == value_id) {
          result.push_back(curr_value_id);
          break;
        }
      }
    }

    return result;
  }

  bool can_delete_relation(unsigned long id, char const* value1,
                           char const* value2) {

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

  dbg::write << fprefix << fsuffix;

  Poset poset;
  posets()->push_back(poset);
  assert(posets()->size());
  poset::set_deleted(next_poset_id, false);

  dbg::write << "poset " << next_poset_id << " created";
  dbg::print();
  return next_poset_id++;
}

void poset_delete(unsigned long id) {

  dbg::write << fprefix << id << fsuffix;

  if (id >= next_poset_id || poset::is_deleted(id)) {
    dbg::poset_does_not_exist(id);
    dbg::print();
    return;
  }

  poset::get_values_list(id)->clear();
  poset::get_values_ids(id)->clear();
  poset::get_adjacency_list(id)->clear();
  poset::set_deleted(id, true);

  dbg::write << "poset " << id << " deleted";
  dbg::print();
}

size_t poset_size(unsigned long id) {

  dbg::write << fprefix << id << fsuffix;

  if (!poset_exists(id)) {
    dbg::poset_does_not_exist(id);
    dbg::print();
    return 0;
  }

  size_t result = poset::get_values_ids(id)->size();

  dbg::write << "poset " << id << " contains " << result << " element(s)";
  dbg::print();
  return result;
}

bool poset_insert(unsigned long id, char const *value) {

  dbg::write << fprefix << id << ", \"" << (value ? value : "NULL") << "\""
             << fsuffix;

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

  dbg::write << "poset " << id << ", ";

  if (is_value_in_poset(id, value)) {
    dbg::element_already_exists(value);
    dbg::print();
    return false;
  }

  std::string new_value(value);
  unsigned long new_value_id = poset::get_adjacency_list(id)->size();
  std::vector<unsigned long> empty_vec;

  poset::get_values_list(id)->push_back(new_value);
  poset::get_values_ids(id)->insert(std::make_pair(new_value, new_value_id));
  poset::get_adjacency_list(id)->push_back(empty_vec);

  dbg::write << "element \"" << value << "\" inserted";
  dbg::print();
  return true;
}

bool poset_remove(unsigned long id, char const *value) {

  dbg::write << fprefix << id << ", \"" << (value ? value : "NULL") << "\""
             << fsuffix;

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

  dbg::write << "poset " << id << ", ";

  if (!is_value_in_poset(id, value)) {
    dbg::element_does_not_exist(value);
    dbg::print();
    return false;
  }

  Adjacency_list *adjacency_list = poset::get_adjacency_list(id);
  unsigned long value_id = poset::get_values_ids(id)->at(value);

  poset::get_values_ids(id)->erase(value);
  poset::get_values_list(id)->at(value_id).clear();

  std::vector<unsigned long> edges_from_value =
          find_all_with_edge_to(id, value_id);
  std::vector<unsigned long> *edges_to_value = &adjacency_list->at(value_id);

  delete_all_edges_from(id, edges_from_value, value_id);
  add_all_edges_between(id, edges_from_value, *edges_to_value);
  adjacency_list->at(value_id).clear();

  dbg::write << "element \"" << value << "\" removed";
  dbg::print();
  return true;
}

bool poset_add(unsigned long id, char const *value1, char const *value2) {

  dbg::write << fprefix << id << ", \"" << (value1 ? value1 : "NULL") << "\""
             << ", \"" << (value2 ? value2 : "NULL") << "\"" << fsuffix;

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
      dbg::write << __func__ << ": ";
    }
    dbg::invalid_value("value2");
    dbg::print();
  }

  if (!is_valid_value(value1) || !is_valid_value(value2)) {
    return false;
  }

  dbg::write << "poset " << id << ", ";

  if (!is_value_in_poset(id, value1)) {
    dbg::write << "element \"" << value1 << "\" or \"" << value2
               << "\" does not exist";
    dbg::print();
    return false;
  }

  if (!is_value_in_poset(id, value2)) {
    dbg::write << "element \"" << value1 << "\" or \"" << value2
               << "\" does not exist";
    dbg::print();
    return false;
  }

  dbg::relation(value1, value2);
  if (find_path(id, value1, value2)) {
    dbg::write << " exists";
    dbg::print();
    return false;
  }

  if (find_path(id, value2, value1)) {
    dbg::write << " cannot be added";
    dbg::print();
    return false;
  }

  Values_ids *values_ids = poset::get_values_ids(id);
  unsigned long value1_id = values_ids->at(value1);
  unsigned long value2_id = values_ids->at(value2);
  poset::get_adjacency_list(id)->at(value1_id).push_back(value2_id);

  dbg::write << " added";
  dbg::print();
  return true;
}

bool poset_del(unsigned long id, char const *value1, char const *value2) {

  dbg::write << fprefix << id << ", \"" << (value1 ? value1 : "NULL") << "\""
             << ", \"" << (value2 ? value2 : "NULL") << "\"" << fsuffix;

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
      dbg::write << __func__ << ": ";
    }
    dbg::invalid_value("value2");
    dbg::print();
  }

  if (!is_valid_value(value1) || !is_valid_value(value2)) {
    return false;
  }

  dbg::write << "poset " << id << ", ";
  if (!is_value_in_poset(id, value1)) {
    dbg::write << "element \"" << value1 << "\" or \"" << value2
               << "\" does not exist";
    dbg::print();
    return false;
  }

  if (!is_value_in_poset(id, value2)) {
    dbg::write << "element \"" << value1 << "\" or \"" << value2
               << "\" does not exist";
    dbg::print();
    return false;
  }

  dbg::relation(value1, value2);
  if (!find_path(id, value1, value2)) {
    dbg::write << " does not exist";
    dbg::print();
    return false;
  }

  if (!can_delete_relation(id, value1, value2)) {
    dbg::write << " cannot be deleted";
    dbg::print();
    return false;
  }

  Values_ids *values_ids = poset::get_values_ids(id);
  unsigned long value1_id = values_ids->at(value1);
  unsigned long value2_id = values_ids->at(value2);
  delete_edge(id, value1_id, value2_id);

  std::vector<unsigned long> from_list1 =
          find_all_with_edge_to(id, value1_id);
  std::vector<unsigned long> to_list1 = {value2_id};

  std::vector<unsigned long> from_list2 = {value1_id};
  std::vector<unsigned long> *to_list2 =
          &poset::get_adjacency_list(id)->at(value2_id);

  add_all_edges_between(id, from_list1, to_list1);
  add_all_edges_between(id, from_list2, *to_list2);

  dbg::write << " deleted";
  dbg::print();
  return true;
}

bool poset_test(unsigned long id, char const *value1, char const *value2) {

  dbg::write << fprefix << id << ", \"" << (value1 ? value1 : "NULL") << "\""
             << ", \"" << (value2 ? value2 : "NULL") << "\"" << fsuffix;

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
      dbg::write << __func__ << ": ";
    }
    dbg::invalid_value("value2");
    dbg::print();
  }

  if (!is_valid_value(value1) || !is_valid_value(value2)) {
    return false;
  }

  dbg::write << "poset " << id << ", ";
  if (!is_value_in_poset(id, value1) || !is_value_in_poset(id, value2)) {
    dbg::write << "element \"" << value1 << "\" or \"" << value2
               << "\" does not exist";
    dbg::print();
    return false;
  }

  dbg::relation(value1, value2);
  if (find_path(id, value1, value2)) {
    dbg::write << " exists";
    dbg::print();
    return true;
  } else {
    dbg::write << " does not exist";
    dbg::print();
    return false;
  }
}

void poset_clear(unsigned long id) {

  dbg::write << fprefix << id << fsuffix;

  if (!poset_exists(id)) {
    dbg::poset_does_not_exist(id);
    dbg::print();
    return;
  }

  poset::get_values_list(id)->clear();
  poset::get_values_ids(id)->clear();
  poset::get_adjacency_list(id)->clear();

  dbg::write << "poset " << id << " cleared";
  dbg::print();
}

int main() {
  unsigned long p1;

  p1 = poset_new();
  poset_delete(p1);

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