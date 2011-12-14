#ifndef DEJAGNU_H
#define DEJAGNU_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

static const char *outstate_list[] = {
  "FAILED: ", "PASSED: ", "UNTESTED: ", "UNRESOLVED: ", "XFAILED: ", "XPASSED: "
};

static const char ** outstate = outstate_list;

enum teststate { FAILED, PASSED, UNTESTED, UNRESOLVED, XFAILED, XPASSED};

class TestState {
 private:
  teststate laststate;
  std::string lastmsg;
    int passed, failed, untest, xpassed, xfailed, unresolve;
 public:
  TestState (void)
    {
      passed = 0;
      failed = 0;
      untest = 0;
      xpassed = 0;
      xfailed = 0;
      unresolve = 0;
    }

  bool had_errors() const { return (failed > 0 || xpassed > 0); }
  ~TestState (void) { totals(); }

  void operator()( bool b, std::string s ) { testrun(b,s); }
  void operator()( bool b ) { testrun(b, "Anonymous test"); }
  template <typename Foo, typename Bar>
  void equals( const Foo& f, const Bar& b, std::string s = "" ) { testrun(f == b,s); }

  void testrun (bool b, std::string s)
    {
      if (b)
	pass (s);
      else
	fail (s);
    }

    void pass (std::string s)
      {
        passed++;
        laststate = PASSED;
        lastmsg = s;
        //std::cout << "\t" << outstate[PASSED] << s << std::endl;
      }

    void pass (const char *c)
      {
	std::string s = c;
        pass (s);
      }

    void xpass (std::string s)
      {
        xpassed++;
        laststate = PASSED;
        lastmsg = s;
        std::cout << "\t" << outstate[XPASSED] << s << std::endl;
      }

    void xpass (const char *c)
      {
	std::string s = c;
        xpass (s);
      }

    void fail (std::string s)
      {
        failed++;
        laststate = FAILED;
        lastmsg = s;
        std::cout << "\t" << outstate[FAILED] << s << std::endl;
      }

    void fail (const char *c)
      {
        std::string s = c;
        fail (s);
      }

    void xfail (std::string s)
      {
        xfailed++;
        laststate = XFAILED;
        lastmsg = s;
        std::cout << "\t" << outstate[XFAILED] << s << std::endl;
      }

    void xfail (const char *c)
      {
        std::string s = c;
        xfail (s);
      }

    void untested (std::string s)
      {
        untest++;
        laststate = UNTESTED;
        lastmsg = s;
        std::cout << "\t" << outstate[UNTESTED] << s << std::endl;
      }

    void untested (const char *c)
      {
        std::string s = c;
        untested (s);
      }
    
    void unresolved (std::string s)
      {
        unresolve++;
        laststate = UNRESOLVED;
        lastmsg = s;
        std::cout << "\t" << outstate[UNRESOLVED] << s << std::endl;
      }

    void unresolved (const char *c)
      {
        std::string s = c;
        unresolved (s);
      }

    void totals (void)
      {
        std::cout << "\t#passed:\t\t" << passed << std::endl;
        std::cout << "\t#real failed:\t\t" << failed << std::endl;
        if (xfailed)
	  std::cout << "\t#expected failures:\t\t" << xfailed << std::endl;
        if (xpassed)
	  std::cout << "\t#unexpected passes:\t\t" << xpassed << std::endl;
        if (untest)
	  std::cout << "\t#untested:\t\t" << untest << std::endl;
        if (unresolve)
	  std::cout << "\t#unresolved:\t\t" << unresolve << std::endl;
      }
    
    // This is so this class can be printed in an ostream.
    friend std::ostream & operator << (std::ostream &os, TestState& t)
      {
	return os << "\t" << outstate[t.laststate] << t.lastmsg ;
      }
    
    int GetState (void) { return laststate; }
    std::string GetMsg (void) { return lastmsg; }
};

#endif /* _DEJAGNU_H_ */
