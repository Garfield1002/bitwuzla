/***
 * Bitwuzla: Satisfiability Modulo Theories (SMT) solver.
 *
 * Copyright (C) 2023 by the authors listed in the AUTHORS file at
 * https://github.com/bitwuzla/bitwuzla/blob/main/AUTHORS
 *
 * This file is part of Bitwuzla under the MIT license. See COPYING for more
 * information at https://github.com/bitwuzla/bitwuzla/blob/main/COPYING
 */

#include <assert.h>
#include <bitwuzla/cpp/bitwuzla.h>
#include <bitwuzla/cpp/parser.h>

#include <iostream>

using namespace bitwuzla;

int
main()
{
  // First, create a Bitwuzla options instance.
  Options options;

  // We will parse example file `smt2/quickstart.smt2`.
  // Create parser instance.
  parser::Parser parser(options);

  // Now parse the input file.
  bool res = parser.parse("../smt2/quickstart.smt2");
  // We expect no error to occur.
  assert(res);

  // Now we retrieve the set of asserted formulas and print them.
  auto assertions = parser.bitwuzla()->get_assertions();
  std::cout << "Assertions:" << std::endl << "{" << std::endl;
  for (const auto& a : assertions)
  {
    std::cout << "  " << a << std::endl;
  }
  std::cout << "}" << std::endl;

  // Now we add an assertion via parsing from string.
  res = parser.parse("(assert (distinct (select a x) y))", true, false);
  // We expect no error to occur.
  assert(res);
  // Now the formula is unsat.
  Result result = parser.bitwuzla()->check_sat();

  std::cout << "Expect: unsat" << std::endl;
  std::cout << "Bitwuzla: "
            << (result == Result::SAT
                    ? "sat"
                    : (result == Result::UNSAT ? "unsat" : "unknown"))
            << std::endl
            << std::endl;
  return 0;
}
