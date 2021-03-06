// { dg-do compile }
// 1999-06-28 bkoz

// Copyright (C) 1999, 2001, 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 24.5.3 template class istreambuf_iterator

#include <sstream>
#include <iterator>
#include <testsuite_hooks.h>

void test01()
{
  using namespace std;

  // Check for required base class.
  typedef istreambuf_iterator<char> test_iterator;
  typedef char_traits<char>::off_type off_type;
  typedef iterator<input_iterator_tag, char, off_type, char*, char&> base_iterator;

  istringstream isstream("this tag");
  test_iterator  r_it(isstream);
  base_iterator* base __attribute__((unused)) = &r_it;

  // Check for required typedefs
  typedef test_iterator::value_type value_type;
  typedef test_iterator::difference_type difference_type;
  typedef test_iterator::pointer pointer;
  typedef test_iterator::reference reference;
  typedef test_iterator::iterator_category iteratory_category;

  typedef test_iterator::char_type char_type;
  typedef test_iterator::traits_type traits_type;
  typedef test_iterator::istream_type istream_type;
  typedef test_iterator::streambuf_type streambuf_type;
}
