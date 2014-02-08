/* Copyright © 2006-2013  Roger Leigh <rleigh@debian.org>
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

#include <gtest/gtest.h>

#include <schroot/lock.h>

#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <config.h>

class FileLockParameters
{
public:
  schroot::lock::type initial;
  schroot::lock::type establish;
  bool willthrow;

  FileLockParameters(schroot::lock::type initial,
                     schroot::lock::type establish,
                     bool               willthrow):
    initial(initial),
    establish(establish),
    willthrow(willthrow)
  {}
};

class FileLock : public ::testing::TestWithParam<FileLockParameters>
{
public:
  int fd;
  schroot::file_lock *lck;

  FileLock():
    fd(-1),
    lck(0)
  {
    // Remove test file if it exists.
    unlink(TESTDATADIR "/filelock.ex1");
  }

  virtual ~FileLock()
  {}

  void SetUp()
  {
    fd = open(TESTDATADIR "/filelock.ex1", O_RDWR|O_EXCL|O_CREAT, 0600);
    ASSERT_GE(fd, 0);

    ssize_t wsize = write(fd,
                          "This file exists in order to test "
                          "schroot::file_lock locking.\n", 61);
    ASSERT_EQ(wsize, 61);

    lck = new schroot::file_lock(fd);
    ASSERT_NE(lck, nullptr);
  }

  void TearDown()
  {
    ASSERT_NE(lck, nullptr);
    lck->unset_lock();
    delete lck;

    ASSERT_EQ(close(fd), 0);
    ASSERT_EQ(unlink(TESTDATADIR "/filelock.ex1"), 0);
  }
};

TEST_P(FileLock, Locking)
{
  const FileLockParameters& params = GetParam();

  lck->unset_lock();
  int pid = fork();
  ASSERT_GE(pid, 0);
  if (pid == 0)
    {
      try
        {
          lck->set_lock(params.initial, 1);
          // Note: can cause unexpected success if < 4.  Set to 8 to
          // allow for slow or heavily-loaded machines.
          sleep(4);
          lck->unset_lock();
        }
      catch (const std::exception& e)
        {
          try
            {
              lck->unset_lock();
            }
          catch (const std::exception& ignore)
            {
            }
          std::cerr << "Child fail: " << e.what() << std::endl;
            _exit(EXIT_FAILURE);
        }
      _exit(EXIT_SUCCESS);
    }
  else
    {
      try
        {
          sleep(2);
          lck->set_lock(params.establish, 1);

          int status;
          ASSERT_GE(waitpid(pid, &status, 0), 0);
          ASSERT_EQ(WIFEXITED(status) && WEXITSTATUS(status), 0);
          ASSERT_FALSE(params.willthrow);
        }
      catch (const std::exception& e)
        {
          int status;
          waitpid(pid, &status, 0);
          ASSERT_TRUE(params.willthrow);
        }
    }
}

FileLockParameters params[] =
  {
    FileLockParameters(schroot::lock::LOCK_NONE,      schroot::lock::LOCK_NONE,      false),
    FileLockParameters(schroot::lock::LOCK_NONE,      schroot::lock::LOCK_SHARED,    false),
    FileLockParameters(schroot::lock::LOCK_NONE,      schroot::lock::LOCK_EXCLUSIVE, false),
    FileLockParameters(schroot::lock::LOCK_SHARED,    schroot::lock::LOCK_NONE,      false),
    FileLockParameters(schroot::lock::LOCK_SHARED,    schroot::lock::LOCK_SHARED,    false),
    FileLockParameters(schroot::lock::LOCK_SHARED,    schroot::lock::LOCK_EXCLUSIVE, true),
    FileLockParameters(schroot::lock::LOCK_EXCLUSIVE, schroot::lock::LOCK_NONE,      false),
    FileLockParameters(schroot::lock::LOCK_EXCLUSIVE, schroot::lock::LOCK_SHARED,    true),
    FileLockParameters(schroot::lock::LOCK_EXCLUSIVE, schroot::lock::LOCK_EXCLUSIVE, true)
  };

INSTANTIATE_TEST_CASE_P(LockVariants, FileLock, ::testing::ValuesIn(params));
