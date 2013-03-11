/*

Copyright (C) 1993-2012 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_lex_h)
#define octave_lex_h 1

#include <deque>
#include <limits>
#include <list>
#include <set>
#include <stack>

#include "comment-list.h"
#include "input.h"
#include "token.h"

extern OCTINTERP_API void cleanup_parser (void);

// Is the given string a keyword?
extern bool is_keyword (const std::string& s);

class
stream_reader
{
public:
  virtual int getc (void) = 0;
  virtual int ungetc (int c) = 0;

protected:
  stream_reader (void) { }
  ~stream_reader (void) { }

private:

  // No copying!
  stream_reader (const stream_reader&);
  stream_reader& operator = (const stream_reader&);
};

// For communication between the lexer and parser.

class
lexical_feedback
{
public:

  // Track nesting of square brackets, curly braces, and parentheses.

  class bbp_nesting_level
  {
  private:

    enum bracket_type
      {
        BRACKET = 1,
        BRACE = 2,
        PAREN = 3,
        ANON_FCN_BODY = 4
      };

  public:

    bbp_nesting_level (void) : context () { }

    bbp_nesting_level (const bbp_nesting_level& nl) : context (nl.context) { }

    bbp_nesting_level& operator = (const bbp_nesting_level& nl)
    {
      if (&nl != this)
        context = nl.context;

      return *this;
    }

    ~bbp_nesting_level (void) { }

    void reset (void)
    {
      while (! context.empty ())
        context.pop ();
    }

    void bracket (void) { context.push (BRACKET); }

    bool is_bracket (void)
    {
      return ! context.empty () && context.top () == BRACKET;
    }

    void brace (void) { context.push (BRACE); }

    bool is_brace (void)
    {
      return ! context.empty () && context.top () == BRACE;
    }

    void paren (void) { context.push (PAREN); }

    bool is_paren (void)
    {
      return ! context.empty () && context.top () == PAREN;
    }

    void anon_fcn_body (void) { context.push (ANON_FCN_BODY); }

    bool is_anon_fcn_body (void)
    {
      return ! context.empty () && context.top () == ANON_FCN_BODY;
    }

    bool is_bracket_or_brace (void)
    {
      return (! context.empty ()
              && (context.top () == BRACKET || context.top () == BRACE));
    }

    bool none (void) { return context.empty (); }

    void remove (void)
    {
      if (! context.empty ())
        context.pop ();
    }

    void clear (void)
    {
      while (! context.empty ())
        context.pop ();
    }

  private:

    std::stack<int> context;
  };

  class token_cache
  {
  public:

    // Store an "unlimited" number of tokens.
    token_cache (size_t sz_arg = std::numeric_limits<size_t>::max ())
      : buffer (), sz (sz_arg)
    { }

    void push (token *tok)
    {
      if (buffer.size () == sz)
        pop ();

      buffer.push_front (tok);
    }

    void pop (void)
    {
      if (! empty ())
        {
          delete buffer.back ();
          buffer.pop_back ();
        }
    }

    // Direct access.
    token *at (size_t n)
    {
      return empty () ? 0 : buffer.at (n);
    }

    const token *at (size_t n) const
    {
      return empty () ? 0 : buffer.at (n);
    }

    // Most recently pushed.
    token *front (void)
    {
      return empty () ? 0 : buffer.front ();
    }

    const token *front (void) const
    {
      return empty () ? 0 : buffer.front ();
    }

    token *back (void)
    {
      return empty () ? 0 : buffer.back ();
    }

    const token *back (void) const
    {
      return empty () ? 0 : buffer.back ();
    }

    // Number of elements currently in the buffer, max of sz.
    size_t size (void) const { return buffer.size (); }

    bool empty (void) const { return buffer.empty (); }

    void clear (void)
    {
      while (! empty ())
        pop ();
    }

  private:

    std::deque<token *> buffer;

    size_t sz;

    // No copying!

    token_cache (const token_cache&);

    token_cache& operator = (const token_cache&);
  };
  
  lexical_feedback (void)
    : end_of_input (false), at_beginning_of_statement (true),
      looking_at_anon_fcn_args (false), looking_at_return_list (false),
      looking_at_parameter_list (false), looking_at_decl_list (false),
      looking_at_initializer_expression (false),
      looking_at_matrix_or_assign_lhs (false),
      looking_for_object_index (false), 
      looking_at_indirect_ref (false), parsing_class_method (false),
      maybe_classdef_get_set_method (false), parsing_classdef (false),
      quote_is_transpose (false), force_script (false),
      reading_fcn_file (false), reading_script_file (false),
      reading_classdef_file (false),
      input_line_number (1), current_input_column (1),
      bracketflag (0), braceflag (0),
      looping (0), defining_func (0), looking_at_function_handle (0),
      block_comment_nesting_level (0), token_count (0),
      current_input_line (), comment_text (), help_text (),
      fcn_file_name (), fcn_file_full_name (), looking_at_object_index (),
      parsed_function_name (), pending_local_variables (),
      nesting_level (), tokens ()
  {
    init ();
  }

  ~lexical_feedback (void);

  void init (void);

  void reset (void);

  int previous_token_value (void) const;

  bool previous_token_value_is (int tok_val) const;

  void mark_previous_token_trailing_space (void);

  bool space_follows_previous_token (void) const;

  bool previous_token_is_binop (void) const;

  bool previous_token_is_keyword (void) const;

  bool previous_token_may_be_command (void) const;

  // true means that we have encountered eof on the input stream.
  bool end_of_input;

  // true means we are at the beginning of a statement, where a
  // command name is possible.
  bool at_beginning_of_statement;

  // true means we are parsing an anonymous function argument list.
  bool looking_at_anon_fcn_args;

  // true means we're parsing the return list for a function.
  bool looking_at_return_list;

  // true means we're parsing the parameter list for a function.
  bool looking_at_parameter_list;

  // true means we're parsing a declaration list (global or
  // persistent).
  bool looking_at_decl_list;

  // true means we are looking at the initializer expression for a
  // parameter list element.
  bool looking_at_initializer_expression;

  // true means we're parsing a matrix or the left hand side of
  // multi-value assignment statement.
  bool looking_at_matrix_or_assign_lhs;

  // object index not possible until we've seen something.
  bool looking_for_object_index;

  // true means we're looking at an indirect reference to a
  // structure element.
  bool looking_at_indirect_ref;

  // true means we are parsing a class method in function or classdef file.
  bool parsing_class_method;

  // true means we are parsing a class method declaration line in a
  // classdef file and can accept a property get or set method name.
  // for example, "get.propertyname" is recognized as a function name.
  bool maybe_classdef_get_set_method;

  // true means we are parsing a classdef file
  bool parsing_classdef;

  // return transpose or start a string?
  bool quote_is_transpose;

  // TRUE means treat the current file as a script even if the first
  // token is "function" or "classdef".
  bool force_script;

  // TRUE means we're parsing a function file.
  bool reading_fcn_file;

  // TRUE means we're parsing a script file.
  bool reading_script_file;

  // TRUE means we're parsing a classdef file.
  bool reading_classdef_file;

  // the current input line number.
  int input_line_number;

  // the column of the current token.
  int current_input_column;

  // square bracket level count.
  int bracketflag;

  // curly brace level count.
  int braceflag;

  // true means we're in the middle of defining a loop.
  int looping;

  // nonzero means we're in the middle of defining a function.
  int defining_func;

  // nonzero means we are parsing a function handle.
  int looking_at_function_handle;

  // nestng level for blcok comments.
  int block_comment_nesting_level;

  // Count of tokens recognized by this lexer since initialized or
  // since the last reset.
  size_t token_count;

  // The current line of input.
  std::string current_input_line;

  // The current comment text.
  std::string comment_text;

  // The current help text.
  std::string help_text;

  // Simple name of function file we are reading.
  std::string fcn_file_name;

  // Full name of file we are reading.
  std::string fcn_file_full_name;

  // if the front of the list is true, the closest paren, brace, or
  // bracket nesting is an index for an object.
  std::list<bool> looking_at_object_index;

  // if the top of the stack is true, then we've already seen the name
  // of the current function.  should only matter if
  // current_function_level > 0
  std::stack<bool> parsed_function_name;

  // set of identifiers that might be local variable names.
  std::set<std::string> pending_local_variables;

  // is the closest nesting level a square bracket, squiggly brace,
  // a paren, or an anonymous function body?
  bbp_nesting_level nesting_level;

  // Tokens generated by the lexer.
  token_cache tokens;

private:

  // No copying!

  lexical_feedback (const lexical_feedback&);

  lexical_feedback& operator = (const lexical_feedback&);
};

// octave_lexer inherits from lexical_feedback because we will
// eventually have several different constructors and it is easier to
// intialize if everything is grouped in a parent class rather than
// listing all the members in the octave_lexer class.

class
octave_lexer : public lexical_feedback
{
public:

  // Handle buffering of input for lexer.

  class input_buffer
  {
  public:

    input_buffer (void)
      : buffer (), pos (0), chars_left (0), eof (false)
    { }

    void fill (const std::string& input, bool eof_arg);

    // Copy at most max_size characters to buf.
    int copy_chunk (char *buf, size_t max_size);

    bool empty (void) const { return chars_left == 0; }

    bool at_eof (void) const { return eof; }

  private:

    std::string buffer;
    const char *pos;
    size_t chars_left;
    bool eof;
  };

  octave_lexer (void)
    : lexical_feedback (), scanner (0), input_buf (), input_reader ()
  {
    init ();
  }

  octave_lexer (FILE *file)
    : lexical_feedback (), scanner (0), input_buf (),
      input_reader (file)
  {
    init ();
  }

  octave_lexer (const std::string& eval_string)
    : lexical_feedback (), scanner (0), input_buf (),
      input_reader (eval_string)
  {
    init ();
  }

  ~octave_lexer (void);

  void init (void);

  void reset (void);

  void prep_for_file (void);

  int read (char *buf, unsigned int max_size);

  int handle_end_of_input (void);

  char *flex_yytext (void);

  int flex_yyleng (void);

  int text_yyinput (void);

  void xunput (char c, char *buf);

  void xunput (char c);

  bool looking_at_space (void);

  bool inside_any_object_index (void);

  int is_keyword_token (const std::string& s);

  bool whitespace_is_significant (void);

  void handle_number (void);

  void handle_continuation (void);

  void finish_comment (octave_comment_elt::comment_type typ,
                       bool looking_at_continuation = false);

  bool have_continuation (bool trailing_comments_ok = true);

  bool have_ellipsis_continuation (bool trailing_comments_ok = true);

  int handle_string (char delim);

  int handle_close_bracket (int bracket_type);

  bool next_token_can_follow_bin_op (void);

  bool looks_like_command_arg (void);

  int handle_superclass_identifier (void);

  int handle_meta_identifier (void);

  int handle_identifier (void);

  void maybe_warn_separator_insert (char sep);

  void gripe_single_quote_string (void);

  void gripe_matlab_incompatible (const std::string& msg);

  void maybe_gripe_matlab_incompatible_comment (char c);

  void gripe_matlab_incompatible_continuation (void);

  void gripe_matlab_incompatible_operator (const std::string& op);

  void push_token (token *);

  token *current_token (void);

  void display_token (int tok);

  void fatal_error (const char *msg);

  void lexer_debug (const char *pattern);

  // Internal state of the flex-generated lexer.
  void *scanner;

  // Object that reads and buffers input.
  input_buffer input_buf;

  octave_input_reader input_reader;

  void increment_promptflag (void) { input_reader.increment_promptflag (); }

  void decrement_promptflag (void) { input_reader.decrement_promptflag (); }

  int promptflag (void) const { return input_reader.promptflag (); }

  int promptflag (int n) { return input_reader.promptflag (n); }

  std::string input_source (void) const
  {
    return input_reader.input_source ();
  }

  bool input_from_terminal (void) const
  {
    return input_source () == "terminal";
  }

  bool input_from_file (void) const
  {
    return input_source () == "file";
  }

  bool input_from_eval_string (void) const
  {
    return input_source () == "eval_string";
  }

  void push_start_state (int state);

  void pop_start_state (void);

  void clear_start_state (void);

  int start_state (void) const { return start_state_stack.top (); }

  void display_start_state (void) const;

  int handle_op (const char *pattern, int tok, bool bos = false);

  int handle_incompatible_op (const char *pattern, int tok, bool bos = false);

  bool maybe_unput_comma_before_unary_op (int tok);

  int handle_unary_op (int tok, bool bos = false);

  int handle_incompatible_unary_op (int tok, bool bos = false);

  int handle_assign_op (const char *pattern, int tok);

  int handle_incompatible_assign_op (const char *pattern, int tok);

  int handle_op_internal (int tok, bool bos, bool compat);

  int handle_token (const std::string& name, int tok);

  int handle_token (int tok, token *tok_val = 0);

  int count_token (int tok);

  int count_token_internal (int tok);

  int show_token (int tok);

  // For unwind protect.
  static void cleanup (octave_lexer *lexer) { delete lexer; }

private:

  std::stack<int> start_state_stack;

  // No copying!

  octave_lexer (const octave_lexer&);

  octave_lexer& operator = (const octave_lexer&);
};

#endif
