#include "afl.h"

void test_log(void *payload, TSLogType type, const char *string) {
  puts(string);
}

TSLogger logger = {
  .log = test_log
};

int parse_file_contents(TSLanguage* language, const char* file_contents) {
  printf("language version: %d\n", ts_language_version(language));
  printf("%s", file_contents);
  printf("\n----\n");

  TSDocument *document = ts_document_new();
  ts_document_set_language(document, language);
  ts_document_set_input_string(document, file_contents);

  // Debugging graphs are written on stderror, use dot to produce svgs for viewing.
  // Example:
  //   alf-ruby test.rb 2> info.dot
  //   cat info.dot | dot -Tsvg > info.html
  //
  // NOTES:
  //  - For infinite loops you'll have to Ctrl-C and then edit the end of
  //    info.dot to remove any unfinished parts of the graph.
  //  - Add svg {width: 100%;} to the info.hml.
  // ts_document_print_debugging_graphs(document, true);

  // Sometimes just debug logging is helpful too.
  // ts_document_set_logger(document, logger);

  ts_document_parse(document);

  TSNode root_node = ts_document_root_node(document);

  printf("Syntax tree: %s\n", ts_node_string(root_node, document));
  ts_document_free(document);
  return 0;
}

int parse_file(TSLanguage* language, const char* filename) {
  FILE *fp;
  struct stat filestatus;
  int file_size;
  char* file_contents;

  if ( stat(filename, &filestatus) != 0) {
    fprintf(stderr, "File %s not found\n", filename);
    return 1;
  }

  file_size = filestatus.st_size;
  file_contents = (char*)malloc(file_size + 1);
  if ( file_contents == NULL) {
    fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
    return 1;
  }

  fp = fopen(filename, "rt");
  if (fp == NULL) {
    fprintf(stderr, "Unable to open %s\n", filename);
    fclose(fp);
    free(file_contents);
    return 1;
  }
  if ( fread(file_contents, file_size, 1, fp) != 1 ) {
    fprintf(stderr, "Unable to read content of %s\n", filename);
    fclose(fp);
    free(file_contents);
    return 1;
  }
  // Make this a NULL terminated string.
  file_contents[file_size] = 0;
  fclose(fp);

  if ( parse_file_contents(language, file_contents) != 0) {
    fprintf(stderr, "Unable to parse %s\n", filename);
    free(file_contents);
    return 1;
  }

  free(file_contents);
  return 0;
}

int parse(int argc, char const *argv[], TSLanguage* language) {
  if (argc == 2) {
    return parse_file(language, argv[1]);
  }
  else {
    fprintf(stderr, "ERROR. Must pass one file path to parse.\nusage: afl FILE\n\n       afl-fuzz -i afl_in -o afl_out ./afl @@\n");
    return 1;
  }
}
