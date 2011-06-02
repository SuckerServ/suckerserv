/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include <boost/numeric/conversion/cast.hpp>

namespace fungu{
namespace script{

typedef void (*cast_function)(void * source, const type_id & source_type, void * destination, const type_id & destination_type);

static cast_function base_cast_operations[type_id::base_id_count][type_id::base_id_count];
static cast_function numeric_cast_operations[type_id::sub_id_count][type_id::sub_id_count];

template<typename SourceType, typename DestType>
static void numeric_cast(void * source, const type_id & source_type, void * destination, const type_id & destination_type)
{
    *reinterpret_cast<DestType *>(destination) = boost::numeric_cast<DestType, SourceType>(*reinterpret_cast<SourceType *>(source));
}

static void dispatch_numeric_cast(void * source, const type_id & source_type, void * destination, const type_id & destination_type)
{
    numeric_cast_operations[source_type.sub()][destination_type.sub()](source, source_type, destination, destination_type);
}

static void to_nil(void*, const type_id &, void *, const type_id&)
{
    //no operation
}

static void from_nil(void *, const type_id&, void * destination, const type_id & destination_type)
{
    int nil_value = 0;
    cast(&nil_value, type_id::get(type_tag<int>()), destination, destination_type);
}

static void no_cast(void *, const type_id &, void *, const type_id &)
{
    throw std::bad_cast();
}

void initialize_base_cast_operations()
{
    for(unsigned int i = 0; i < type_id::base_id_count; i++)
        for(unsigned int j = 0; j < type_id::base_id_count; j++)
            base_cast_operations[i][j] = no_cast;
    
    for(unsigned int i = 0; i < type_id::sub_id_count; i++)
        for(unsigned int j = 0; j < type_id::sub_id_count; j++)
            numeric_cast_operations[i][j] = no_cast;
    
    // numeric cast
    base_cast_operations[type_id::NUMBER]  [type_id::NUMBER]  = dispatch_numeric_cast;
    base_cast_operations[type_id::BOOLEAN] [type_id::NUMBER]  = dispatch_numeric_cast;
    base_cast_operations[type_id::NUMBER]  [type_id::BOOLEAN] = dispatch_numeric_cast;
    base_cast_operations[type_id::NIL]     [type_id::NUMBER]  = dispatch_numeric_cast;
    base_cast_operations[type_id::NUMBER]  [type_id::NIL]     = dispatch_numeric_cast;
    
    /*
        types = {
            {"SIGNED_CHAR",         "signed char"},
            {"SIGNED_INT",          "signed int"},
            {"SIGNED_SHORT_INT",    "signed short int"},
            {"SIGNED_LONG_INT",     "signed long int"},
            {"UNSIGNED_CHAR",       "unsigned char"},
            {"UNSIGNED_INT",        "unsigned int"},
            {"UNSIGNED_SHORT_INT",  "unsigned short int"},
            {"UNSIGNED_LONG_INT",   "unsigned long int"},
            {"FLOAT",               "float"},
            {"DOUBLE",              "double"},
            {"BOOL",                "bool"},
            {"NIL_INT",             "int"}
        }

        for _, source_type_id in ipairs(types) do
            for _, dest_type_id in ipairs(types) do
                
                local lvalue = string.format("numeric_cast_operations[type_id::%s][type_id::%s]", source_type_id[1], dest_type_id[1])
                local rvalue = ""
                
                if source_type_id[1] == "NIL_INT" or dest_type_id[1] == "NIL_INT" then
                    if source_type_id[1] == "NIL_INT" then
                        rvalue = "from_nil"
                    else
                        rvalue = "to_nil"
                    end
                else
                    rvalue = string.format("numeric_cast<%s, %s>", source_type_id[2], dest_type_id[2])
                end
                
                print(lvalue .. " = " .. rvalue .. ";")
            end
        end
    */
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::SIGNED_CHAR] = numeric_cast<signed char, signed char>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::SIGNED_INT] = numeric_cast<signed char, signed int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::SIGNED_SHORT_INT] = numeric_cast<signed char, signed short int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::SIGNED_LONG_INT] = numeric_cast<signed char, signed long int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::UNSIGNED_CHAR] = numeric_cast<signed char, unsigned char>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::UNSIGNED_INT] = numeric_cast<signed char, unsigned int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::UNSIGNED_SHORT_INT] = numeric_cast<signed char, unsigned short int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::UNSIGNED_LONG_INT] = numeric_cast<signed char, unsigned long int>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::FLOAT] = numeric_cast<signed char, float>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::DOUBLE] = numeric_cast<signed char, double>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::BOOL] = numeric_cast<signed char, bool>;
    numeric_cast_operations[type_id::SIGNED_CHAR][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::SIGNED_CHAR] = numeric_cast<signed int, signed char>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::SIGNED_INT] = numeric_cast<signed int, signed int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<signed int, signed short int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::SIGNED_LONG_INT] = numeric_cast<signed int, signed long int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::UNSIGNED_CHAR] = numeric_cast<signed int, unsigned char>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::UNSIGNED_INT] = numeric_cast<signed int, unsigned int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<signed int, unsigned short int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<signed int, unsigned long int>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::FLOAT] = numeric_cast<signed int, float>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::DOUBLE] = numeric_cast<signed int, double>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::BOOL] = numeric_cast<signed int, bool>;
    numeric_cast_operations[type_id::SIGNED_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::SIGNED_CHAR] = numeric_cast<signed short int, signed char>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::SIGNED_INT] = numeric_cast<signed short int, signed int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<signed short int, signed short int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::SIGNED_LONG_INT] = numeric_cast<signed short int, signed long int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::UNSIGNED_CHAR] = numeric_cast<signed short int, unsigned char>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::UNSIGNED_INT] = numeric_cast<signed short int, unsigned int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<signed short int, unsigned short int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<signed short int, unsigned long int>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::FLOAT] = numeric_cast<signed short int, float>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::DOUBLE] = numeric_cast<signed short int, double>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::BOOL] = numeric_cast<signed short int, bool>;
    numeric_cast_operations[type_id::SIGNED_SHORT_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::SIGNED_CHAR] = numeric_cast<signed long int, signed char>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::SIGNED_INT] = numeric_cast<signed long int, signed int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<signed long int, signed short int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::SIGNED_LONG_INT] = numeric_cast<signed long int, signed long int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::UNSIGNED_CHAR] = numeric_cast<signed long int, unsigned char>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::UNSIGNED_INT] = numeric_cast<signed long int, unsigned int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<signed long int, unsigned short int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<signed long int, unsigned long int>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::FLOAT] = numeric_cast<signed long int, float>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::DOUBLE] = numeric_cast<signed long int, double>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::BOOL] = numeric_cast<signed long int, bool>;
    numeric_cast_operations[type_id::SIGNED_LONG_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::SIGNED_CHAR] = numeric_cast<unsigned char, signed char>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::SIGNED_INT] = numeric_cast<unsigned char, signed int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::SIGNED_SHORT_INT] = numeric_cast<unsigned char, signed short int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::SIGNED_LONG_INT] = numeric_cast<unsigned char, signed long int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::UNSIGNED_CHAR] = numeric_cast<unsigned char, unsigned char>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::UNSIGNED_INT] = numeric_cast<unsigned char, unsigned int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::UNSIGNED_SHORT_INT] = numeric_cast<unsigned char, unsigned short int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::UNSIGNED_LONG_INT] = numeric_cast<unsigned char, unsigned long int>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::FLOAT] = numeric_cast<unsigned char, float>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::DOUBLE] = numeric_cast<unsigned char, double>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::BOOL] = numeric_cast<unsigned char, bool>;
    numeric_cast_operations[type_id::UNSIGNED_CHAR][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::SIGNED_CHAR] = numeric_cast<unsigned int, signed char>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::SIGNED_INT] = numeric_cast<unsigned int, signed int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<unsigned int, signed short int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::SIGNED_LONG_INT] = numeric_cast<unsigned int, signed long int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::UNSIGNED_CHAR] = numeric_cast<unsigned int, unsigned char>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::UNSIGNED_INT] = numeric_cast<unsigned int, unsigned int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<unsigned int, unsigned short int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<unsigned int, unsigned long int>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::FLOAT] = numeric_cast<unsigned int, float>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::DOUBLE] = numeric_cast<unsigned int, double>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::BOOL] = numeric_cast<unsigned int, bool>;
    numeric_cast_operations[type_id::UNSIGNED_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::SIGNED_CHAR] = numeric_cast<unsigned short int, signed char>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::SIGNED_INT] = numeric_cast<unsigned short int, signed int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<unsigned short int, signed short int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::SIGNED_LONG_INT] = numeric_cast<unsigned short int, signed long int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::UNSIGNED_CHAR] = numeric_cast<unsigned short int, unsigned char>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::UNSIGNED_INT] = numeric_cast<unsigned short int, unsigned int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<unsigned short int, unsigned short int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<unsigned short int, unsigned long int>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::FLOAT] = numeric_cast<unsigned short int, float>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::DOUBLE] = numeric_cast<unsigned short int, double>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::BOOL] = numeric_cast<unsigned short int, bool>;
    numeric_cast_operations[type_id::UNSIGNED_SHORT_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::SIGNED_CHAR] = numeric_cast<unsigned long int, signed char>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::SIGNED_INT] = numeric_cast<unsigned long int, signed int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::SIGNED_SHORT_INT] = numeric_cast<unsigned long int, signed short int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::SIGNED_LONG_INT] = numeric_cast<unsigned long int, signed long int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::UNSIGNED_CHAR] = numeric_cast<unsigned long int, unsigned char>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::UNSIGNED_INT] = numeric_cast<unsigned long int, unsigned int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<unsigned long int, unsigned short int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::UNSIGNED_LONG_INT] = numeric_cast<unsigned long int, unsigned long int>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::FLOAT] = numeric_cast<unsigned long int, float>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::DOUBLE] = numeric_cast<unsigned long int, double>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::BOOL] = numeric_cast<unsigned long int, bool>;
    numeric_cast_operations[type_id::UNSIGNED_LONG_INT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::FLOAT][type_id::SIGNED_CHAR] = numeric_cast<float, signed char>;
    numeric_cast_operations[type_id::FLOAT][type_id::SIGNED_INT] = numeric_cast<float, signed int>;
    numeric_cast_operations[type_id::FLOAT][type_id::SIGNED_SHORT_INT] = numeric_cast<float, signed short int>;
    numeric_cast_operations[type_id::FLOAT][type_id::SIGNED_LONG_INT] = numeric_cast<float, signed long int>;
    numeric_cast_operations[type_id::FLOAT][type_id::UNSIGNED_CHAR] = numeric_cast<float, unsigned char>;
    numeric_cast_operations[type_id::FLOAT][type_id::UNSIGNED_INT] = numeric_cast<float, unsigned int>;
    numeric_cast_operations[type_id::FLOAT][type_id::UNSIGNED_SHORT_INT] = numeric_cast<float, unsigned short int>;
    numeric_cast_operations[type_id::FLOAT][type_id::UNSIGNED_LONG_INT] = numeric_cast<float, unsigned long int>;
    numeric_cast_operations[type_id::FLOAT][type_id::FLOAT] = numeric_cast<float, float>;
    numeric_cast_operations[type_id::FLOAT][type_id::DOUBLE] = numeric_cast<float, double>;
    numeric_cast_operations[type_id::FLOAT][type_id::BOOL] = numeric_cast<float, bool>;
    numeric_cast_operations[type_id::FLOAT][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::DOUBLE][type_id::SIGNED_CHAR] = numeric_cast<double, signed char>;
    numeric_cast_operations[type_id::DOUBLE][type_id::SIGNED_INT] = numeric_cast<double, signed int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::SIGNED_SHORT_INT] = numeric_cast<double, signed short int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::SIGNED_LONG_INT] = numeric_cast<double, signed long int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::UNSIGNED_CHAR] = numeric_cast<double, unsigned char>;
    numeric_cast_operations[type_id::DOUBLE][type_id::UNSIGNED_INT] = numeric_cast<double, unsigned int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::UNSIGNED_SHORT_INT] = numeric_cast<double, unsigned short int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::UNSIGNED_LONG_INT] = numeric_cast<double, unsigned long int>;
    numeric_cast_operations[type_id::DOUBLE][type_id::FLOAT] = numeric_cast<double, float>;
    numeric_cast_operations[type_id::DOUBLE][type_id::DOUBLE] = numeric_cast<double, double>;
    numeric_cast_operations[type_id::DOUBLE][type_id::BOOL] = numeric_cast<double, bool>;
    numeric_cast_operations[type_id::DOUBLE][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::BOOL][type_id::SIGNED_CHAR] = numeric_cast<bool, signed char>;
    numeric_cast_operations[type_id::BOOL][type_id::SIGNED_INT] = numeric_cast<bool, signed int>;
    numeric_cast_operations[type_id::BOOL][type_id::SIGNED_SHORT_INT] = numeric_cast<bool, signed short int>;
    numeric_cast_operations[type_id::BOOL][type_id::SIGNED_LONG_INT] = numeric_cast<bool, signed long int>;
    numeric_cast_operations[type_id::BOOL][type_id::UNSIGNED_CHAR] = numeric_cast<bool, unsigned char>;
    numeric_cast_operations[type_id::BOOL][type_id::UNSIGNED_INT] = numeric_cast<bool, unsigned int>;
    numeric_cast_operations[type_id::BOOL][type_id::UNSIGNED_SHORT_INT] = numeric_cast<bool, unsigned short int>;
    numeric_cast_operations[type_id::BOOL][type_id::UNSIGNED_LONG_INT] = numeric_cast<bool, unsigned long int>;
    numeric_cast_operations[type_id::BOOL][type_id::FLOAT] = numeric_cast<bool, float>;
    numeric_cast_operations[type_id::BOOL][type_id::DOUBLE] = numeric_cast<bool, double>;
    numeric_cast_operations[type_id::BOOL][type_id::BOOL] = numeric_cast<bool, bool>;
    numeric_cast_operations[type_id::BOOL][type_id::NIL_INT] = to_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::SIGNED_CHAR] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::SIGNED_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::SIGNED_SHORT_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::SIGNED_LONG_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::UNSIGNED_CHAR] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::UNSIGNED_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::UNSIGNED_SHORT_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::UNSIGNED_LONG_INT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::FLOAT] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::DOUBLE] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::BOOL] = from_nil;
    numeric_cast_operations[type_id::NIL_INT][type_id::NIL_INT] = from_nil;
}

void cast(void * source, const type_id & source_type, void * destination, const type_id & destination_type)
{
    base_cast_operations[source_type.base()][destination_type.base()](source, source_type, destination, destination_type);
}

} //namespace script
} //namespace fungu
