// $Id$

#include "ace/ACE.h"
#include "orbsvcs/LoggerC.h"
#include "orbsvcs/Log/Logger_i.h"
#include "ace/Log_Record.h"

ACE_RCSID(Log, Logger_i, "$Id$")


  
#if defined (ACE_HAS_TEMPLATE_SPECIALIZATION)
#define TAO_Logger_Hash \
  ACE_Hash_Map_Manager<const char *, Logger_i *, ACE_Null_Mutex>
#define TAO_Logger_Hash_Entry \
        ACE_Hash_Map_Entry<const char *, Logger_i *>

long unsigned int
TAO_Logger_Hash::hash (const char *const &ext_id)
{
  return ACE::hash_pjw (ext_id);
}

int
TAO_Logger_Hash::equal (const char *const &id1, const char *const &id2)
{
  return ACE_OS::strcmp (id1, id2) == 0;
}


TAO_Logger_Hash_Entry::ACE_Hash_Map_Entry (const char *const &ext_id,
					   Logger_i *const &int_id,
					   TAO_Logger_Hash_Entry *next,
					   TAO_Logger_Hash_Entry *prev)
  : ext_id_ (ext_id ? ACE_OS::strdup (ext_id) : ACE_OS::strdup ("")),
    int_id_ (int_id),
    next_ (next),
    prev_ (prev)
{
}

TAO_Logger_Hash_Entry::ACE_Hash_Map_Entry (TAO_Logger_Hash_Entry *next,
					   TAO_Logger_Hash_Entry *prev)
  : ext_id_ (0),
    next_ (next),
    prev_ (prev)
{
}

TAO_Logger_Hash_Entry::~ACE_Hash_Map_Entry (void)
{
  ACE_OS::free ((void *) ext_id_);
}


#undef TAO_Logger_Hash
#undef TAO_Logger_Hash_Entry
#endif /* ACE_HAS_TEMPLATE_SPECIALIZATION */

Logger_Factory_i::Logger_Factory_i (void)
  :hash_map_ ()
{
  //Do nothing?
  //hash_map_.open (void);
}

Logger_Factory_i::~Logger_Factory_i (void)
{
  // Do nothing?
  //hash_map_.close (void);
}

Logger_ptr
Logger_Factory_i::make_logger (const char *name,
                               CORBA::Environment &_env)
{
  Logger_i  *result;
  // If name is already in the map, find() will assign <result> to the 
  // appropriate value
  if (hash_map_.find (name, result) != 0)
    {
      if (TAO_debug_level > 0)
	ACE_DEBUG ((LM_DEBUG,
		    "\nMaking a new logger"));
      ACE_NEW_RETURN (result,
		      Logger_i (name),
		      0);
      hash_map_.bind (name, result);
    }
  else
    {
      ACE_DEBUG ((LM_DEBUG,
		  "\nLogger name already bound"));
    }
  return result->_this (_env);
}



Logger_i::Logger_i (const char *name)
  : name_ (ACE_OS::strdup (name)),
    verbosity_level_ (Logger::VERBOSE)
{
  // Do nothing
}

Logger_i::~Logger_i (void)
{
  ACE_OS::free (this->name_);
}


ACE_Log_Priority
Logger_i::priority_conversion (Logger::Log_Priority priority)
{
  if (priority == Logger::LM_MAX)
    return LM_MAX;
  else
    {
      int pval = ACE_static_cast (int, priority);

      return ACE_static_cast (ACE_Log_Priority,
                              1 << pval);
      // (1 << pval) == 2^pval. <ACE_Log_Priority> are powers of 2.
    }
}

u_long
Logger_i::verbosity_conversion (Logger::Verbosity_Level verbosity_level)
{
  // This isn't very elegant, but it's because there's no simple
  // mapping from <Logger::Verbosity_Level>'s to the verbosity flags
  // specified in <ace/Log_Msg.h>
  switch (verbosity_level)
    {
    case Logger::SILENT:
      return 040;
    case Logger::VERBOSE_LITE:
      return 020;
    default:
    case Logger::VERBOSE:  
      return 010;
    }
}

void
Logger_i::log (const Logger::Log_Record &log_rec,
	       CORBA::Environment &_env)
{
  ACE_Time_Value temp (log_rec.time);

  // Create an <ACE_Log_Record> to leverage existing logging
  // code. Since Logger::Log_Priority enum tags don't cleanly map to
  // ACE_Log_Priority tags, <priority_conversion> is used to coerce
  // the mapping.
  ACE_Log_Record rec (this->priority_conversion (log_rec.type),
		      ACE_Time_Value (log_rec.time),
                      log_rec.app_id);
  
  // Create a temporary buffer for manipulating the logging message,
  // adding additional space for formatting characters..
  ASYS_TCHAR msgbuf [ACE_MAXLOGMSGLEN + 4];

  // Format the message for proper display.
  ACE_OS::strcpy (msgbuf, "\n::");

  // Copy the message data into the temporary buffer
  ACE_OS::strncat (msgbuf, 
		   log_rec.msg_data,
                   ACE_MAXLOGMSGLEN);

  // Set <ACE_Log_Record.msg_data> to the value stored in <msgbuf>.
  rec.msg_data (msgbuf);

  CORBA::Long addr = log_rec.host_addr;

  // The constructor for <ACE_INET_Addr> requires a port number, which
  // is not relevant in this context, so we give it 0.
  ACE_INET_Addr addy (ACE_static_cast (u_short, 0),
                      ACE_static_cast (ACE_UINT32,
                                       addr));

  // Create a buffer and fill it with the host name of the logger
  ASYS_TCHAR namebuf[MAXHOSTNAMELEN + 1];
  addy.get_host_name (namebuf,
                      MAXHOSTNAMELEN);

  u_long verb_level = this->verbosity_conversion (this->verbosity_level_);
  
  rec.print (namebuf,
	     verb_level,
	     stderr);
  // Print out the logging message to stderr with the given level of
  // verbosity
}

Logger::Verbosity_Level 
Logger_i::verbosity (void) const
{
  return verbosity_level_;
}

void
Logger_i::verbosity (Logger::Verbosity_Level level, CORBA::Environment &env)
{
  ACE_UNUSED_ARG (env);
  verbosity_level_ = level;
}
