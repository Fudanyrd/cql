/*****************************************************
 * File: variable_manager.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * Can manage all variables defined by users.
 *****************************************************/
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "type.h"

namespace cql {

class VariableManager {
 private:
  /** A variable manager should track all variables by name and data. */
  std::unordered_map<std::string, std::vector<DataBox>> variables_;
 public:
  VariableManager() = default;
  // disallow copy.
  VariableManager(const VariableManager &var_mg) = delete;

  /**
   * @brief add a variable to the manager.
   * Overwrite if already exists.
   */
  void Add(const std::string &var, const std::vector<DataBox> &dat) {
    variables_[var] = dat;
  }
  
  /**
   * @brief Retrive the data from the manager.
   * Throw an domain_error if the variable not exists.
   */
  auto Retrive(const std::string &var) const -> const std::vector<DataBox> & {
    auto iter = variables_.find(var);
    if (iter == variables_.end()) {
      throw std::domain_error("retrieving a non-exist variable?? Impossible!");
    }
    return iter->second;
  }

  /**
   * @return number of variables.
   */
  auto numOfVars() const -> size_t { return variables_.size(); }
};

}  // namespace cql