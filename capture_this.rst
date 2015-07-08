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
Issue
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

Capturing entities by value allows the implicitly declared
closure to be copied before invoking the closure's functon.
For example, to copy the captured-by-value closure to another
NUMA region or GPU memory for subsequent execution by
processing elements *near* to that memory.
However, a lambda expression declared within a non-static
member function has is no mechanism available to implicitly capture
**\*this** by value.
This omission must be resolved to provide true lambda capture-by-value
that copies non-member entities by value *and* **\*this** by value.

-------------------------------------------
Semantics of Capturing **\*this** by value
-------------------------------------------

Lambda capture of **\*this** by value within a non-static member function is as if
- the implicitly generated closure object type were derived from the type of **\*this**,
- the closure object type were declared a **friend** of the of the type of **\*this**,
- the **\*this** object is copied into the closure object.


----------------------------------
Semantically Consistent Resolution
----------------------------------

The semantically consistent resolution is for the *capture-default* **[=]**
to capture **\*this** by value for lambda expressions within a non-static
member function.
The *capture-default* **[&]** within a non-static member function
would continue to conform to the current capture specification for **this**.


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
current standard behavior, a new capture mechanism is necessary
to provide semantically consistent capture-by-value semantics for
lambda expressions within non-status member functions.

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


The *simple-capture* **\*this** declares that **\*this**
is to be captured by value.
The *capture-default* \* declares that the default capture
is by value, including **\*this** if the lambda
expression appears within a non-static member function.
Outside of a non-static member function the *capture-default* \*
is identical to the *capture-default* =.



