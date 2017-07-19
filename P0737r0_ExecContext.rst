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
    - Proposed standard async execution context and executor
    - Proposed interface for generating executors from execution context.
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
Minimal *Concept* Specification
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
    void wait();
    template< class Clock , class Duration >
    bool wait_until( chrono::time_point<Clock,Duration> const & );
    template< class Rep , class Period >
    bool wait_for( chrono::duration<Rep,Period> const & );
  };

  bool operator == ( ExecutionContext const & , ExecutionContext const & );
  bool operator != ( ExecutionContext const & , ExecutionContext const & );

  template< class ... ExecutorProperties >
    /* exposition only */ detail::executor_t< ExecutionContext , ExecutorProperties... >
  executor( ExecutionContext /* exposition only */ , ExecutorProperties... );

..

Let ``EC`` be an *ExecutionContext* type.

``void EC::wait();``

  Effects:
  Waits until the number of incomplete callables submitted to the
  execution conect is observed to be zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context. --end note]

| ``template< class Clock , class Duration >``
| ``bool EC::wait_until( chrono::time_point<Clock,Duration> const & dt );``
| ``template< class Rep , class Period >``
| ``bool EC::wait_for( chrono::duration<Rep,Period> const & dt );``

  Returns:
  ``true`` if the number of incomplete callables is observed zero
  at any point during the call to wait.

  Effects:
  Waits at least ``dt`` for the number of incomplete
  callables submitted to the execution context to be observed zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context, but may
  only poll to honor the time out.  --end note]

| ``template< class ... ExecutorProperties >``
|   *detail::executor_t< EC , ExecutorProperties... >*
| ``executor(`` *EC* ``ec , ExecutorProperties ... p );``

  Returns:
  An *executor* with *execution context* ``ec`` and
  execution properties ``p``.
  [Note: The *detail::executor_t* is for exposition only denoting the
  expectation that an implementation will use an implementation-defined
  metafunction to determine the type of the returned executor. --end note]

  Remark:
  A particular execution property may have semantic and interface implications,
  such as whether application of the exector returns a future or not
  (sometimes referred to as a two-way or one-way property).
  A particular execution property may only be a performance hint.


------------------------------------------------------------------------------
Standard Async Execution Context and Executor
------------------------------------------------------------------------------

.. code-block:: c++

  namespace std {

  class async_execution_context_t {
    // conforming to ExecutionContext concept
    // ... and other implementation defined members
  };

  class async_executor_t ; // implementation defined

  extern async_execution_context_t async_execution_context ;

  template< class ... ExecutorProperties >
  async_executor_t
  executor( async_execution_context_t & ec , ExecutorProperties ... p );

  template< class Function , class ... Args >
  future<std::result_of<std::decay_t<Function>(std::decay_t<Args>...)>>
  async( async_executor_t exec , Function && f , Args && ... args );

  }

..

``extern async_execution_context_t async_execution_context``

  Global execution context object enabling the
  equivalent invocation of callables 
  through the with-executor ``std::async``
  and without-executor ``std::async``.
  Guaranteed to be initialized during or before the first use.


| ``template< class ... ExecutorProperties >``
| ``async_executor_t``
| ``executor( async_execution_context_t & ec , ExecutorProperties ...p );``

  Returns:
  An ``async_executor_t`` executor with execution context ``ec``
  and executor properties ``p``. 
  Executor properties ``p`` can be empty, 
  can include ``std::launch::async`` or ``std::launch::deferred``,
  and include other implementation defined launch properties.

| ``template< class Function , class ... Args >``
| ``future<std::result_of<std::decay_t<Function>(std::decay_t<Args>...)>>``
| ``async( async_executor_t exec , Function && f , Args && ... args );``

  Effects:
  If ``exec`` has a ``std::launch`` *policy*
  then equivalent to invoking ``std::async(`` *policy* ``, f , args... );``
  otherwise equivalent to invoking ``std::async( f , args... );``
  Equivalency is symmetric with respect to the non-executor ``std::async``
  functions.

.. code-block:: c++

  // Equivalent without- and with-executor async statements without launch policy

  auto f = std::async( []{ std::cout << "anonymous way\n"} );
  auto f = std::async( std::executor( async_execution_context ) , []{ std::cout << "executor way\n"} );

  // Equivalent without- and with-executor async statements with launch policy

  auto f = std::async( std::launch::deferred , []{ std::cout << "anonymous way\n"} );
  auto f = std::async( std::executor( async_execution_context , std::launch::deferred ) , []{ std::cout << "executor way\n"} );

..

******************************************************************
Potential additions, request straw poll for each
******************************************************************

  #. A mechanism to accumulate and query exceptions thrown by
     callables that were submitted by a one-way executor.

  #. A mechanism to provide a callable that is invoked to consume
     exceptions thrown by callables that were submitted by a one-way executor.

  #. A mechanism for cancelling submitted callables that have not been invoked.

  #. A mechanism for aborting callables that are executing.

  #. A mechanism for preventing further submissions.

  #. An **execution resource** concept.

  #. An **execution architecture** trait.

  #. A preferred-locality (affinity) memory space allocator


