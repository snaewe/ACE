// File: Trader_Utils.cpp
// $Id$

#include "Trader_Utils.h"

  // *************************************************************
  // TAO_Policy_Manager
  // *************************************************************

TAO_Policy_Manager::TAO_Policy_Manager (int num_policies)
  : policies_ (num_policies),
    num_policies_ (0)
{
  for (int i = 0; i < TAO_Policies::REQUEST_ID + 1; i++)
    this->poltable_[i] = -1;
}

void
TAO_Policy_Manager::search_card (CORBA::ULong scard)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::SEARCH_CARD);  
  policy.value <<= scard;    
}
  
void
TAO_Policy_Manager::match_card (CORBA::ULong mcard)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::MATCH_CARD);  
  policy.value <<= mcard;  
}

void
TAO_Policy_Manager::return_card (CORBA::ULong rcard)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::RETURN_CARD);  
  policy.value <<= rcard;  
}

void
TAO_Policy_Manager::use_modifiable_properties (CORBA::Boolean mod_props)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::USE_MODIFIABLE_PROPERTIES);  
  policy.value <<= CORBA::Any::from_boolean (mod_props);  
}

void
TAO_Policy_Manager::use_dynamic_properties (CORBA::Boolean dyn_props)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::USE_DYNAMIC_PROPERTIES);  
  policy.value <<= CORBA::Any::from_boolean (dyn_props);  
}

void
TAO_Policy_Manager::use_proxy_offers (CORBA::Boolean prox_offs)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::USE_PROXY_OFFERS);  
  policy.value <<= CORBA::Any::from_boolean (prox_offs);  
}

void
TAO_Policy_Manager::starting_trader (CosTrading::TraderName* name)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::STARTING_TRADER);  
  policy.value <<= *name;  
}

void
TAO_Policy_Manager::
link_follow_rule (CosTrading::FollowOption follow_option)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::LINK_FOLLOW_RULE);  
  policy.value <<= follow_option;  
}

void
TAO_Policy_Manager::hop_count (CORBA::ULong hop_count)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::HOP_COUNT);  
  policy.value <<= hop_count;  
}

void
TAO_Policy_Manager::exact_type_match (CORBA::Boolean exact_type)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::EXACT_TYPE_MATCH);  
  policy.value <<= CORBA::Any::from_boolean (exact_type);  
}

void
TAO_Policy_Manager::request_id (CosTrading::Admin::OctetSeq* request_id)
{
  CosTrading::Policy& policy =
    this->fetch_next_policy (TAO_Policies::REQUEST_ID);  
  policy.value <<= *request_id;
}

TAO_Policy_Manager::operator const CosTrading::PolicySeq& (void) const
{
  return this->policies_;
}

const CosTrading::PolicySeq&
TAO_Policy_Manager::policy_seq (void) const
{
  return this->policies_;
}

CosTrading::Policy&
TAO_Policy_Manager::fetch_next_policy (TAO_Policies::POLICY_TYPE pol_type)
{
  CORBA::ULong index = 0;

  if (this->poltable_[pol_type] == -1)
    {
      CORBA::ULong length = this->policies_.length ();
      this->num_policies_++;
      
      if (length < this->num_policies_)
	this->policies_.length (this->num_policies_);
      
      index = this->num_policies_ - 1;
      this->policies_[index].name = TAO_Policies::POLICY_NAMES[pol_type];
      this->poltable_[pol_type] = index;
    }
  else
    index = this->poltable_[pol_type];

  return this->policies_[index];
}


  // *************************************************************
  // TAO_Property_Evaluator
  // *************************************************************

// Constructor

TAO_Property_Evaluator::
TAO_Property_Evaluator(const CosTrading::PropertySeq& props,
		       CORBA::Boolean supports_dp)
  : props_ (props),
    supports_dp_ (supports_dp)
{
}


TAO_Property_Evaluator::
TAO_Property_Evaluator(CosTrading::Offer& offer,
		       CORBA::Boolean supports_dp)
  : props_ (offer.properties),
    supports_dp_ (supports_dp)
{
}

int
TAO_Property_Evaluator::is_dynamic_property (int index)
{
  CORBA::Environment env;
  int return_value = 0,
    num_properties = this->props_.length();

  // Ensure index is in bounds. 
  if (index >= 0 && index < num_properties)
    {
      // Obtain the value of the property at index <index>.
      const CORBA::Any& value = this->props_[index].value;
      CORBA::TypeCode* type = value.type ();
      
      if (type->equal (CosTradingDynamic::_tc_DynamicProp, env))
	return_value = 1;
    }
  
  return return_value;
}


CORBA::Any*
TAO_Property_Evaluator::property_value (int index,
                                        CORBA::Environment& _env)
    TAO_THROW_SPEC ((CosTradingDynamic::DPEvalFailure))
{
  CORBA::Any* prop_val = 0;
  
  if (! this->is_dynamic_property(index))
    prop_val = (CORBA::Any *) &(this->props_[index].value);
  else if (this->supports_dp_)
    {
      // Property is defined at this point.
      DP_Eval* dp_eval;
      CosTradingDynamic::DynamicProp* dp_struct;
      const CORBA::String_var name = this->props_[index].name;
      const CORBA::Any& value = this->props_[index].value;

      // Extract the DP_Struct.
      value >>= dp_struct;

#if defined TAO_HAS_OBJECT_IN_STRUCT_MARSHAL_BUG
      CORBA::ORB_ptr orb = TAO_ORB_Core_instance ()->orb ();
      CORBA::Object_ptr obj = orb->string_to_object (dp_struct->eval_if, _env);
      TAO_CHECK_ENV_RETURN (_env, 0);
      dp_eval = CosTradingDynamic::DynamicPropEval::_narrow (obj, _env);
      TAO_CHECK_ENV_RETURN (_env, 0);
#else
      dp_eval = dp_struct->eval_if;
#endif /* TAO_HAS_DYNAMIC_PROPERTY_BUG */
	  
      if (CORBA::is_nil (dp_eval))
	TAO_THROW_RETURN (CosTradingDynamic::DPEvalFailure (), prop_val);
      else
	{
	  CORBA::TypeCode* type = dp_struct->returned_type;
	  CORBA::Any& info = dp_struct->extra_info;
	  
	  TAO_TRY
	    {
	      // Retrieve the value of the dynamic property.
	      prop_val = dp_eval->evalDP(name, type, info, TAO_TRY_ENV);
	      TAO_CHECK_ENV;
	    }
	  TAO_CATCH (CORBA::SystemException, excp)
	    {
	      TAO_THROW_RETURN
		(CosTradingDynamic::DPEvalFailure (name, type, info),
		 prop_val);
	    }
	  TAO_ENDTRY;
	}
    }

  return prop_val;
}

CORBA::TypeCode*
TAO_Property_Evaluator::property_type(int index)
{
  CORBA::TypeCode* prop_type = CORBA::TypeCode::_nil();
  
  // Determine if property is both defined and dynamic.
  if (this->is_dynamic_property (index))
    {
      // Extract type information from the DP_Struct.
      const CORBA::Any& value = this->props_[index].value;
      CosTradingDynamic::DynamicProp* dp_struct;
      value >>= dp_struct;
      
      // Grab a pointer to the returned_type description
      prop_type = CORBA::TypeCode::_duplicate(dp_struct->returned_type);
    }
  else
    // TypeCode is self-evident at this point.
    prop_type = this->props_[index].value.type();
  
  return prop_type;
}

  // *************************************************************
  // TAO_Property_Evaluator_By_Name
  // *************************************************************

TAO_Property_Evaluator_By_Name::
TAO_Property_Evaluator_By_Name (const CosTrading::PropertySeq& properties,
				CORBA::Environment& _env,
				CORBA::Boolean supports_dp)
  TAO_THROW_SPEC ((CosTrading::DuplicatePropertyName,
		   CosTrading::IllegalPropertyName))
    : TAO_Property_Evaluator (properties, supports_dp)
{
  int length = this->props_.length();

  for (int i = 0; i < length; i++)
    {
      const CosTrading::Property& prop = this->props_[i];

      if (! TAO_Trader_Base::is_valid_identifier_name (prop.name))
	TAO_THROW (CosTrading::IllegalPropertyName (prop.name));
      
      TAO_String_Hash_Key prop_name = prop.name;
      if (this->table_.bind (prop_name, i))
	TAO_THROW (CosTrading::DuplicatePropertyName (prop.name));
    }
}

TAO_Property_Evaluator_By_Name::
TAO_Property_Evaluator_By_Name(CosTrading::Offer& offer,
			       CORBA::Boolean supports_dp)
  : TAO_Property_Evaluator(offer, supports_dp)
{
  int length = this->props_.length();

  for (int i = 0; i < length; i++)
    {
      TAO_String_Hash_Key prop_name = (const char*) this->props_[i].name;
      this->table_.bind (prop_name, i);
    }
}

int
TAO_Property_Evaluator_By_Name::
is_dynamic_property(const char* property_name)
{
  int predicate = 0, index = 0;
  TAO_String_Hash_Key prop_name (property_name);

  // If the property name is in the map, delegate evaluation to our
  // superclass. Otherwise, throw an exception.
  if (this->table_.find (prop_name, index) == 0)
    predicate = this->TAO_Property_Evaluator::is_dynamic_property(index);

  return predicate;
}

CORBA::Any* 
TAO_Property_Evaluator_By_Name::property_value (const char* property_name,
						CORBA::Environment& _env)
  TAO_THROW_SPEC ((CosTradingDynamic::DPEvalFailure))
{
  int index = 0;
  CORBA::Any* prop_value = 0;
  TAO_String_Hash_Key prop_name (property_name);

  // If the property name is in the map, delegate evaluation to our
  // superclass. Otherwise, throw an exception.
  if (this->table_.find (prop_name, index) == 0)
    prop_value = this->TAO_Property_Evaluator::property_value (index, _env);

  return prop_value;
}
  
CORBA::TypeCode*
TAO_Property_Evaluator_By_Name::property_type (const char* property_name)
{
  int index = 0;
  TAO_String_Hash_Key prop_name (property_name);
  CORBA::TypeCode* prop_type = CORBA::TypeCode::_nil();
  
  // If the property name is in the map, delegate evaluation to our
  // superclass. Otherwise, throw an exception.
  if (this->table_.find (prop_name, index) == 0)
    prop_type = this->TAO_Property_Evaluator::property_type (index);

  return prop_type;
}

const CosTrading::Property*
TAO_Property_Evaluator_By_Name::get_property (const char* property_name)
{
  int index = 0;
  CosTrading::Property* property = 0;
  TAO_String_Hash_Key prop_name (property_name);

  if (this->table_.find (prop_name, index) == 0)
    property = (CosTrading::Property *) &this->props_[index];

  return property;
}

  // *************************************************************
  // TAO_Policies
  // *************************************************************

const char* TAO_Policies::POLICY_NAMES[] = 
{
  "exact_type_match",
  "hop_count",
  "link_follow_rule",
  "match_card",
  "return_card",
  "search_card",
  "starting_trader",
  "use_dynamic_properties",
  "use_modifiable_properties",
  "use_proxy_offers",
  "request_id"
};

TAO_Policies::TAO_Policies (TAO_Trader_Base& trader,
			    const CosTrading::PolicySeq& policies,
			    CORBA::Environment& _env)
  TAO_THROW_SPEC ((CosTrading::Lookup::IllegalPolicyName,
		  CosTrading::DuplicatePolicyName))
  : trader_ (trader)
{
  for (int i = 0; i < TAO_NUM_POLICIES; i++)
    this->policies_[i] = 0;

  for (CORBA::ULong j = 0; j < policies.length (); j++)
    {      
      const char* pol_name = (const char*) policies[j].name;
      size_t length = (pol_name == 0) ? 0 : ACE_OS::strlen (pol_name);
      int index = -1;

      if (length < ACE_OS::strlen (POLICY_NAMES[HOP_COUNT]))
	TAO_THROW (CosTrading::Lookup::IllegalPolicyName (pol_name));
      
      switch (pol_name[0])
	{
	case 'e':
	  index = EXACT_TYPE_MATCH;
	  break;
	case 'h':
	  index = HOP_COUNT;
	  break;
	case 'l':
	  index = LINK_FOLLOW_RULE;
	  break;
	case 'm':
	  index = MATCH_CARD;
	  break;
	case 'r':
	  if (pol_name[2] == 't')
	    index = RETURN_CARD;
	  else if (pol_name[2] == 'q')
	    index = REQUEST_ID;
	  break;
	case 's':
	  if (pol_name[1] == 't')
	    index == STARTING_TRADER;
	  else if (pol_name[1] == 'e')
	    index = SEARCH_CARD;
	  break;
	case 'u':
	  if (pol_name[4] == 'd')
	    index = USE_DYNAMIC_PROPERTIES;
	  if (pol_name[4] == 'm')
	    index = USE_MODIFIABLE_PROPERTIES;
	  if (pol_name[4] == 'p')
	    index = USE_PROXY_OFFERS;
	}

      // Match the name of the policy, and insert its value into the
      // vector. 
      if (index == -1 || ACE_OS::strcmp (POLICY_NAMES[index], pol_name) != 0)
	TAO_THROW (CosTrading::Lookup::IllegalPolicyName (pol_name));
      else
	{
	  if (this->policies_[index] != 0)
	    TAO_THROW (CosTrading::DuplicatePolicyName (pol_name));
	  else
	    this->policies_[index] = (CosTrading::Policy *) &(policies[j]);
	}
    }
}

TAO_Policies::~TAO_Policies (void)
{
}

CORBA::ULong
TAO_Policies::ulong_prop (POLICY_TYPE pol,
			  CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  CORBA::ULong return_value = 0, max_value;
  const TAO_Import_Attributes_Impl& import_attrs =
    this->trader_.import_attributes ();

  // Discover the default values for each of the possible cardinality
  // policies. 
  switch (pol)
    {
    case SEARCH_CARD:
      return_value = import_attrs.def_search_card ();
      max_value = import_attrs.max_search_card ();
      break;
    case MATCH_CARD:
      return_value = import_attrs.def_match_card ();
      max_value = import_attrs.max_match_card ();
      break;
    case RETURN_CARD:
      return_value = import_attrs.def_return_card ();
      max_value = import_attrs.max_return_card ();
      break;
    case HOP_COUNT:
      return_value = import_attrs.def_hop_count ();
      max_value = import_attrs.max_hop_count ();
      break;
    }
  
  if (this->policies_[pol] != 0)
    {
      // Extract the desired policy value.
      const CosTrading::Policy* policy = this->policies_[pol];
      const CosTrading::PolicyValue& value = policy->value;
      CORBA::TypeCode* type = value.type ();
      
      if (!type->equal (CORBA::_tc_ulong, _env))
	TAO_THROW_RETURN (CosTrading::Lookup::PolicyTypeMismatch (*policy),
			  return_value);
      else
	value >>= return_value;

      if (max_value < return_value)
	return_value = max_value;
    }
  
  return return_value;
}

CORBA::ULong
TAO_Policies::search_card (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->ulong_prop (SEARCH_CARD, _env);
}

CORBA::ULong
TAO_Policies::match_card (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->ulong_prop (MATCH_CARD, _env);
}

CORBA::ULong
TAO_Policies::return_card (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->ulong_prop (RETURN_CARD, _env);
}

CORBA::Boolean
TAO_Policies::boolean_prop (POLICY_TYPE pol,
			    CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  CORBA::Boolean def_value = CORBA::B_TRUE,
    return_value = CORBA::B_TRUE;
  const TAO_Support_Attributes_Impl& support_attrs =
    this->trader_.support_attributes ();
  
  switch (pol)
    {
    case USE_MODIFIABLE_PROPERTIES:
      def_value = support_attrs.supports_modifiable_properties ();
      break;
    case USE_DYNAMIC_PROPERTIES:
      def_value = support_attrs.supports_dynamic_properties ();
      break;
    case USE_PROXY_OFFERS:
      def_value = support_attrs.supports_proxy_offers ();
      break;
    case EXACT_TYPE_MATCH:
      def_value = CORBA::B_FALSE;
      break;
    }
  
  if (this->policies_[pol] != 0)
    {
      const CosTrading::Policy* policy = this->policies_[pol];
      const CosTrading::PolicyValue& value = policy->value;
      CORBA::TypeCode* type = value.type ();
      
      if (!type->equal (CORBA::_tc_boolean, _env))
	TAO_THROW_RETURN (CosTrading::Lookup::PolicyTypeMismatch (*policy),
			  return_value);
      else
        value >>= CORBA::Any::to_boolean (return_value);

      if (def_value == CORBA::B_FALSE &&
	  pol != EXACT_TYPE_MATCH)
	return_value = CORBA::B_FALSE;
    }
  else
    return_value = def_value;

  return return_value;
}


CORBA::Boolean
TAO_Policies::use_modifiable_properties (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->boolean_prop (USE_MODIFIABLE_PROPERTIES, _env);
}

CORBA::Boolean
TAO_Policies::use_dynamic_properties (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->boolean_prop (USE_DYNAMIC_PROPERTIES, _env);
}

CORBA::Boolean
TAO_Policies::use_proxy_offers (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->boolean_prop (USE_PROXY_OFFERS, _env);
}

CORBA::Boolean
TAO_Policies::exact_type_match (CORBA::Environment& _env) const
    TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->boolean_prop (EXACT_TYPE_MATCH, _env);
}


CosTrading::TraderName*
TAO_Policies::starting_trader (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch,
		   CosTrading::Lookup::InvalidPolicyValue))
{
  CosTrading::TraderName* trader_name = 0;
  
  if (this->policies_[STARTING_TRADER] != 0)
    {
      CosTrading::Policy* policy = this->policies_[STARTING_TRADER];
      CosTrading::PolicyValue& value = policy->value;
      CORBA::TypeCode* type = value.type ();
      
      if (!type->equal (CosTrading::_tc_TraderName, _env))
	TAO_THROW_RETURN (CosTrading::Lookup::PolicyTypeMismatch (*policy),
			  trader_name);
      else
        value >>= trader_name;
    }

  return trader_name;
}

CosTrading::FollowOption
TAO_Policies::link_follow_rule (CORBA::Environment& _env) const
    TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  CosTrading::FollowOption return_value = 
    this->trader_.import_attributes ().def_follow_policy ();

  if (this->policies_[LINK_FOLLOW_RULE] != 0)
    {
      CosTrading::FollowOption max_follow_policy = 
	this->trader_.import_attributes ().max_follow_policy ();

      CosTrading::Policy* policy = this->policies_[LINK_FOLLOW_RULE];
      CosTrading::PolicyValue& value = policy->value;
      CORBA::TypeCode* type = value.type ();

      // Extract the link follow rule 
      if (!type->equal (CosTrading::_tc_FollowOption, _env))
	TAO_THROW_RETURN (CosTrading::Lookup::PolicyTypeMismatch (*policy),
			  return_value);
      else
	value >>= return_value;

      if (return_value > max_follow_policy)
	return_value = max_follow_policy;
    }

  return return_value;
}

CosTrading::FollowOption
TAO_Policies::link_follow_rule (const CosTrading::Link::LinkInfo& link_info,
				CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch,
		   CosTrading::Lookup::InvalidPolicyValue,
		   CosTrading::Link::IllegalLinkName,
		   CosTrading::Link::UnknownLinkName))
{
  CosTrading::FollowOption return_value = CosTrading::local_only;
  CosTrading::FollowOption trader_max_follow_policy = 
    this->trader_.import_attributes ().max_follow_policy ();
  CosTrading::FollowOption link_limiting_follow_rule =
    link_info.limiting_follow_rule;
  
  // If not defined defaults to trader.def_link_follow_rule
  CosTrading::FollowOption query_link_follow_rule =
    this->link_follow_rule (_env);
  TAO_CHECK_ENV_RETURN (_env, return_value);
  
  return_value = (query_link_follow_rule < trader_max_follow_policy) 
    ? query_link_follow_rule : trader_max_follow_policy;
  return_value = (return_value < link_limiting_follow_rule)
    ? return_value : link_limiting_follow_rule;

  return return_value;
}

CORBA::ULong
TAO_Policies::hop_count (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  return this->ulong_prop (HOP_COUNT, _env);
}

CosTrading::Admin::OctetSeq*
TAO_Policies::request_id (CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Lookup::PolicyTypeMismatch))
{
  CosTrading::Admin::OctetSeq* request_id = 0;
  
  if (this->policies_[REQUEST_ID] != 0)
    {
      CosTrading::Policy* policy = this->policies_[REQUEST_ID];
      CosTrading::PolicyValue& value = policy->value;
      CORBA::TypeCode* type = value.type ();
      
      if (!type->equal (CosTrading::Admin::_tc_OctetSeq, _env))
        {
          TAO_THROW_RETURN (CosTrading::Lookup::PolicyTypeMismatch (*policy),
                            request_id);
        }
      else
        value >>= request_id;
    }

  return request_id;
}

void
TAO_Policies::
copy_in_follow_option (CosTrading::PolicySeq& policy_seq,
                       const CosTrading::Link::LinkInfo& link_info,
                       CORBA::Environment& _env) const
  TAO_THROW_SPEC ((CosTrading::Link::IllegalLinkName,
		   CosTrading::Link::UnknownLinkName))
{
  CosTrading::FollowOption follow_option = CosTrading::local_only;
  CosTrading::FollowOption trader_max_follow_policy = 
    this->trader_.import_attributes ().max_follow_policy ();

  CosTrading::FollowOption link_limiting_follow_rule =
    link_info.limiting_follow_rule;
  
  if (this->policies_[LINK_FOLLOW_RULE] != 0)
    {
      CORBA::Environment env;
      CosTrading::FollowOption query_link_follow_rule =
        this->link_follow_rule (env);
      TAO_CHECK_ENV_RETURN_VOID (env);
      
      follow_option =
        (link_info.limiting_follow_rule < trader_max_follow_policy) ?
        ((link_info.limiting_follow_rule < query_link_follow_rule) ?
         link_info.limiting_follow_rule : query_link_follow_rule) :
        ((trader_max_follow_policy < query_link_follow_rule) ?
         trader_max_follow_policy : query_link_follow_rule);
    }
  else
    {
      follow_option =
        (link_info.def_pass_on_follow_rule < trader_max_follow_policy) ?
        link_info.def_pass_on_follow_rule : trader_max_follow_policy;
    }

  int i = 0;
  for (i = 0; i < policy_seq.length (); i++)
    {
      if (ACE_OS::strcmp (policy_seq[i].name,
                          POLICY_NAMES[LINK_FOLLOW_RULE]) == 0)
        {
          policy_seq[i].value <<= follow_option;
          break;
        }
    }

  if (i == policy_seq.length ())
    {
      policy_seq.length (i + 1);
      policy_seq[i].name = POLICY_NAMES[LINK_FOLLOW_RULE];
      policy_seq[i].value <<= follow_option;
    }
}

void
TAO_Policies::
copy_to_pass (CosTrading::PolicySeq& policy_seq,
              const CosTrading::Admin::OctetSeq& request_id) const
{
  CORBA::ULong counter = 0;
  CosTrading::Policy* policy_buffer =
    CosTrading::PolicySeq::allocbuf (REQUEST_ID + 1);
  
  if (policy_buffer == 0)
    return;
  
  for (int i = EXACT_TYPE_MATCH; i <= REQUEST_ID; i++)
    {
      CosTrading::Policy& new_policy = policy_buffer[counter];
      
      if (this->policies_[i] != 0)
	{
	  // Copy in the existing policies.
	  new_policy.name = POLICY_NAMES[i];
          new_policy.value = this->policies_[i]->value;          
          counter++;
        }
      else if (i == REQUEST_ID)
        {
          // Set the new request id.
          new_policy.name = POLICY_NAMES[REQUEST_ID];
          new_policy.value <<= request_id;
          counter++;
        }

      // We always require a hop count.
      if (i == HOP_COUNT)
        {
          CORBA::Environment env;
          new_policy.name = POLICY_NAMES[HOP_COUNT];
          new_policy.value <<= this->hop_count (env) - 1;

          // Don't count hop count twice.
          if (this->policies_[i] == 0)
            counter++;
        }
    }

  policy_seq.replace (REQUEST_ID + 1, counter, policy_buffer, CORBA::B_TRUE);
}


void
TAO_Policies::copy_to_forward (CosTrading::PolicySeq& policy_seq,
                               CosTrading::TraderName* trader_name) const
{
  // Create a new policy sequence, shortening the starting trader
  // policy by one link.
  
  CORBA::ULong counter = 0;
  CosTrading::Policy* policy_buffer =
    CosTrading::PolicySeq::allocbuf (REQUEST_ID + 1);
  
  if (policy_buffer == 0)
    return;
  
  for (int i = EXACT_TYPE_MATCH; i <= REQUEST_ID; i++)
    {
      CosTrading::Policy& new_policy = policy_buffer[counter];
      
      if (this->policies_[i] != 0)
	{
	  // Copy in the existing policies.
	  if (i == STARTING_TRADER)
	    {
              // Eliminate the first link of the trader name.
              CORBA::ULong length = trader_name->length ();
              
              if (length > 1)
                {
                  // Only pass on the property if the sequence
                  // contains more links after us. 
                  for (CORBA::ULong j = 1; j < length; j++)
                    (*trader_name)[j - 1] = (*trader_name)[j];  
                      
                  trader_name->length (length - 1);
                  new_policy.name = this->policies_[i]->name;
                  new_policy.value <<= trader_name;
                  counter++;
                }
            }
	  else
	    {
              new_policy.name = this->policies_[i]->name;
	      new_policy.value = this->policies_[i]->value;
	      counter++;
	    }
	}
    }
  
  // Create the new sequence
  policy_seq.replace (REQUEST_ID + 1, counter, policy_buffer, CORBA::B_TRUE);
}

  // *************************************************************
  // TAO_Offer_Modifier
  // *************************************************************

TAO_Offer_Modifier::
TAO_Offer_Modifier (const char* type_name,
		    CosTradingRepos::ServiceTypeRepository::TypeStruct* type_struct,
		    CosTrading::Offer& offer)
  : offer_ (offer),
    type_ (type_name)
{
  CosTrading::PropertySeq& prop_seq = this->offer_.properties;
  CosTradingRepos::ServiceTypeRepository::PropStructSeq&
    pstructs = type_struct->props;
  CORBA::ULong pstructs_length = pstructs.length (),
    props_length = prop_seq.length (),
    i = 0;

  // Separate the type defined properties into mandatory and readonly
  for (i = 0; i < pstructs_length; i++)
    {
      if (pstructs[i].mode ==
	  CosTradingRepos::ServiceTypeRepository::PROP_MANDATORY)
	{
	  TAO_String_Hash_Key prop_name ((const char *) pstructs[i].name);
	  this->mandatory_.insert (prop_name);
	}
      else if (pstructs[i].mode ==
	       CosTradingRepos::ServiceTypeRepository::PROP_READONLY)
	{
	  TAO_String_Hash_Key prop_name ((const char *) pstructs[i].name);
	  this->readonly_.insert (prop_name);
	}
    }

  // Insert the properties of the offer into a map.
  for (i = 0; i < props_length; i++)
    {
      TAO_String_Hash_Key prop_name = (const char*) prop_seq[i].name;
      this->props_.bind (prop_name, &prop_seq[i]);
    }
}

void
TAO_Offer_Modifier::
delete_properties (const CosTrading::PropertyNameSeq& deletes,
		   CORBA::Environment& _env)
  TAO_THROW_SPEC ((CosTrading::Register::UnknownPropertyName, 
		  CosTrading::Register::MandatoryProperty,
		  CosTrading::IllegalPropertyName,
		  CosTrading::DuplicatePropertyName))
{
  Prop_Names delete_me;
  // Validate that the listed property names can be deleted
  CORBA::ULong i = 0,
    length = deletes.length ();
  for (i = 0, length = deletes.length (); i < length; i++)
    {
      const char* dname = (const char *) deletes[i];
      if (! TAO_Trader_Base::is_valid_identifier_name (dname))
	TAO_THROW (CosTrading::IllegalPropertyName (dname));
      else
	{
	  TAO_String_Hash_Key prop_name (dname);
	  if (this->mandatory_.find (prop_name) == 0)
	    TAO_THROW (CosTrading::Register::MandatoryProperty (this->type_, dname));
	  else if (delete_me.insert (prop_name) == 1)
	    TAO_THROW (CosTrading::DuplicatePropertyName (dname));
	  else if (this->props_.find (prop_name) == 0)
	    TAO_THROW (CosTrading::Register::UnknownPropertyName (dname));
	}
    }

  // Delete those properties from the offer.
  for (i = 0; i < length; i++)
    {
      TAO_String_Hash_Key prop_name = (const char *) deletes[i];
      this->props_.unbind (prop_name);
    } 
}

void
TAO_Offer_Modifier::
merge_properties (const CosTrading::PropertySeq& modifies,
		  CORBA::Environment& _env)
  TAO_THROW_SPEC ((CosTrading::IllegalPropertyName,
		   CosTrading::DuplicatePropertyName,
		   CosTrading::Register::ReadonlyProperty))
{
  int i = 0, length = 0;
  Prop_Names modify_me;
  // Ensure that the proposed changes aren't to readonly properties or
  // otherwise invalid.
  for (i = 0, length = modifies.length (); i < length; i++)
    {
      const char*  mname =(const char *) modifies[i].name;
      if (! TAO_Trader_Base::is_valid_identifier_name (mname))
	TAO_THROW (CosTrading::IllegalPropertyName (mname));
      else
	{
	  TAO_String_Hash_Key prop_name (mname);
	  if (this->readonly_.find (prop_name) == 0 &&
	      this->props_.find (prop_name) == 0)
	    TAO_THROW (CosTrading::Register::ReadonlyProperty (this->type_, mname));
	  else if (modify_me.insert (prop_name) == 1)
	    TAO_THROW (CosTrading::DuplicatePropertyName (mname));
	}
    }

  for (i = 0; i < length; i++)
    {
      // Add a property to the destination if it doesn't already exist.
      Props::ENTRY* existing_entry = 0;
      TAO_String_Hash_Key prop_name ((const char*) modifies[i].name);

      if (this->props_.bind (prop_name,
			     (CosTrading::Property *) &modifies[i],
			     existing_entry) == 1)
	{
	  // Modify a property if it already exists in the destination.
	  CosTrading::Property* prop = existing_entry->int_id_;	  
	  prop->value = modifies[i].value;
	}
    }
}

CosTrading::Offer&
TAO_Offer_Modifier::affect_change (void)
{
  int elem = 0;
  CosTrading::PropertySeq prop_seq;

  // Create a new property list reflecting the deletes, modifies, and
  // add operations performed, and place this property list in the
  // offer. 
  prop_seq.length (this->props_.current_size ());
  for (Props::iterator props_iter (this->props_);
       ! props_iter.done ();
       props_iter++, elem++)
    {
      prop_seq[elem] = *(*props_iter).int_id_;
    }

  this->offer_.properties = prop_seq;
  return this->offer_;
}

  // *************************************************************
  // TAO_Offer_Filter
  // *************************************************************

TAO_Offer_Filter::
TAO_Offer_Filter (SERVICE_TYPE_REPOS::TypeStruct* type_struct,
		  TAO_Policies& policies,
		  CORBA::Environment& _env)
  : search_card_ (policies.search_card (_env)),
    match_card_ (policies.match_card (_env)),
    return_card_ (policies.return_card (_env)),
    dp_ (policies.use_dynamic_properties (_env)),
    mod_ (policies.use_modifiable_properties (_env))
{
  SERVICE_TYPE_REPOS::PropStructSeq& prop_seq = type_struct->props;

  // Take note of modifiable properties in the type_struct
  for (int i = prop_seq.length () - 1; i >= 0; i--)
    {
      SERVICE_TYPE_REPOS::PropertyMode mode = prop_seq[i].mode;
      if (mode != SERVICE_TYPE_REPOS::PROP_MANDATORY_READONLY ||
	  mode != SERVICE_TYPE_REPOS::PROP_READONLY)
	{
	  TAO_String_Hash_Key prop_name ((const char*) prop_seq[i].name);
	  this->mod_props_.insert (prop_name);
	}
    }

  if (policies.exact_type_match (_env) == CORBA::B_TRUE)
    {
      TAO_String_Hash_Key exact_match
	(TAO_Policies::POLICY_NAMES[TAO_Policies::EXACT_TYPE_MATCH]);
      this->limits_.insert (exact_match);
    }
    
}

CORBA::Boolean
TAO_Offer_Filter::ok_to_consider (CosTrading::Offer* offer)
{
  TAO_String_Hash_Key use_mods =
    TAO_Policies::POLICY_NAMES[TAO_Policies::USE_MODIFIABLE_PROPERTIES];
  TAO_String_Hash_Key use_dyns =
    TAO_Policies::POLICY_NAMES[TAO_Policies::USE_DYNAMIC_PROPERTIES];
  CORBA::Boolean return_value = CORBA::B_TRUE;
  TAO_Property_Evaluator prop_eval (*offer);

  // If we should screen offers, determine if this offer is unworthy
  // for consideration. 
  if (! (this->mod_ && this->dp_))
    {
      for (int i = offer->properties.length () - 1;
	   i >= 0 && return_value;
	   i--)
	{
	  // Winnow away the unwanted offers with modifiable or
	  // dynamic properties.
	  if (! this->mod_)
	    {
	      // Determine if this property name is found in the set
	      // of modifiable properties for the type being considered.
	      TAO_String_Hash_Key prop_name ((const char*) offer->properties[i].name);
	      if (this->mod_props_.find (prop_name) == 0)
		{
		  this->limits_.insert (use_mods);
		  return_value = 0;
		}
	    }

	  if (! this->dp_ && return_value)
	    {
	      // Determine if this property is dynamic.
	      if (prop_eval.is_dynamic_property (i))
		{
		  this->limits_.insert (use_dyns);
		  return_value = 0;
		}
	    }

	  if (return_value == 0)
	    break;
	}
    }

  // If we're good to go, consider this offer considered and decrement 
  // the search cardinality counter. 
  if (return_value)
    {
      this->search_card_--;
      if (this->search_card_ == 0)
	{
	  TAO_String_Hash_Key search_card =
	    TAO_Policies::POLICY_NAMES[TAO_Policies::SEARCH_CARD];
	  this->limits_.insert (search_card);
	}
    }

  return return_value;
}

CORBA::Boolean
TAO_Offer_Filter::ok_to_consider_more (void)
{
  return this->search_card_ > 0 && this->match_card_ > 0;
}

void
TAO_Offer_Filter::matched_offer (void)
{
  this->match_card_--;
  this->return_card_--;

  if (this->match_card_ == 0)
    {
      TAO_String_Hash_Key match_card =
	TAO_Policies::POLICY_NAMES[TAO_Policies::MATCH_CARD];
      this->limits_.insert (match_card);
    }

  if (this->return_card_ == 0)
    {
      TAO_String_Hash_Key return_card =
	TAO_Policies::POLICY_NAMES[TAO_Policies::MATCH_CARD];
      this->limits_.insert (return_card);
    }
}

CosTrading::PolicyNameSeq*
TAO_Offer_Filter::limits_applied (void)
{
  int i = 0;
  CORBA::ULong size = this->limits_.size ();
  CosTrading::PolicyName* temp =
    CosTrading::PolicyNameSeq::allocbuf (size);

  for (Names::iterator p_iter (this->limits_.begin());
       ! p_iter.done ();
       p_iter.advance ())
    {
      TAO_String_Hash_Key* policy_name_ptr = 0;
      p_iter.next (policy_name_ptr);
      temp[i++] = CORBA::string_dup ((const char*) *policy_name_ptr);
    }
  
  return new CosTrading::PolicyNameSeq (i, i, temp, 1);
}

  // *************************************************************
  // TAO_Property_Filter
  // *************************************************************

TAO_Property_Filter::
TAO_Property_Filter (const SPECIFIED_PROPS& desired_props,
		     CORBA::Environment& _env)
  TAO_THROW_SPEC ((CosTrading::IllegalPropertyName,
		   CosTrading::DuplicatePropertyName))
  : policy_  (desired_props._d ())
{
  if (this->policy_ == CosTrading::Lookup::some)
    {
      const CosTrading::PropertyNameSeq&
	prop_seq = desired_props.prop_names ();
      int length = prop_seq.length ();

      for (int i = 0; i < length; i++)
	{
	  const char* pname = prop_seq[i];

	  // Check for errors or duplicates
	  if (TAO_Trader_Base::is_valid_identifier_name (pname))
	    {
	      TAO_String_Hash_Key prop_name (pname);
	      if (this->props_.insert (prop_name) == 1)
                TAO_THROW (CosTrading::DuplicatePropertyName (pname));
	    }
	  else
	    TAO_THROW (CosTrading::IllegalPropertyName (pname));
	}
    }
}

TAO_Property_Filter::
TAO_Property_Filter (const TAO_Property_Filter& prop_filter)
  : props_ (prop_filter.props_),
    policy_ (prop_filter.policy_)
{  
}

TAO_Property_Filter&
TAO_Property_Filter::operator= (const TAO_Property_Filter& other)
{
  this->props_ = other.props_;
  this->policy_ = other.policy_;

  return *this;
}

void
TAO_Property_Filter::filter_offer (CosTrading::Offer& source,
				   CosTrading::Offer& destination)
{
  Prop_Queue prop_queue;
  CosTrading::PropertySeq& s_props = source.properties;
  CosTrading::PropertySeq& d_props = destination.properties;
  int length = s_props.length (), elem = 0;

  destination.reference = source.reference->_duplicate (source.reference);
  if (this->policy_ != CosTrading::Lookup::none)
    {
      for (int i = 0; i < length; i++)
	{
	  if (this->policy_ == CosTrading::Lookup::all)
	    prop_queue.enqueue_tail (&s_props[i]);
	  else
	    {
	      TAO_String_Hash_Key prop_name ((const char*) s_props[i].name);

	      // Save those property that match.
	      if (this->props_.find (prop_name) == 0)
		prop_queue.enqueue_tail (&s_props[i]);
	    }
	}

      // Shove the matched properties into the destination property
      // sequence. 
      length = prop_queue.size ();
      d_props.length (prop_queue.size ());
      for (Prop_Queue::ITERATOR prop_iter (prop_queue);
	   ! prop_iter.done ();
	   prop_iter.advance (), elem++)
	{
	  CosTrading::Property** prop_ptr = 0;
	  
	  prop_iter.next (prop_ptr);
	  d_props[elem] = **prop_ptr;
	}
    }
}

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)
template class ACE_Hash_Map_Manager<TAO_String_Hash_Key, int, ACE_Null_Mutex>;
template class ACE_Hash_Map_Manager<TAO_String_Hash_Key, CosTrading::Property*, ACE_Null_Mutex>;
template class ACE_Unbounded_Set<TAO_String_Hash_Key>;
template class ACE_Unbounded_Queue<CosTrading::Property*>;
#elif defined (ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)
#pragma instantiate ACE_Hash_Map_Manager<TAO_String_Hash_Key, int, ACE_Null_Mutex>;
#pragma instantiate ACE_Hash_Map_Manager<TAO_String_Hash_Key, CosTrading::Property*, ACE_Null_Mutex>;
#pragma instantiate ACE_Unbounded_Set<TAO_String_Hash_Key>;
#pragma instantiate ACE_Unbounded_Queue<CosTrading::Property*>;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
