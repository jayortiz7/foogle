/*
 * Copyright ©2025 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2025 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);
static void ProcessQueries(DocTable *dt, MemIndex *mi);
static int GetNextLine(FILE *f, char **ret_str);


//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char **argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.
  DocTable *dt;
  MemIndex *mi;
  CrawlFileTree(argv[1], &dt, &mi);
  ProcessQueries(dt, mi);
  DocTable_Free(dt);
  MemIndex_Free(mi);
  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable *dt, MemIndex *mi) {
  char *line = NULL;

  while (true) {
    printf("search> ");

    if (GetNextLine(stdin, &line) == -1) {
      // EOF -- clean up and stop
      break;
    }

    // Split the line into words on whitespace, growing the array as needed.
    int query_capacity = 8;
    char **query = (char**) malloc(sizeof(char*) * query_capacity);
    Verify333(query != NULL);

    int num_words = 0;
    char *saveptr;
    char *word = strtok_r(line, " ", &saveptr);
    while (word != NULL) {
      if (num_words == query_capacity) {
        query_capacity *= 2;
        query = (char**) realloc(query, query_capacity * sizeof(char*));
        Verify333(query != NULL);
      }
      // Lowercase the word to match how the index stores words.
      for (char *c = word; *c != '\n'; c++) {
        *c = tolower(*c);
      }
      query[num_words++] = word;
      word = strtok_r(NULL, " ", &saveptr);
    }

    if (num_words > 0) {
      LinkedList *results = MemIndex_Search(mi, query, num_words);
      if (results == NULL) {
        LLIterator *it = LLIterator_Allocate(results);
        Verify333(it != NULL);
        while(LLIterator_IsValid(it)) {
          LLPayload_t payload;
          LLIterator_Get(it, &payload);
          SearchResult *sr = (SearchResult*) payload;
          char *doc_name = DocTable_GetDocName(dt, sr->doc_id);
          printf(" %s (%d)\n", doc_name, sr->rank);
          LLIterator_Next(it);
        }
        LLIterator_Free(it);
        LinkedList_Free(results, (LLPayloadFreeFnPtr) free);
      }
    }
    free(query);
    free(line);
  }
}

static int GetNextLine(FILE *f, char **ret_str) {
  size_t buf_size = 1024;
  char *buf = (char*) malloc(buf_size);
  Verify333(buf != NULL);

  if (fgets(buf, buf_size, f) == NULL) {
    // EOF or error.  Return -1 to indicate this.
    free(buf);
    return -1;
  }

  // Grow the buffer and keep reading if the line was longer than what
  // fgets grabbed in one call.
  while (strchr(buf, '\n') == NULL && !feof(f)) {
    size_t old_len = strlen(buf);
    buf_size *= 2;
    buf = (char*) realloc(buf, buf_size);
    Verify333(buf != NULL);
    if (fgets(buf + old_len, buf_size - old_len, f) == NULL) {
      break;
    }
  }

  // Strip the trailing newline, if there is one.
  size_t len = strlen(buf);
  if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
  }
  *ret_str = buf;
  return 0;
}
