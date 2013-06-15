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
#include <cstdio>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include "cubescript.hpp"

namespace cubescript{

namespace expression{
enum token_id
{
    ERROR = 0,
    WHITESPACE,
    CHAR,
    END_ROOT_EXPRESSION,
    START_EXPRESSION,
    END_EXPRESSION,
    START_SYMBOL,
    START_REFERENCE,
    START_END_STRING,
    START_MULTILINE_STRING,
    END_MULTILINE_STRING,
    START_COMMENT
};
static const token_id symbols[] = 
    {ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, WHITESPACE, END_ROOT_EXPRESSION, ERROR, ERROR, END_ROOT_EXPRESSION, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    WHITESPACE, CHAR, START_END_STRING, START_COMMENT, START_SYMBOL, CHAR, START_REFERENCE, CHAR, 
    START_EXPRESSION, END_EXPRESSION, CHAR, CHAR, CHAR, CHAR, CHAR, START_COMMENT, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, END_ROOT_EXPRESSION, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, START_MULTILINE_STRING, CHAR, END_MULTILINE_STRING, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, 
    CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, CHAR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, 
    ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR};
} //namespace expression

static void throw_unexpected(unsigned char c, const char * where)
{
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    
    const char * invalid_character = buf;
    
    // Non-printable characters
    std::string hex_representation;
    if(c < 40 || c > 176)
    {
        std::stringstream format;
        format<<"\\x"<<std::hex<<static_cast<int>(c);
        hex_representation = format.str();
        invalid_character = hex_representation.c_str();
    }
    
    switch(c)
    {
        case '\0': invalid_character = "\\0"; break;
        case '\a': invalid_character = "\\a"; break;
        case '\b': invalid_character = "\\b"; break;
        case '\t': invalid_character = "\\t"; break;
        case '\n': invalid_character = "\\n"; break;
        case '\f': invalid_character = "\\f"; break;
        case '\r': invalid_character = "\\r"; break;
        case '\"': invalid_character = "\\\""; break;
        default:;
    }
    
    std::stringstream format;
    format<<"unexpected '"<<invalid_character<<"' in "<<where;
    throw parse_error(format.str());
}

enum word_type{
    WORD_INTEGER = 0,
    WORD_REAL    = 1,
    WORD_STRING  = 2
};

enum parse_number_state
{
    NAN = 0,
    START_NUMBER,
    INTEGER,
    REAL,
    START_EXPONENT,
    EXPONENT_DIGITS
};

void eval_word(const char ** source_begin, 
               const char * source_end, command_stack & command)
{
    const char * start = *source_begin;
    
    word_type type = WORD_INTEGER;
    parse_number_state number_parsing_stage = START_NUMBER;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        char c = *cursor;
        expression::token_id token_id = 
            expression::symbols[static_cast<std::size_t>(c)];
        
        if(token_id != expression::CHAR && c != '/' && c != '#' && c != '&')
        {
            *source_begin = cursor - 1;
            std::size_t length = cursor - start;
            
            if(token_id != expression::ERROR && cursor != start) 
            {
                if(number_parsing_stage == NAN) type = WORD_STRING;
                
                switch(type)
                {
                    case WORD_INTEGER:
                    {
                        float n;
                        std::sscanf(start, "%f", &n);
                        command.push_argument(static_cast<int>(n));
                        break;
                    }
                    case WORD_REAL:
                    {
                        float n;
                        std::sscanf(start, "%f", &n);
                        command.push_argument(n);
                        break;
                    }
                    case WORD_STRING:
                        command.push_argument(start, length);
                        break;
                }
                
                return;
            }
            else
            {
                *source_begin = cursor;
                throw_unexpected(c, "word");
            }
        }
        
        switch(number_parsing_stage)
        {
            case START_NUMBER:
                number_parsing_stage = ((c >= '0' && c <= '9') || c == '-' ? INTEGER : NAN);
                break;
            case INTEGER:
                if(c >= '0' && c <= '9') break;
                if(c == '.')
                {
                    number_parsing_stage = REAL;
                    type = WORD_REAL;
                }
                else if(c == 'e' || c == 'E')
                    number_parsing_stage = START_EXPONENT;
                else number_parsing_stage = NAN;
                break;
            case REAL:
                if(c >= '0' && c <= '9') break;
                number_parsing_stage = (c == 'e' || c == 'E' ? START_EXPONENT : NAN);
                break;
            case START_EXPONENT:
                number_parsing_stage = 
                    ((c >='0' && c<='9') || c== '-' || c=='+' ? EXPONENT_DIGITS : NAN);
                break;
            case EXPONENT_DIGITS:
                if(!(c >= '0' && c <= '9')) 
                    number_parsing_stage = NAN;
                break;
            default:;
        }
        
    }
    
    *source_begin = source_end;
    throw parse_incomplete();
}

static std::string decode_string(const char ** begin, const char * end, 
                              const std::vector<const char *> & escape_sequence)
{
    const char * start = *begin;
    
    std::string result;
    result.reserve(end - start);
    
    result.append(start, escape_sequence[0]);
    
    typedef std::vector<const char *>::const_iterator iterator;
    for(iterator iter = escape_sequence.begin(); iter != escape_sequence.end();
        iter++)
    {
        assert(*iter != end);
        
        const char * escape = *iter;
        
        switch(*(escape + 1))
        {
            case '\"': result.append(1, '\"'); break; 
            case '\\': result.append(1, '\\'); break; 
            case 'n':  result.append(1, '\n'); break; 
            case 'r':  result.append(1, '\r'); break; 
            case 't':  result.append(1, '\t'); break; 
            case 'f':  result.append(1, '\f'); break; 
            case 'b':  result.append(1, '\b'); break;
        }
        
        const char * sub_end = end;
        if(iter + 1 != escape_sequence.end())
            sub_end = *(iter + 1);
        
        result.append(escape + 2, sub_end);
    }
    
    return result;
}

void eval_string(const char ** source_begin, 
                 const char * source_end, command_stack & command)
{
    assert(**source_begin == '"');
    const char * start = (*source_begin) + 1;
    *source_begin = start;
    
    std::vector<const char *> escape_sequence;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        char c = *cursor;
        switch(c)
        {
            case '"':
                if(escape_sequence.size())
                {
                    std::string string = decode_string(source_begin, cursor,
                                                       escape_sequence);
                    command.push_argument(string.c_str(), string.length());
                }
                else
                {
                    std::size_t length = cursor - start;
                    command.push_argument(start, length);
                }
                
                *source_begin = cursor;
                return;
            case '\\':
            case '^':
                escape_sequence.push_back(cursor);
                cursor++;
                break;
            case '\r':
            case '\n':
                *source_begin = cursor;
               throw parse_error("unfinished string");
            default:break;
        }
    }
    
    *source_begin = source_end;
    throw parse_incomplete();
}

static
void eval_interpolation_symbol(const char ** source_begin, 
                               const char * source_end, command_stack & command)
{
    const char * start = *source_begin;
    const char * cursor = start;
    
    for(; cursor != source_end; cursor++)
    {
        expression::token_id token_id = 
            expression::symbols[static_cast<std::size_t>(*cursor)];
        
        if(token_id != expression::CHAR)
        {
            if(token_id != expression::ERROR) break;
            else
            {
                *source_begin = cursor;
                throw_unexpected(*cursor, "interpolation symbol");
            }
        }
    }
    
    std::size_t length = cursor - start;
    command.push_argument_symbol(start, length);
    
    *source_begin = cursor - 1;
}

static 
void eval_interpolation_string(const char ** begin, const char * end,
                               const std::vector<const char *> & interpolations, 
                               command_stack & command)
{
    const char * start = *begin;
    
    std::size_t cmd_index = command.push_command();
    command.push_argument_symbol("@", 1);
    
    std::size_t length = interpolations[0] - start;
    if(length) command.push_argument(start, length);
    
    typedef std::vector<const char *>::const_iterator iterator;
    for(iterator iter = interpolations.begin(); iter != interpolations.end();
        iter++)
    {
        const char * cursor = *iter;
        
        for(; *cursor == '@' && cursor != end; cursor++);
        
        if(*cursor == '(') eval_expression(&cursor, end, command, true);
        else eval_interpolation_symbol(&cursor, end, command);
        
        if(cursor + 1 < end)
        {
            const char * sub_end = end;
            if(iter + 1 != interpolations.end())
                sub_end = *(iter + 1);
            
            cursor++;
            length = sub_end - cursor;
            if(length) command.push_argument(cursor, length);
        }
    }
    
    command.call(cmd_index);
}

void eval_multiline_string(const char ** source_begin,
                           const char * source_end,
                           command_stack & command)
{
    assert(**source_begin == '[');
    const char * start = (*source_begin) + 1;
    *source_begin = start;
    
    std::vector<const char *> interpolations;
    int nested = 1;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        char c = *cursor;
        
        switch(c)
        {
            case '[':
                nested++;
                break;
            case ']':
                if(--nested == 0)
                {
                    if(interpolations.size())
                    {
                        eval_interpolation_string(
                            source_begin, 
                            cursor,
                            interpolations,
                            command);
                    }
                    else 
                    {
                        std::size_t length = cursor - start;
                        command.push_argument(start, length);
                    }
                    
                    *source_begin = cursor;
                    return;
                }
                break;
            case '@':
            {
                const char * first_at = cursor;
                for(; *cursor == '@' && cursor != source_end; cursor++);
                if(cursor - first_at >= nested)
                    interpolations.push_back(first_at);
                cursor--;
                break;
            }
            default:;
        }
    }
    
    *source_begin = source_end;
    throw parse_incomplete();
}

void eval_symbol(const char ** source_begin, 
                 const char * source_end,
                 command_stack & command)
{
    const char * start = *source_begin;
    if(*start == '$') *source_begin = ++start;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        expression::token_id token_id = 
            expression::symbols[static_cast<std::size_t>(*cursor)];
        
        if(token_id != expression::CHAR)
        {
            *source_begin = cursor - 1;
            if(token_id != expression::ERROR)
            {
                std::size_t length = cursor - start;
                
                std::size_t call_index = command.push_command();
                command.push_argument_symbol("$", 1);
                command.push_argument_symbol(start, length);
                command.call(call_index);
                
                return;
            }
            else
            {
                *source_begin = cursor;
                throw_unexpected(*cursor, "symbol");
            }
        }
    }
    
    *source_begin = source_end;
    
    throw parse_incomplete();
}

void eval_reference(const char ** source_begin, const char * source_end, command_stack & command)
{
    const char * start = *source_begin;
    if(*start == '&') *source_begin = ++start;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        expression::token_id token_id = 
            expression::symbols[static_cast<std::size_t>(*cursor)];
        
        if(token_id != expression::CHAR)
        {
            *source_begin = cursor - 1;
            if(token_id != expression::ERROR)
            {
                std::size_t length = cursor - start;
                command.push_argument_symbol(start, length);
                return;
            }
            else
            {
                *source_begin = cursor;
                throw_unexpected(*cursor, "reference");
            }
        }
    }
    
    *source_begin = source_end;
    
    throw parse_incomplete();
}

void eval_comment(const char ** source_begin, 
                  const char * source_end,
                  command_stack & command)
{
    assert(**source_begin == '/' || **source_begin == '#');
    const char * start = (*source_begin) + 1;
    *source_begin = start;
    
    start++; // Ignore second slash character
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        if(*cursor == '\n' || *cursor == '\r')
        {
            *source_begin = cursor - 1;
            return;
        }
    }
    
    *source_begin = source_end;
    
    throw parse_incomplete();
}

void eval_expression(const char ** source_begin, 
                     const char * source_end,
                     command_stack & command,
                     bool is_sub_expression)
{
    const char * start = *source_begin;
    
    if(is_sub_expression)
    {
        assert(*start == '(');
        start++;
    }
    
    std::size_t call_index = command.push_command();
    
    bool first_argument = true;
    
    for(const char * cursor = start; cursor != source_end; cursor++)
    {
        char c = *cursor;
        expression::token_id token_id = 
            expression::symbols[static_cast<std::size_t>(c)];
        
        switch(token_id)
        {
            case expression::WHITESPACE:
                // Do nothing
                break;
            case expression::END_EXPRESSION:
            {
                *source_begin = cursor;
                if(!is_sub_expression) throw_unexpected(')', "expression");
                command.call(call_index);
                return;
            }
            case expression::END_ROOT_EXPRESSION:
            {
                if(is_sub_expression)
                {
                    if(c == ';') throw_unexpected(';', "expression");
                    break; // Allow new lines in sub expressions
                }
                
                *source_begin = cursor + 1;
                command.call(call_index);
                return;
            }
            case expression::START_EXPRESSION:
            {
                eval_expression(&cursor, source_end, command, true);
                first_argument = false;
                break;
            }
            case expression::START_SYMBOL:
            {
                eval_symbol(&cursor, source_end, command);
                first_argument = false;
                break;
            }
            case expression::START_REFERENCE:
            {
                eval_reference(&cursor, source_end, command);
                first_argument = false;
                break;
            }
            case expression::START_END_STRING:
            {
                eval_string(&cursor, source_end, command);
                first_argument = false;
                break;
            }
            case expression::START_MULTILINE_STRING:
            {
                eval_multiline_string(&cursor, source_end, command);
                first_argument = false;
                break;
            }
            case expression::CHAR:
            {
                if(!first_argument) eval_word(&cursor, source_end, command);
                else eval_reference(&cursor, source_end, command);
                first_argument = false;
                break;
            }
            case expression::START_COMMENT:
            {
                eval_comment(&cursor, source_end, command);
                break;
            }
            case expression::ERROR:
            default:
                *source_begin = cursor;
                throw_unexpected(c, "expression");
        }
    }
    
    throw parse_incomplete();
}

void eval(const char ** source_begin, 
          const char * source_end, 
          command_stack & stack)
{
    while(*source_begin < source_end)
        eval_expression(source_begin, source_end, stack);
}

eval_error::eval_error(const std::string & what)
 :std::runtime_error(what)
{

}

parse_error::parse_error(const std::string & what)
 :eval_error(what)
{
    
}

parse_incomplete::parse_incomplete()
 :parse_error("unterminated syntax")
{
    
}

command_error::command_error(const std::string & what)
 :eval_error(what)
{
    
}

bool is_complete_code(const char * start, const char * end)
{
    class null_command_stack:public command_stack
    {
    public:
        virtual std::size_t push_command(){return 0;}
        virtual void push_argument_symbol(const char *, std::size_t){}
        virtual void push_argument(){}
        virtual void push_argument(bool){}
        virtual void push_argument(int){}
        virtual void push_argument(float){}
        virtual void push_argument(const char *, std::size_t){}
        virtual std::string pop_string(){return "";}
        virtual void call(std::size_t){}
    };
    
    try
    {
        null_command_stack null_command;
        eval(&start, end, null_command);
    }
    catch(parse_incomplete)
    {
        return false;
    }
    catch(eval_error)
    {
        // Let other types of errors be caught by the real evaluator
    }
    return true;
}

} //namespace cubescript

