#ifndef THEOREM_PROVER_PROPOSITIONAL_LOGIC_DEDUCTION_TREE
#define THEOREM_PROVER_PROPOSITIONAL_LOGIC_DEDUCTION_TREE
#include "propositional_combine.hpp"
#include "propositional_letter.hpp"
#include "value_less.hpp"
#include <map>
#include <memory>
#include <string>
namespace theorem_prover
{
	namespace propositional_logic
	{
		enum satisfiability
		{ valid, satisfiable, unsatisfiable };
		template< typename proposition >
		struct deduction_tree
		{
			std::map< const std::shared_ptr< proposition >, bool, value_less< std::shared_ptr< proposition > > > sequent;
			std::map< std::string, bool > expanded_symbol;
			bool insert( const std::shared_ptr< proposition > p, bool b )
			{
				auto res = sequent.insert( make_pair( p, b ) );
				if ( ( ! res.second ) && res.first->second != b ) { return false; }
				return true;
			}

			deduction_tree(
					const std::map< const std::shared_ptr< proposition >, bool, value_less< std::shared_ptr< proposition > > > & sequent,
					const std::map< std::string, bool > & expanded_symbol ) :
				sequent( sequent ), expanded_symbol( expanded_symbol ) { }

			deduction_tree( std::map< const std::shared_ptr< proposition >, bool, value_less< std::shared_ptr< proposition > > > && sequent ) :
				sequent( sequent ) { }

			bool is_satisfiable( const std::shared_ptr< proposition > & p, bool b )
			{
				deduction_tree nt( * this );
				if ( ! nt.insert( p, b ) ) { return false; }
				return nt.is_satisfiable( );
			}

			static bool need_branching( const std::shared_ptr< proposition > & prop, bool need_satisfy )
			{
				return ( ! prop->is_atom ) &&
						( ( boost::get< proposition_combine< const std::shared_ptr< proposition >  > >( prop->data ).s == logical_or && need_satisfy ) ||
							( boost::get< proposition_combine< const std::shared_ptr< proposition >  > >( prop->data ).s == logical_and && ! need_satisfy ) );
			}

			bool is_satisfiable( )
			{
				while ( ! sequent.empty( ) )
				{
					bool branching_allow = false;
					while ( true )
					{
						auto current_expand = sequent.begin( );
						if ( ! branching_allow )
						{
							while ( current_expand != sequent.end( ) )
							{
								if ( ! need_branching( current_expand->first, current_expand->second ) ) { break; }
								else { ++current_expand; }
							}
							if ( current_expand == sequent.end( ) )
							{
								branching_allow = true;
								continue;
							}
						}
						if ( current_expand == sequent.end( ) ) { break; }
						const std::shared_ptr< proposition > prop = current_expand->first;
						const bool need_satisfy = current_expand->second;
						sequent.erase( current_expand );
						if ( prop->is_atom )
						{
							auto res = expanded_symbol.insert( std::make_pair( boost::get< propositional_letter >( prop->data ).data, need_satisfy ) );
							if ( ( ! res.second ) && res.first->second != need_satisfy ) { return false; }
						}
						else
						{
							auto & p = boost::get< proposition_combine< const std::shared_ptr< proposition >  > >( prop->data );
							if ( p.s == logical_and )
							{
								if ( current_expand->second )
								{
									if ( ( ! insert( p.p.first, true ) ) ||
											 ( ! insert( p.p.second, true ) ) )
									{ return false; }
								}
								else if ( branching_allow )
								{
									if ( is_satisfiable( p.p.first, false ) ) { return true; }
									if ( ! insert( p.p.second, false ) ) { return false; }
								}
							}
							else if ( p.s == logical_or )
							{
								if ( current_expand->second )
								{
									if ( branching_allow )
									{
										if ( is_satisfiable( p.p.first, true ) ) { return true; }
										if ( ! insert( p.p.second, true ) ) { return false; }
									}
								}
								else
								{
									if ( ( ! insert( p.p.first, false ) ) ||
											 ( ! insert( p.p.second, false ) ) )
									{ return false; }
								}
							}
							else
							{ if ( ! insert( p.p.first, ! need_satisfy ) ) { return false; } }
						}
						branching_allow = false;
					}
				}
				return true;
			}
		};
	}
}
#endif //THEOREM_PROVER_PROPOSITIONAL_LOGIC_DEDUCTION_TREE
