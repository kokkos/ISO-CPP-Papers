===================================================================
PXXXXr01 ``Lambda Capture *this by Value``
===================================================================

:Author: H\. Carter Edwards
:Contact: hcedwar@sandia.gov
:Author: Christian Trott
:Contact: crtrott@sandia.gov
:Date: 2015-07-07
:Version: 01
:URL: https://github.com/kokkos/ISO-CPP-Papers/blob/master/capture_this.rst

.. sectnum::

---------
Rationale
---------

Lambda expressions declared within a non-static member function explicilty
or implicitly captures **this** to access to member variables of **this**.
Both capture-by-reference **[&]** and capture-by-value **[=]** implicitly
capture the **this** pointer, therefore member variables are always accessed
by reference via **this**.
Thus the capture-default has no effect on the capture of **this**.

.. code-block:: c++

  struct S {
    int x ;
    void f() {
      auto a = [&]() { x = 42 ; } // OK: transformed to (*this).x
      auto b = [=]() { x = 43 ; } // OK: transformed to (*this).x
      a(); // x == 42
      b(); // x == 43
    }
  };

.. /*

Capturing entities by value allows the implicitly declared closure to be
copied before invoking the closure's functon.  For example, to copy the
captured-by-value closure to another NUMA region or GPU memory for
subsequent execution by processing elements *near* to that memory.
However, when a lambda expression is declared within a non-static
member function their is no mechanism available to implicitly capture
**\*this** by value.

A lambda capture mechanism is required to capture **\*this** by value;
i.e., all member data of **\*this** are copied by value into the closure.
Such a mechanism allows a capture-by-value closure to be copied and
all captured variables to accessed from the copied location.

----------------------------------
Semantically Consistent Resolution
----------------------------------

Capture-default by value **[=]** within a non-static member function of a
containing class implicitly declares a closure object type as if that
type were derived from and a friend of the containing class.
Capture by value then copies the containing class into the closure object
such that all of the member objects are effectively captured by value.

Capture-default by reference **[&]** within a non-static member function of a
containing class continues to conform to the current specification for
capture of **this**.


.. code-block:: c++

  struct S {
    int x ;
    void f() {
      auto a = [&]() { x = 42 ; } // OK: transformed to (*this).x
      auto b = [=]() { x = 42 ; }
        // Error: captured copy of '*this'
        // and lambda function is 'const'
    }
  };

.. /*

--------------------------------------
Resolution to Patch the Specification
--------------------------------------

Given that the semantically consistent resolution would break
current standard behavior a new capture mechanism is necessary
to enable capture of **\*this** by value.

Extent the *capture-default* and *simple-capture* to include:
|  *capture-default*:
|     &
|     =
|     *
|  *simple-capture*:
|    *identifier*
|    & *identifier*
|    **this**
|    **\*this**


The *simple-capture* **\*this** declares that the containing
class is to be captured by value.
The *capture-default* \* declares that the default capture
is by value, including the containing class if the lambda
expression appears within a non-static member function.
Outside of a non-static member function the \* *capture-default*
is identical to the = *capture-default*.



