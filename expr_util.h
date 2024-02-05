/*****************************************************
 * File: expr_util.h
 * Author: Fudanyrd (email: yangrundong7@gmail.com)
 *
 * For converting a series of words into expression
 * Tree. This is useful.
 *****************************************************/
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "expr.h"

namespace cql {

/**
 * @return the post-order expression using stack.
 * For debugging, you generally don't have to call this.
 */
auto toPostOrder(const std::vector<std::string> &words) -> std::vector<AbstractExprRef>;

/**
 * @brief convert a series of words(done by partitioner) into a
 * Abstract Expression Tree.
 */
auto toExprRef(const std::vector<std::string> &words) -> AbstractExprRef;

}  // namespace cql