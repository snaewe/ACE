// This may look like C, but it's really -*- C++ -*-

// ========================================================================
//
// = LIBRARY 
//     TAO 
//  
// = FILENAME 
//     arg_shifter
// 
// = AUTHOR 
//     Seth Widoff 
// 
// = VERSION 
//     $Id$
// ========================================================================


#if !defined(ARG_SHIFTER_H)
#define ARG_SHIFTER_H

class Arg_Shifter
// = TITLE
//    Happy ADT to shift known args to the back of the argv vector, so 
//    deeper levels of argument parsing can locate the yet unprocessed 
//    arguments at the beginning of the vector.
//
// = DESCRIPTION
//    The Arg_Shifter copies the pointers of the argv vector into a
//    temporary array. As the Arg_Shifter iterates over the temp, is
//    places known arguments in the rear of the argv and unknown
//    ones in the beginning. So, after having visited all the arguments 
//    in the temp vector, Arg_Shifter has placed all the unknown
//    arguments in their original order at the front of argv.
{
 public:

  Arg_Shifter(int& argc, char** argv, char** temp = 0);
  // Sumbit to the Arg_Shifter the vector over which to iterate, also
  // providing the temporary array if the client doesn't want the
  // arg_shifter to dynamically allocate its own. If internal dynamic
  // allocation fails, the Arg_Shifter will set all the indices to the 
  // end of the vector, forbidding iteration. Following iteration over 
  // argv, the argc value will contain the number of unconsumed arguments.
  
  ~Arg_Shifter(void);
  
  char* get_current(void) const;
  // Get the current head of the vector.

  int consume_arg (int number = 1);
  // Consume <number> argument(s) by sticking them/it on the end of
  // the vector. 

  int ignore_arg (int number = 1);
  // Place <number> arguments in the same relative order ahead of the
  // known arguemnts in the vector.

  int is_anything_left (void) const;
  // Returns the number of args left to see in the vector.

  int is_option_next (void) const;
  // Returns 1 if there's a next item in the vector and it begins with 
  // '-'.
  
  int is_parameter_next (void) const;
  // Returns 1 if there's a next item in the vector and it doesn't
  // begin with '-'.
  
  int num_ignored_args (void) const;
  // Returns the number of irrelevant args seen.
  
private:

  int& argc_;
  // The size of the argument vector.

  int total_size_;
  // The size of argv_.
  
  char** temp_;
  // The temporary array over which we traverse.
  
  char** argv_;
  // The array in which the arguments are reordered.

  int current_index_;
  // The element in <temp_> we're currently examining.

  int back_;
  // The index of <argv_> in which we'll stick the next unknown
  // argument.
  
  int front_;
  // The index of <argv_> in which we'll stick the next known
  // argument. 
};

#endif /* ARG_SHIFTER_H */
