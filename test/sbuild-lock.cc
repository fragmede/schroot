/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
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

#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>

#include <lockdev.h>

#include <schroot/sbuild-lock.h>

using namespace CppUnit;

class test_file_lock : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_file_lock);
  CPPUNIT_TEST(test_none_none_lock);
  CPPUNIT_TEST(test_none_shr_lock);
  CPPUNIT_TEST(test_none_excl_lock);
  CPPUNIT_TEST(test_shr_none_lock);
  CPPUNIT_TEST(test_shr_shr_lock);
  CPPUNIT_TEST_EXCEPTION(test_shr_excl_lock, sbuild::Lock::error);
  CPPUNIT_TEST(test_excl_none_lock);
  CPPUNIT_TEST_EXCEPTION(test_excl_shr_lock, sbuild::Lock::error);
  CPPUNIT_TEST_EXCEPTION(test_excl_excl_lock, sbuild::Lock::error);
  CPPUNIT_TEST_SUITE_END();

protected:
  int fd;
  sbuild::FileLock *lck;

public:
  test_file_lock():
    TestFixture(),
    fd(-1),
    lck()
  {
    // Remove test file if it exists.
    unlink(SRCDIR "/filelock.ex1");
  }

  virtual ~test_file_lock()
  {}

  void setUp()
  {
    int fd = open(SRCDIR "/filelock.ex1", O_RDWR|O_EXCL|O_CREAT, 0600);
    CPPUNIT_ASSERT(write(fd,
			 "This file exists in order to test "
			 "sbuild::FileLock locking.\n", 60) == 60);
    CPPUNIT_ASSERT(fd >= 0);
    this->lck = new sbuild::FileLock(fd);
  }

  void tearDown()
  {
    this->lck->unset_lock();
    delete this->lck;
    CPPUNIT_ASSERT(close(fd) < 0);
    CPPUNIT_ASSERT(unlink(SRCDIR "/filelock.ex1") == 0);
  }

  void test(sbuild::Lock::Type initial,
	    sbuild::Lock::Type establish)
  {
    this->lck->unset_lock();
    int pid = fork();
    CPPUNIT_ASSERT(pid >= 0);
    if (pid == 0)
      {
	try
	  {
	    this->lck->set_lock(initial, 1);
	    // Note: can cause unexpected success if < 4.  Set to 8 to
	    // allow for slow or heavily-loaded machines.
	    sleep(8);
	    this->lck->unset_lock();
	  }
	catch (std::exception const& e)
	  {
	    try
	      {
		this->lck->unset_lock();
	      }
	    catch (std::exception const& ignore)
	      {
	      }
	    std::cerr <<"Child fail: " << e.what() << std::endl;
	    _exit(EXIT_FAILURE);
	  }
	_exit(EXIT_SUCCESS);
      }
    else
      {
	try
	  {
	    sleep(2);
	    this->lck->set_lock(establish, 1);

	    int status;
	    CPPUNIT_ASSERT(waitpid(pid, &status, 0) >= 0);
	    CPPUNIT_ASSERT(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	  }
	catch (std::exception const& e)
	  {
	    int status;
	    waitpid(pid, &status, 0);
	    throw;
	  }
      }
  }

  void test_none_none_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_NONE);
  }

  void test_none_shr_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_SHARED);
  }

  void test_none_excl_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_EXCLUSIVE);
  }

  void test_shr_none_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_NONE);
  }

  void test_shr_shr_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_SHARED);
  }

  void test_shr_excl_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_EXCLUSIVE);
  }

  void test_excl_none_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_NONE);
  }

  void test_excl_shr_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_SHARED);
  }

  void test_excl_excl_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_EXCLUSIVE);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_file_lock);

class test_dev_lock : public TestFixture
{
  CPPUNIT_TEST_SUITE(test_dev_lock);
  CPPUNIT_TEST(test_none_none_lock);
  CPPUNIT_TEST(test_none_shr_lock);
  CPPUNIT_TEST(test_none_excl_lock);
  CPPUNIT_TEST(test_shr_none_lock);
  CPPUNIT_TEST_EXCEPTION(test_shr_shr_lock, sbuild::Lock::error);
  CPPUNIT_TEST_EXCEPTION(test_shr_excl_lock, sbuild::Lock::error);
  CPPUNIT_TEST(test_excl_none_lock);
  CPPUNIT_TEST_EXCEPTION(test_excl_shr_lock, sbuild::Lock::error);
  CPPUNIT_TEST_EXCEPTION(test_excl_excl_lock, sbuild::Lock::error);
  CPPUNIT_TEST_SUITE_END();

protected:
  int fd;
  sbuild::DeviceLock *lck;

public:
  test_dev_lock():
    TestFixture(),
    fd(-1),
    lck()
  {}

  virtual ~test_dev_lock()
  {
    unlock();
  }

  void unlock()
  {
    CPPUNIT_ASSERT(dev_unlock("/dev/null", 0) == 0);
  }

  void setUp()
  {
    unlock();
    this->lck = new sbuild::DeviceLock("/dev/null");
  }

  void tearDown()
  {
    delete this->lck;
  }

  void test(sbuild::Lock::Type initial,
	    sbuild::Lock::Type establish)
  {
    unlock();
    int pid = fork();
    CPPUNIT_ASSERT(pid >= 0);
    if (pid == 0)
      {
	try
	  {
	    this->lck->set_lock(initial, 1);
	    // Note: can cause unexpected success if < 4.  Set to 8 to
	    // allow for slow or heavily-loaded machines.
	    sleep(8);
	    this->lck->unset_lock();
	  }
	catch (std::exception const& e)
	  {
	    try
	      {
		this->lck->unset_lock();
	      }
	    catch (std::exception const& ignore)
	      {
	      }
	    std::cerr <<"Child fail: " << e.what() << std::endl;
	    _exit(EXIT_FAILURE);
	  }
	_exit(EXIT_SUCCESS);
      }
    else
      {
	try
	  {
	    sleep(2);
	    this->lck->set_lock(establish, 1);
	    this->lck->unset_lock();

	    int status;
	    CPPUNIT_ASSERT(waitpid(pid, &status, 0) >= 0);
	    CPPUNIT_ASSERT(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	  }
	catch (std::exception const& e)
	  {
	    int status;
	    waitpid(pid, &status, 0);
	    throw;
	  }
      }
  }

  void test_none_none_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_NONE);
  }

  void test_none_shr_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_SHARED);
  }

  void test_none_excl_lock()
  {
    test(sbuild::Lock::LOCK_NONE, sbuild::Lock::LOCK_EXCLUSIVE);
  }

  void test_shr_none_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_NONE);
  }

  void test_shr_shr_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_SHARED);
  }

  void test_shr_excl_lock()
  {
    test(sbuild::Lock::LOCK_SHARED, sbuild::Lock::LOCK_EXCLUSIVE);
  }

  void test_excl_none_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_NONE);
  }

  void test_excl_shr_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_SHARED);
  }

  void test_excl_excl_lock()
  {
    test(sbuild::Lock::LOCK_EXCLUSIVE, sbuild::Lock::LOCK_EXCLUSIVE);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(test_dev_lock);

/*
 * Local Variables:
 * mode:C++
 * End:
 */
