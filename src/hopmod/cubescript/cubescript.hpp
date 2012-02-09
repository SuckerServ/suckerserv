/*
  Copyright (c) 2010 Graham Daws <graham.daws@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#ifndef CUBESCRIPT_HPP
#define CUBESCRIPT_HPP

#include <cstddef>
#include <string>
#include <stdexcept>

namespace cubescript{

/**
    A space to make function calls. The command_stack class is an abstract base
    class; where the function and argument values are stored and how they are
    represented is left up to the implementation of a derived class. The
    cubescript::eval function uses the command_stack class to construct and
    invoke a function call without caring what host language or environment
    it's working on.
    
    Any of the methods in this class may throw an eval_error exception.
*/
class command_stack
{
public:
    /**
        Start a new function call. The next argument to be pushed on the stack
        is the function, the following values pushed are the function
        arguments.
        
        @return Index value to be used as the argument for the call method.
    */
    virtual std::size_t push_command()=0;
    
    /**
        Push the value of a variable to the top of the stack.
        
        @param id The variable name
        @param id_length The string length of the variable name argument 
    */
    virtual void push_argument_symbol(const char * id, std::size_t id_length)=0;
    
    /**
        Push a null value at the top of the stack
    */
    virtual void push_argument()=0;
    
    /**
        Push a boolean value at the top of the stack
    */
    virtual void push_argument(bool)=0;
    
    /**
        Push a integer value at the top of the stack
    */
    virtual void push_argument(int)=0;
    
    /**
        Push a real value at the top of the stack
    */
    virtual void push_argument(float)=0;
    
    /**
        Push a string value at the top of the stack
    */
    virtual void push_argument(const char *, std::size_t)=0;
    
    /**
        Pop the value from the stop of the stack and return it as a string 
        value.
    */
    virtual std::string pop_string()=0;
    
    /**
        Call function. The values from the top of the stack down to the function
        are used as arguments. By the end of the function call, the function
        and arguments have been removed, and the return values pushed onto the 
        stack, starting at position index.
        
        @param index The location of the function on the stack.
    */
    virtual void call(std::size_t index)=0;
};

void eval_word(const char **, const char*, command_stack &);
void eval_string(const char **, const char*, command_stack &);
void eval_multiline_string(const char **, const char *, command_stack &);
void eval_symbol(const char **, const char *, command_stack &);
void eval_reference(const char **, const char *, command_stack &);
void eval_comment(const char **, const char *, command_stack &);
void eval_expression(const char **, const char *, command_stack &, 
                           bool is_sub_expression = false);

/**
    Parse the input string as Cubescript code to determine if the input string
    contains the code for a complete expression starting from the beginning of 
    the string. In most cases, calling this function and having it return true 
    is a prerequisite for calling eval(). If there's no further input to add
    to the current input string (i.e. end of file) then call eval() and let the
    parse error be thrown.
*/
bool is_complete_code(const char * start, const char * end);

/**
    Evaluate the input string as Cubescript code. Each expression in the code
    is turned into a function call and applied to the command_stack object. 
    Function calls can admit return values to the stack which are left up to the
    caller, of this function, to be picked off the stack.
    
    Parse errors and errors in the command stack operations can throw exceptions
    derived from the eval_error class.
*/
void eval(const char **, const char *, command_stack &);

class eval_error:public std::runtime_error
{
public:
    eval_error(const std::string &);    
};

class parse_error:public eval_error
{
public:
    parse_error(const std::string &);
};

class parse_incomplete:public parse_error
{
public:
    parse_incomplete();
};

class command_error:public eval_error
{
public:
    command_error(const std::string &);
};

} //namespace cubescript

#endif

