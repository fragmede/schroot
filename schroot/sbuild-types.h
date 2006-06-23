/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#ifndef SBUILD_TYPES_H
#define SBUILD_TYPES_H

#include <ctime>
#include <ios>
#include <locale>
#include <string>
#include <vector>

namespace sbuild
{

  /// A string vector.
  typedef std::vector<std::string> string_list;

  /// A date representation.
  class date
  {
  public:
    date (time_t unix_time):
      unix_time(unix_time)
    {}

    ~date ()
    {}

    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
		 date const&                       dt)
    {
      std::ios_base::iostate err = std::ios_base::goodbit;

      std::tm dtm;
      if (gmtime_r(&dt.unix_time, &dtm) == 0)
	{
	  err = std::ios_base::badbit;
	}
      else
	{
	  try
	    {
	      typename std::basic_ostream<charT, traits>::sentry sentry(stream);
	      if (sentry)
		{
		  const char nfmt[] = "%d %B %Y";
		  charT wfmt[sizeof(nfmt)/sizeof(nfmt[0])];
		  std::use_facet<std::ctype<charT> >(stream.getloc())
		    .widen(nfmt, nfmt + (sizeof(nfmt)/sizeof(nfmt[0])) - 1, wfmt);

		  typedef std::time_put<charT,std::ostreambuf_iterator<charT,traits> >
		    time_type;
		  if (std::use_facet<time_type>(stream.getloc())
		      .put(stream, stream, stream.fill(),
			   &dtm, wfmt + 0, wfmt + sizeof(wfmt)/sizeof(wfmt[0]) - 1)
		      .failed())
		    {
		      err = std::ios_base::badbit;
		    }
		  stream.width(0);
		}
	    }
	  catch (...)
	    {
	      bool flag = false;
	      try
		{
		  stream.setstate(std::ios::failbit);
		}
	      catch (std::ios_base::failure)
		{
		  flag = true;
		}
	      if (flag)
		throw;
	    }
	}

      if (err)
	stream.setstate(err);

      return stream;
    }

  private:
    time_t unix_time;
  };

}

#endif /* SBUILD_TYPES_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
