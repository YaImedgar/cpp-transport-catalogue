#include "json_builder.h"

namespace json
{
    using std::string_literals::operator""s;

    BaseContext Builder::Value( const Node& value )
    {
        CheckReady( __func__ );

        if ( !head_.has_value() )
        {
            is_ready_ = true;
            head_ = value;
            return { *this };
        }

        if ( is_array_ )
        {
            auto& last_elem = nodes_stack_.top();

            if ( !last_elem->IsArray() )
            {
                throw std::logic_error( "Bad usage of array" );
            }

            auto& array = std::get<Array>( last_elem->GetValue() );

            array.emplace_back( value );

            if ( value.IsDict() || value.IsArray() )
            {
                nodes_stack_.push( &array.back() );
            }
        }
        else
        {
            nodes_stack_.top()->GetValue() = value;

            auto& elem = *nodes_stack_.top();

            if ( is_key_ )
            {
                nodes_stack_.pop();
            }

            if ( value.IsDict() || value.IsArray() )
            {
                nodes_stack_.push( &elem );
            }
        }

        if ( nodes_stack_.top()->IsDict() )
        {
            is_wait_key = true;
        }

        is_key_ = false;

        return { *this };
    }

    DictItemContext Builder::StartDict()
    {
        CheckReady( __func__ );
        CheckUsage( __func__ );

        Dict dict = Dict{};
        if ( InitHead( dict ) )
        {
            is_wait_key = true;
            return { *this };
        }

        Value( dict );

        is_array_ = false;
        is_wait_key = true;

        return { *this };
    }

    KeyItemContext Builder::Key( const std::string& key )
    {
        CheckReady( __func__ );
        CheckKey();

        is_wait_key = false;
        is_key_ = true;

        Dict& cur_dict = std::get<Dict>( nodes_stack_.top()->GetValue() );

        nodes_stack_.push( &cur_dict[key] );

        return { *this };
    }

    BaseContext Builder::EndDict()
    {
        EndStructure( __func__ );

        return { *this };
    }

    ArrayItemContext Builder::StartArray()
    {
        CheckReady( __func__ );
        CheckUsage( __func__ );

        Array arr = Array{};
        if ( InitHead( arr ) )
        {
            is_array_ = true;
            return { *this };
        }

        Value( arr );

        is_array_ = true;

        return { *this };
    }

    BaseContext Builder::EndArray()
    {
        EndStructure( __func__ );

        return { *this };
    }

    Node Builder::Build()
    {
        if ( !head_.has_value() )
        {
            throw std::logic_error( "Can't create blank json"s );
        }

        if ( !nodes_stack_.empty() || !is_ready_ )
        {
            throw std::logic_error( "Can't create non finished json"s );
        }

        return std::move( head_.value() );
    }

    void Builder::CheckReady( const std::string& function_name ) const
    {
        if ( is_ready_ )
        {
            throw std::logic_error( "Json object is already prepared. Bad usage of: \""s + function_name + "\""s );
        }

        if ( function_name == "Value" && is_wait_key )
        {
            throw std::logic_error( "Waited key after creation dictionary. Bad usage of: \""s + function_name + "\""s );
        }
    }

    void Builder::CheckEnd( const std::string& function_name ) const
    {
        std::string error_msg = "Bad usage of "s + function_name + ". "s;

        if ( nodes_stack_.empty() )
        {
            throw std::logic_error( error_msg + "Trying to close empty node"s );
        }

        if ( function_name == "EndArray" )
        {
            if ( !nodes_stack_.top()->IsArray() )
            {
                throw std::logic_error( error_msg + "Trying to close array"s );
            }
        }
        else if ( function_name == "EndDict" )
        {
            if ( !nodes_stack_.top()->IsDict() )
            {
                throw std::logic_error( error_msg + "Trying to close dictionary"s );
            }
        }
    }

    void Builder::CheckKey() const
    {
        std::string error_msg = "Bad usage of Key(). Trying to set Key for "s;
        if ( is_key_ )
        {
            throw std::logic_error( error_msg + "already defined key"s );
        }

        if ( nodes_stack_.empty() )
        {
            throw std::logic_error( error_msg + "empty node"s );
        }

        if ( !nodes_stack_.top()->IsDict() )
        {
            throw std::logic_error( error_msg + "non dictionary"s );
        }
    }

    void Builder::EndStructure( const std::string& function_name )
    {
        CheckReady( function_name );
        CheckEnd( function_name );

        nodes_stack_.pop();

        is_array_ = false;
        is_wait_key = false;

        if ( nodes_stack_.empty() )
        {
            is_ready_ = true;
        }
        else if ( nodes_stack_.top()->IsArray() )
        {
            is_array_ = true;
        }
        else if ( nodes_stack_.top()->IsDict() )
        {
            is_wait_key = true;
        }
    }

    bool Builder::InitHead( const Node& node )
    {
        if ( !head_.has_value() )
        {
            head_ = node;
            nodes_stack_.push( &( *head_ ) );
            return true;
        }
        return false;
    }

    void Builder::CheckUsage( const std::string& function_name ) const
    {
        if ( !head_.has_value() )
        {
            return;
        }

        if ( !is_key_ && ( nodes_stack_.empty() && !nodes_stack_.top()->IsArray() ) )
        {
            throw std::logic_error( "Bad usage of "s + function_name + ". "s +
                                    "Trying to add in non array or value of key"s );
        }
    }

    DictItemContext BaseContext::StartDict()
    {
        return builder_.StartDict();
    }

    KeyItemContext BaseContext::Key( const std::string& key )
    {
        return builder_.Key( key );
    }

    BaseContext BaseContext::EndDict()
    {
        return builder_.EndDict();
    }

    ArrayItemContext BaseContext::StartArray()
    {
        return builder_.StartArray();
    }

    BaseContext BaseContext::EndArray()
    {
        return builder_.EndArray();
    }

    json::Node BaseContext::Build()
    {
        return builder_.Build();
    }

    BaseContext::BaseContext( Builder& builder )
        : builder_( builder )
    {}

    BaseContext BaseContext::Value( const json::Node& value )
    {
        auto context = builder_.Value( value );
        return static_cast< ArrayValueItemContext >( context );
    }

    DictValueItemContext KeyItemContext::Value( const Node& value )
    {
        auto context = builder_.Value( value );
        CheckLastItem( value );
        return static_cast< DictValueItemContext >( context );
    }

    ArrayValueItemContext ArrayItemContext::Value( const json::Node& value )
    {
        auto context = builder_.Value( value );
        CheckLastItem( value );
        return static_cast< ArrayValueItemContext >( context );
    }

    ArrayValueItemContext::ArrayValueItemContext( BaseContext& context )
        : BaseContext( std::move( context ) )
    {}

    ArrayValueItemContext ArrayValueItemContext::Value( const Node& value )
    {
        auto context = builder_.Value( value );
        CheckLastItem( value );
        return static_cast< ArrayValueItemContext >( context );
    }

    void BaseContext::CheckLastItem( const Node& value )
    {
        if ( value.IsArray() )
        {
            builder_.EndArray();
        }
        else if ( value.IsDict() )
        {
            builder_.EndDict();
        }
    }

    DictValueItemContext::DictValueItemContext( BaseContext& context )
        : BaseContext( std::move( context ) )
    {}

    BaseValueItemContext::BaseValueItemContext( BaseContext& context )
        : BaseContext( std::move( context ) )
    {}
}
