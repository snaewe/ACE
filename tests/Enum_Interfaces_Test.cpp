/* -*- C++ -*- */
// $Id$

// ============================================================================
//
// = LIBRARY
//    tests
//
// = FILENAME
//    Enum_interfaces.cpp
//
// = DESCRIPTION
//     This is a simple test of <ACE::get_ip_interfaces>.  This call
//     retrieves the IP addresses assigned to the host by
//     interrogating the kernel.  Network applications typically
//     assume gethostbyname(uname()) will work, but this is just a
//     convention. It is also problematic if the resolver code
//     (DNS/NIS+...) is misconfigured. This happens more than
//     programmers realize. It is better to find out by asking the
//     kernel for local address assignments. This API is similar to
//     what netstat -ni or ifconfig -a produces on UNIX or ipconfig on
//     Windows NT. In fact, it was by reverse engineering these tools
//     that this api was created.
//
// = AUTHOR
//    Michael R. MacFaden <mrm@cisco.com>
//
// ============================================================================

#include "test_config.h"
#include "ace/OS_NS_sys_utsname.h"
#include "ace/INET_Addr.h"

ACE_RCSID(tests, Enum_Interfaces_Test, "$Id$")

int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Enum_Interfaces_Test"));

  ACE_utsname uname;
  ACE_OS::uname (&uname);
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Machine: %s running on %s\n"),
              uname.nodename, uname.machine ));
  ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("Platform: %s, %s, %s\n"),
              uname.sysname, uname.release, uname.version ));

  ACE_INET_Addr *the_addr_array;
  size_t how_many = 0;

  int rc = ACE::get_ip_interfaces (how_many, the_addr_array);

  if (rc != 0)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("%p\n"),
                ACE_TEXT ("ACE::get_ip_interfaces failed")));
  else if (how_many == 0)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("No interfaces presently configured in the kernel\n")));
  else
    {
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT (" there are %d interfaces\n"), how_many));

#if defined (ACE_HAS_IPV6)
      int num_ipv4 = 0;
      int num_ipv6 = 0;
#endif /* ACE_HAS_IPV6 */
      for (size_t i = 0; i < how_many; i++)
        {
#if defined (ACE_HAS_IPV6)
          if (the_addr_array[i].get_type() == AF_INET)
            ++num_ipv4;
          else if (the_addr_array[i].get_type() == AF_INET6)
            ++num_ipv6;
#endif /* ACE_HAS_IPV6 */
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("\t%s\n"),
                    ACE_TEXT_CHAR_TO_TCHAR (the_addr_array[i].get_host_addr ())));
        }

      delete [] the_addr_array;

#if defined (ACE_HAS_IPV6)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT (" there are %d IPv4 interfaces, ")
                  ACE_TEXT ("and %d IPv6 interfaces\n"),
                  num_ipv4, num_ipv6));
      if (num_ipv6 == 0)
        {
          rc = 1;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT (" Error: At least 1 ipv6 interface ")
                      ACE_TEXT ("was expected\n")));
        }
#endif /* ACE_HAS_IPV6  */
    }

  ACE_END_TEST;
  return rc != 0;       // return 1 if get_ip_interfaces() failed
}
