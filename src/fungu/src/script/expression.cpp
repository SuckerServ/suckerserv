/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

static inline bool is_expr_terminator(const_string::value_type c)
{
    switch(c)
    {
        case ';': case ')': case '\r': case '\n': case '\0': return true;
        default: return false;
    }
}

static inline bool is_numeric(const const_string & str)
{
    const_string::const_iterator i=str.begin();
    if(i<str.end()-1 && (*i=='-' || *i=='+')) ++i;
    for(; i!=str.end(); ++i)
    {
        if(!(*i>='0' && *i<='9')) return false;
    }
    return true;
}

bool expression::word_exit_terminals::is_member(const_string::value_type c)
{
    switch(c)
    {
        case '\"': case '\'': case ')': case '[': case ']': case ' ':
        case '\r': case '\n': case '\t': case ';': case '{' :case '}': 
        case '\0':
            return true;
        default: return false;
    }
}

expression::expression()
 :m_parsing(NULL),
  m_first_construct(NULL),
  m_operation_symbol(NULL),
  m_line(0),
  m_source_ctx(NULL)
{
    
}

expression::~expression()
{
    delete m_parsing;
    delete m_first_construct;
    delete m_source_ctx;
}
    
parse_state expression::parse(source_iterator * first, source_iterator last, env_frame * frame)
{
    if(!m_source_ctx)
    {
        m_source_ctx = frame->get_env()->get_source_context() ? frame->get_env()->get_source_context()->clone() : NULL;
        m_line = m_source_ctx->get_line_number();
    }
    
    if(m_parsing)
    {
        try
        {
            source_iterator start_pos = *first;
            
            parse_state substate = m_parsing->parse(first, last, frame);
            if( substate != PARSE_COMPLETE ) return substate;
            
            if( start_pos == *first && !is_expr_terminator(*start_pos)) 
                throw error(UNEXPECTED_SYMBOL, boost::make_tuple(**first));
            
            if(m_parsing->is_eval_supported()) add_child_construct(m_parsing);
            else delete m_parsing;
            
            m_parsing = NULL;
        }
        catch(const error & key)
        {
            throw new error_trace(
                key, const_string(), frame->get_env()->get_source_context()->clone());
        }
    }
    
    if(*first > last) return PARSE_PARSING;
    
    if(is_expr_terminator(**first))
    {
        ++(*first);
        
        if(!is_empty_expression())
        {
            if(is_alias_assignment(frame)) translate_alias_assignment(frame);
            
            fill_constarg_vector(frame);
            
            if(m_first_construct->is_string_constant())
                m_operation_symbol = frame->get_env()->lookup_symbol(m_first_construct->eval(frame).to_string());
        }
        
        return PARSE_COMPLETE;
    }
    
    switch(**first)
    {
        case '\"': m_parsing = new quote;                ++(*first); break;
        case '[':  m_parsing = new block;                ++(*first); break;
        case '(':  m_parsing = new subexpression;        ++(*first); break;
        case '/':
        case '#':  m_parsing = new comment;              ++(*first); break;
        case ' ':
        case '\t':                                       ++(*first); break;
        case '$':  m_parsing = new symbol<word_exit_terminals>;          ++(*first); break;
        case '&':  m_parsing = new reference<word_exit_terminals>;       ++(*first); break;
        default:   m_parsing = new word<word_exit_terminals>;                        break;
    }
    
    if(*first > last) return PARSE_PARSING;
    
    return parse(first,last,frame);
}

any expression::eval(env_frame * frame)
{
    env * environment = frame->get_env();
    env_object * operation_object = NULL;
    const construct * subject_arg = m_first_construct;
    
    const source_context * prev_ctx = environment->get_source_context();
    environment->set_source_context(m_source_ctx);
    
    // This block of code is run on function exit, it's being used because of the several exit points this method has.
    expression * _this = this;
    BOOST_SCOPE_EXIT((&_this)(&environment)(&prev_ctx))
    {
        _this->reset_placeholders();
        environment->set_source_context(prev_ctx);
    } BOOST_SCOPE_EXIT_END

    try
    {
        // Resolve the operation object
        
        any arg1_eval_result = m_first_construct->eval(frame);
        
        if(m_operation_symbol) operation_object = m_operation_symbol->lookup_object(frame);
        else
        {
            m_operation_symbol = frame->get_env()->lookup_symbol(arg1_eval_result.to_string());
            if(m_operation_symbol) operation_object = m_operation_symbol->lookup_object(frame);
            else
            {
                if(arg1_eval_result.get_type() == typeid(env_object::shared_ptr))
                    operation_object = any_cast<env_object::shared_ptr>(arg1_eval_result).get();
            }
        }
        
        if(!operation_object)
        {
            const_string number = arg1_eval_result.to_string();
            if(is_numeric(number)) return number;
            else throw error(UNKNOWN_SYMBOL, boost::make_tuple(number.copy()));
        }
        
        // Evaluate arguments before calling the operation object
        
        std::vector<any> variable_args(m_placeholders.size());
        std::vector<any>::iterator vargIter = variable_args.begin();
        
        // The reason we need to store argument eval results to a local vector
        // is because of the case where an argument evaluation leads 
        // re-evaluation of this expression, if we wrote straight to
        // m_arguments the eval results for the previous arguments would be
        // overwritten.
        
        for(construct * current = m_first_construct->get_next_sibling();
            current; current = current->get_next_sibling(), vargIter++ )
        {
            subject_arg = current;
            *vargIter = current->eval(frame);
        }
        
        std::vector<unsigned char>::const_iterator phIter = m_placeholders.begin();
        for(vargIter = variable_args.begin(); vargIter != variable_args.end() ; ++vargIter, ++phIter)
        {
            m_arguments[*phIter] = *vargIter;
        }
        variable_args.clear();
        
        subject_arg = m_first_construct;
        
        callargs args(m_arguments);
        
        return operation_object -> call(args, frame);
    }
    catch(const error & key)
    {
        throw new error_trace(key, subject_arg->form_source(), get_source_context());
    }
    catch(error_trace * head_info)
    {
        throw new error_trace(head_info, subject_arg->form_source(), get_source_context());
    }
}
    
bool expression::is_string_constant()const
{
    return false;
}

/**
    @brief Check for empty empression.
    
    Returns true if the expression contains no arguments (including
    the operation argument).
    
    @internal The method is used in alias evaluation, for ingoring the 
              evaluation result of empty expressions when setting the
              implicit alias result.
*/
bool expression::is_empty_expression()const
{
    return !m_first_construct;
}

void expression::add_child_construct(construct * child)
{
    if(!m_first_construct) m_first_construct = child;
    else m_first_construct->get_tail_sibling()->set_next_sibling(child);
}
    
bool expression::is_alias_assignment(env_frame * frame)const
{
    construct * arg1 = m_first_construct;
    if(!arg1) return false;
    construct * op = arg1->get_next_sibling();
    if(!op || !op->is_string_constant()) return false;
    construct * arg2 = op->get_next_sibling();
    if(!arg2 || arg2->get_next_sibling()) return false;
    return op->eval(frame).to_string() == const_string(FUNGU_LITERAL_STRING("="));
}

void expression::translate_alias_assignment(env_frame * frame)
{
    construct * arg1 = m_first_construct;
    construct * op = arg1->get_next_sibling();
    construct * arg2 = op->get_next_sibling();
    
    m_first_construct = new word<word_exit_terminals>(const_string(FUNGU_LITERAL_STRING("alias")));
    m_first_construct->set_next_sibling(arg1);
    arg1->set_next_sibling(arg2);
    
    op->set_next_sibling(NULL);
    delete op;
}
    
void expression::fill_constarg_vector(env_frame * frame)
{
    assert(m_first_construct);
    
    construct * last_arg = m_first_construct;
    construct * arg = m_first_construct->get_next_sibling();
    
    while(arg)
    {
        if(arg->is_string_constant())
        {
            any argval = arg->eval(frame);
            
            if(argval.get_type() == typeid(const_string))
            {
                const_string argstrval = any_cast<const_string>(argval);
                if(argstrval.length() && is_numeric(argstrval)) argval = lexical_cast<int>(argstrval);
            }
            
            m_arguments.push_back(argval);
            
            construct * next = arg->get_next_sibling();
            
            arg->set_next_sibling(NULL);
            delete arg;
            
            last_arg->set_next_sibling(next);
            arg = next;
        }
        else
        {
            m_arguments.push_back(any());
            m_placeholders.push_back(m_arguments.size()-1);
            
            last_arg = arg;
            arg = arg->get_next_sibling();
        }
    }
}

source_context * expression::get_source_context()const
{
    if(!m_source_ctx) return NULL;
    source_context * ctx = m_source_ctx->clone();
    ctx->set_line_number(m_line);
    return ctx;
}

void expression::reset_placeholders()
{
    for(std::vector<unsigned char>::const_iterator it = m_placeholders.begin(); 
        it != m_placeholders.end(); it++)
    {
        m_arguments[*it] = any();
    }
}

std::string expression::form_source()const
{
    std::string source;
    
    construct * operation = m_first_construct;
    source += operation->form_source();
    
    construct * current = operation->get_next_sibling();
    
    for(unsigned int i = 0, j = 0; i < m_arguments.size(); i++)
    {
        source += " ";
        if(j < m_placeholders.size() && m_placeholders[j] == i)
        {
            source += current->form_source();
            current = current->get_next_sibling();
            j++;
        }
        else source += m_arguments[i].to_string().copy();
    }
    
    return source;
}

parse_state base_expression::parse(source_iterator * first, source_iterator last, env_frame * frame)
{
    parse_state state = expression::parse(first,last,frame);
    
    if(state == PARSE_COMPLETE && *((*first)-1) == ')')
    {
        throw new error_trace(
            error(UNEXPECTED_SYMBOL, boost::make_tuple(')')),
            const_string(),
            get_source_context() );
    }
    
    return state;
}

std::string base_expression::form_source()const
{
    std::string source;
    source = expression::form_source();
    source += "\n";
    return source;
}

} //namespace script
} //namespace fungu
