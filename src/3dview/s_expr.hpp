#ifndef includeguard_s_expr_hpp_includeguard
#define includeguard_s_expr_hpp_includeguard

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iosfwd>
#include <stack>
#include <cstdlib>

namespace s_expr_impl_detail
{
template <typename T> struct from_string
{
  static T get (const std::string& s)
  {
     T r;
     std::stringstream ss (s);
     ss >> std::boolalpha >> r;
     if (ss.fail ())
     {
       throw std::logic_error ("from_string NG");
     }
     return r;
  }
};

template<> struct from_string<std::string>
{
  static const std::string& get (const std::string& s)
  {
    return s;
  }
};

// ATTENTION HACK
// this is for speeding up float parsing for loading vertex data.
// if there are errors, it won't throw exceptions.
template<> struct from_string<float>
{
  static float get (const std::string& s)
  {
    return (float)std::atof (s.c_str ());
  }
};
} // namespace s_expr_impl_detail

class s_expr
{
public:
  struct parser_hook
  {
    virtual bool on_s_expr_symbol (const std::string&, std::stack<parser_hook*>*) { return true; }
    virtual bool on_s_expr_begin (const s_expr& expr, std::stack<parser_hook*>*) { return true; }
    virtual bool on_s_expr_end (const s_expr& expr, std::stack<parser_hook*>*) { return true; }
  };

  typedef std::vector<s_expr>::iterator iterator;
  typedef std::vector<s_expr>::const_iterator const_iterator;

  s_expr (std::stack<parser_hook*>* h = NULL) : m_annulled (false), m_hook (h) { m_args.reserve (32); }
  s_expr (const std::string& name, std::stack<parser_hook*>* h = NULL) : m_annulled (false), m_hook (h), m_name (name) { m_args.reserve (32); }
  s_expr (const char* name, std::stack<parser_hook*>* h = NULL) : m_annulled (false), m_hook (h), m_name (name) { m_args.reserve (32); }

  template <typename InputIter> s_expr (InputIter begin, InputIter end)
  : m_args (begin, end) { }

  bool symbol (void) const { return m_args.empty (); }
  bool empty (void) const { return m_name.empty () && m_args.empty (); }
  const std::string& name (void) const { return m_name; }
  const std::vector<s_expr>& args (void) const { return m_args; }

  iterator begin (void) { return m_args.begin (); }
  iterator end (void) { return m_args.end (); }
  const_iterator begin (void) const { return m_args.begin (); }
  const_iterator end (void) const { return m_args.end (); }
  const_iterator cbegin (void) const { return m_args.begin (); }
  const_iterator cend (void) const { return m_args.end (); }

  friend std::istream& operator >> (std::istream& in, s_expr& out)
  {
    return out.parse (in);
  }

  friend std::ostream& operator << (std::ostream& out, const s_expr& in)
  {
    return in.print (out);
  }

  // convenience accessor to a key-value list '(key value)'
  const s_expr& value (void) const { return m_args.at (1); }

  // access i-th arg in the list.
  const s_expr& operator () (size_t i) const { return m_args.at (i); }

  // find the lists that have the first arg equal to key
  s_expr operator () (const std::string& key) const;

  template <typename T> T as (void) const
  {
    if (!symbol ())
      throw std::logic_error ("expr not a symbol");

    return s_expr_impl_detail::from_string<T>::get (m_name);
  }

   // variadic templates help in situations like this.
   static s_expr make (const s_expr& a);
   static s_expr make (const s_expr& a, const s_expr& b);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k, const s_expr& l);
   static s_expr make (const s_expr& a, const s_expr& b, const s_expr& c, const s_expr& d, const s_expr& e, const s_expr& f, const s_expr& g, const s_expr& h, const s_expr& i, const s_expr& j, const s_expr& k, const s_expr& l, const s_expr& m);

private:
  bool m_annulled;
  std::stack<parser_hook*>* m_hook;
  std::string m_name;
  std::vector<s_expr> m_args;

  std::ostream& print (std::ostream& out, int indent = 0) const;
  std::istream& parse (std::istream& in);
};



#endif // includeguard_s_expr_hpp_includeguard
