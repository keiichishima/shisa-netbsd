// { dg-do assemble  }
// Test for scope-based hiding of functions.

void f (char *);
struct A {
  void f ();			// { dg-error "" } referred to 
};
struct B : public A {
  void g (char *);
  void h () {
    extern void g ();		// { dg-error "" } 
    f("foo");			// { dg-error "" } hidden
    g("foo");			// { dg-error "" } hidden
  }
};
