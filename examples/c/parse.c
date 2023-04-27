#include <assert.h>
#include <bitwuzla/c/bitwuzla.h>
#include <bitwuzla/c/parser.h>
#include <stdlib.h>

int
main()
{
  // First, create a Bitwuzla options instance.
  BitwuzlaOptions* options = bitwuzla_options_new();

  // We will parse example file `smt2/quickstart.smt2`.
  // Create parser instance.
  FILE* infile = fopen("../smt2/quickstart.smt2", "r");
  assert(infile);
  BitwuzlaParser* parser =
      bitwuzla_parser_new(options, "../smt2/quickstart.smt2", infile, "smt2");

  // Now parse the input file.
  const char* err_msg = bitwuzla_parser_parse(parser, false);
  // We expect no error to occur.
  assert(err_msg == NULL);

  // Finally, delete parser instance.
  bitwuzla_parser_delete(parser);

  return 0;
}