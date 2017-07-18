===================================================================
D0737r0 : Execution Context of Execution Agents
===================================================================

:Project: ISO JTC1/SC22/WG21: Programming Language C++
:Number: D0737r0
:Date: 2017-xx-xx
:Reply-to: hcedwar@sandia.gov
:Author: H\. Carter Edwards
:Audience: SG1 Concurrency and Parallelism
:URL: https://github.com/kokkos/ISO-CPP-Papers/blob/master/Dxxxx.rst


******************************************************************
Revision History
******************************************************************

------------------------------------------------------------
D0737r0
------------------------------------------------------------

  - Initial paper for 2017-11-Albuquerque meeting
  - State: Design exploration
  - Requested straw polls:

    - Proposed definition of ExecutionContext *concept*
    - Proposed wait functions
    - Interest in each of the suggested potential additions
      for initial insertion into Executors TS

******************************************************************
Proposal
******************************************************************

Add this minimal ExecutionContext specification to the Executors TS.

-----------------------------------------------------
Concept / Definition
-----------------------------------------------------

An **execution context** manages a set of 
execution agents on a set of **execution resources**.
These execution agents executes callables submitted by an **executor**
to the execution context.
A callable submitted to an execution context is **incomplete** until it 
(1) is invoked and exits execution by return or exception 
(2) its submission for execution is cancelled.


------------------------------------------------------------------------------
Minimal Specification
------------------------------------------------------------------------------

.. code-block:: c++

  class ExecutionContext /* exposition only */ {
  public:
    // Not copyable or moveable
    ExecutionContext( ExecutionContext const & ) = delete ;
    ExecutionContext( ExecutionContext && ) = delete ;
    ExecutionContext & operator = ( ExecutionContext const & ) = delete ;
    ExecutionContext & operator = ( ExecutionContext && ) = delete ;

    // Waiting functions:
    template< class Clock , class Duration >
    bool wait_until( chrono::time_point<Clock,Duration> const & );
    template< class Rep , class Period >
    bool wait_for( chrono::duration<Rep,Period> const & );
    void wait();
  };

  bool operator == ( ExecutionContext const & , ExecutionContext const & );
  bool operator != ( ExecutionContext const & , ExecutionContext const & );

..

| ``template< class Clock , class Duration >``
| ``bool wait_until( chrono::time_point<Clock,Duration> const & dt );``
| ``template< class Rep , class Period >``
| ``bool wait_for( chrono::duration<Rep,Period> const & dt );``

  Returns:
  ``true`` if the number of incomplete callables is observed zero
  at any point during the call to wait.

  Effects:
  Waits at least ``dt`` for the number of incomplete
  callables submitted to the execution context to be observed zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context. --end note]

``void wait();``

  Effects:
  Waits until the number of incomplete callables submitted to the
  execution conect is observed to be zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context. --end note]

******************************************************************
Potential additions, request straw poll for each
******************************************************************

  1. The execution context to which ``std::async`` submits callables.

  2. Standard interface for generating executors from execution context.
     For example

    | ``template< class ... ExecutorProperties >``
    | ``executor(`` *ExecutionContext* ``, ExecutorProperties... );``
    |
    | Illustrative usage:
    |
    |   auto f =
    |     executor( std::async_execution_context, twoway, nonblocking, newthread )
    |       .execute( []{ std::cout << "Hello, I'm a std::async lambda\\n" ; } );


  3. A mechanism to accumulate and query exceptions thrown by
     callables that were submitted by a one-way executor.

  #. A mechanism to provide a callable that is invoked to consume
     exceptions thrown by callables that were submitted by a one-way executor.

  #. A mechanism for cancelling submitted callables that have not been invoked.

  #. A mechanism for aborting callables that are executing.

  #. A mechanism for preventing further submissions.

  #. An **execution resource** concept.

  #. An **execution architecture** trait.

  #. A preferred-locality (affinity) memory space allocator


