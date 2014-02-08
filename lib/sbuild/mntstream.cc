/* Copyright © 2003-2013  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include <sbuild/mntstream.h>

#include <cerrno>
#include <cstring>

#ifdef HAVE_FSTAB_FUNCTIONS
#include <fstab.h>
#elif HAVE_MNTENT_FUNCTIONS
#include <mntent.h>
#endif

namespace sbuild
{

  namespace
  {

#ifdef HAVE_FSTAB_FUNCTIONS
    mntstream::mntentry
    create_mntentry (const struct fstab& entry)
    {
      mntstream::mntentry newentry;
      newentry.filesystem_name = entry.fs_spec;
      newentry.directory = entry.fs_file;
      newentry.type = entry.fs_vfstype;
      newentry.options = entry.fs_mntops;
      newentry.dump_frequency = entry.fs_freq;
      newentry.fsck_pass = entry.fs_passno;
      return newentry;
    }
#elif HAVE_MNTENT_FUNCTIONS
    mntstream::mntentry
    create_mntentry (const struct mntent& entry)
    {
      mntstream::mntentry newentry;
      newentry.filesystem_name = entry.mnt_fsname;
      newentry.directory = entry.mnt_dir;
      newentry.type = entry.mnt_type;
      newentry.options = entry.mnt_opts;
      newentry.dump_frequency = entry.mnt_freq;
      newentry.fsck_pass = entry.mnt_passno;
      return newentry;
    }
#endif

  }

  template<>
  error<mntstream::error_code>::map_type
  error<mntstream::error_code>::error_strings =
    {
      // TRANSLATORS: %1% = mount file name
      {mntstream::MNT_OPEN,    N_("Failed to open mount file ‘%1%’")},
      // TRANSLATORS: %1% = mount file name
      {mntstream::MNT_READ,    N_("Failed to read mount file ‘%1%’")}
    };

  mntstream::mntstream(const std::string& file):
    file(),
    mntfile(0),
    data(),
    error_status(true),
    eof_status(true)
  {
    open(file);
  }


  mntstream::~mntstream()
  {
    close();
  }

  void
  mntstream::open(const std::string& file)
  {
#ifdef HAVE_FSTAB_FUNCTIONS
    this->mntfile = 0;
    setfstab(file.c_str());
#elif HAVE_MNTENT_FUNCTIONS
    this->mntfile = setmntent(file.c_str(), "r");
    if (this->mntfile == 0)
      {
        this->file.clear();
        this->error_status = true;
        this->eof_status = true;
        throw error(file, MNT_OPEN, strerror(errno));
      }
#endif
    this->file = file;
    this->error_status = false;
    this->eof_status = false;
    read();
  }

  void
  mntstream::read(int quantity)
  {
    int i;

#if HAVE_MNTENT_FUNCTIONS
    if (this->mntfile == 0)
      return;
#endif

    for (i = 0; i < quantity; ++i)
      {
        errno = 0;
#ifdef HAVE_FSTAB_FUNCTIONS
        struct fstab* entry;
        entry = getfsent();
#elif HAVE_MNTENT_FUNCTIONS
        struct mntent* entry;
        entry = getmntent(mntfile);
#endif

        if (entry == 0) // EOF or error
          {
            //std::cerr << "Mount file read error: ";
            if (errno) // error
              {
                this->error_status = true;
                throw error(this->file, MNT_READ, strerror(errno));
              }
            return;
          }

        mntentry newentry(create_mntentry(*entry)); // make a mntentry
        this->data.push_back(newentry); // push onto the end of the list
      }
  }

  void
  mntstream::close()
  {
    // don't throw an exception on failure -- it could be called in
    // the destructor
#ifdef HAVE_FSTAB_FUNCTIONS
    endfsent();
#elif HAVE_MNTENT_FUNCTIONS
    if (this->mntfile)
      endmntent(this->mntfile);
#endif
    this->mntfile = 0;
    this->data.clear();    // clear all data
    this->file.clear();
    this->error_status = true;
    this->eof_status = true;
  }


  bool
  mntstream::eof() const
  {
    return this->eof_status;
  }

  bool
  mntstream::bad() const
  {
    return this->error_status;
  }

  mntstream::operator bool ()
  {
    return !(bad() || eof());
  }

  bool
  mntstream::operator ! ()
  {
    return bad() || eof();
  }


  mntstream&
  operator >> (mntstream&            stream,
               mntstream::mntentry&  entry)
  {
    stream.read(); // read a new entry
    if (stream && !stream.data.empty()) // not at end of file or bad.
      {
        entry = stream.data.front(); // assign next mntentry to entry
        stream.data.pop_front(); // remove the entry
      }
    else // blank the mntentry and set EOF status
      {
        entry = mntstream::mntentry();
        stream.eof_status = true;
      }

    return stream;
  }

}
