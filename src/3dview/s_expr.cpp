#include <ostream>
#include <istream>
#include <limits>
#include <cassert>
#include <cctype>

#include <functional>
#include <array>

#include "s_expr.hpp"

namespace
{

static std::istream& skip_spaces (std::istream& in)
{
  while (in.good () && std::isspace (in.peek ()))
    in.get ();
  return in;
}

}


s_expr s_expr::operator () (const std::string& key) const
{
  s_expr result;

  for (const_iterator i = begin (); i != end (); ++i)
     if (!i->symbol () && !i->args().empty() && i->args ().front ().name () == key)
      result.m_args.push_back (*i);

  return result;
}

std::ostream& s_expr::print (std::ostream& out, int indent) const
{
  if (symbol ())
//    out << "[S]" << m_name << ' ';
    out << m_name << ' ';

  else
  {
    out << '\n' << std::string (indent, ' ') << '(';

//    out << '\n' << std::string (indent, ' ')
//        << "([" << m_args.size () << "] ";

    for (std::vector<s_expr>::const_iterator i = m_args.begin ();
         i != m_args.end (); ++i)
      i->print (out, indent + 2);

      out << '\n' << std::string (indent, ' ') << ") ";
  }
  return out;
}

std::istream& s_expr::parse (std::istream& in)
{
  skip_spaces (in);
  std::string cur_symbol;
  int in_string = 0;

  while (true)
  {
    char c;
    in >> std::noskipws >> c;
    if (!in.good ())
      break;

    // skip comment lines
    if (c == ';' && in_string == 0)
    {
      in.ignore (std::numeric_limits<std::streamsize>::max (), '\n');
      continue;
    }

    if (c == '"' && in_string > 0)
    {
      in_string --;
//      cur_symbol += c;
      continue;
    }

    if (c == '"' && in_string == 0)
    {
      in_string ++;
//      cur_symbol += c;
      continue;
    }

    if (in_string)
    {
      assert (in_string >= 0);
      cur_symbol += c;
      continue;
    }

    if ((c == '(' || c == ')' || isspace (c)) && !cur_symbol.empty ())
    {
      bool add = true;
      if (m_hook != NULL)
        add = m_hook->top ()->on_s_expr_symbol (cur_symbol, m_hook);

      if (add)
        m_args.push_back (s_expr (cur_symbol, m_hook));

      cur_symbol.clear ();
    }

    if (c == '(')
    {
      static s_expr empty_expr;

      if (m_hook != NULL)
        m_hook->top ()->on_s_expr_begin (m_args.empty () ? empty_expr : m_args.front (), m_hook);

      m_args.push_back (s_expr (std::string (), m_hook));
      m_args.back ().parse (in);

      if (m_args.back ().m_annulled)
        m_args.pop_back ();

      skip_spaces (in);
      continue;
    }

    if (c == ')')
    {
      if (m_hook != NULL)
        if (! m_hook->top ()->on_s_expr_end (*this, m_hook))
          this->m_annulled = true;

      break;
    }

    if (!isspace (c))
      cur_symbol += c;
    else
      skip_spaces (in);
  }

  return in;
}

s_expr s_expr::make (const s_expr& a)
{
   std::array<std::reference_wrapper<const s_expr>, 1> x =
   { {
      std::reference_wrapper<const s_expr> (a)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b)
{
   std::array<std::reference_wrapper<const s_expr>, 2> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c)
{
   std::array<std::reference_wrapper<const s_expr>, 3> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d)
{
   std::array<std::reference_wrapper<const s_expr>, 4> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e)
{
   std::array<std::reference_wrapper<const s_expr>, 5> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f)
{
   std::array<std::reference_wrapper<const s_expr>, 6> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g)
{
   std::array<std::reference_wrapper<const s_expr>, 7> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h)
{
   std::array<std::reference_wrapper<const s_expr>, 8> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i)
{
   std::array<std::reference_wrapper<const s_expr>, 9> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h),
      std::reference_wrapper<const s_expr> (i)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j)
{
   std::array<std::reference_wrapper<const s_expr>, 10> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h),
      std::reference_wrapper<const s_expr> (i),
      std::reference_wrapper<const s_expr> (j)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k)
{
   std::array<std::reference_wrapper<const s_expr>, 11> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h),
      std::reference_wrapper<const s_expr> (i),
      std::reference_wrapper<const s_expr> (j),
      std::reference_wrapper<const s_expr> (k)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k, const s_expr& l)
{
   std::array<std::reference_wrapper<const s_expr>, 12> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h),
      std::reference_wrapper<const s_expr> (i),
      std::reference_wrapper<const s_expr> (j),
      std::reference_wrapper<const s_expr> (k),
      std::reference_wrapper<const s_expr> (l)
   } };
   return s_expr (x.begin (), x.end ());
}

s_expr s_expr::make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k, const s_expr& l, const s_expr& m)
{
   std::array<std::reference_wrapper<const s_expr>, 13> x =
   { {
      std::reference_wrapper<const s_expr> (a),
      std::reference_wrapper<const s_expr> (b),
      std::reference_wrapper<const s_expr> (c),
      std::reference_wrapper<const s_expr> (d),
      std::reference_wrapper<const s_expr> (e),
      std::reference_wrapper<const s_expr> (f),
      std::reference_wrapper<const s_expr> (g),
      std::reference_wrapper<const s_expr> (h),
      std::reference_wrapper<const s_expr> (i),
      std::reference_wrapper<const s_expr> (j),
      std::reference_wrapper<const s_expr> (k),
      std::reference_wrapper<const s_expr> (l),
      std::reference_wrapper<const s_expr> (m)
   } };
   return s_expr (x.begin (), x.end ());
}
