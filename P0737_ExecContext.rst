===================================================================
D0737r0 : Execution Context of Execution Agents
===================================================================

:Project: ISO JTC1/SC22/WG21: Programming Language C++
:Number: D0737r0
:Date: 2017-xx-xx
:Reply-to: hcedwar@sandia.gov
:Author: H\. Carter Edwards
:Audience: SG1 Concurrency and Parallelism
:URL: https://github.com/kokkos/ISO-CPP-Papers/blob/master/P0737_ExecContext.rst


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

      - Non-copyable and non-moveable
      - Execution resource
      - Wait functions
      - Executor generator
      - Execution context destruction behavior

    - Proposed thread execution resource
    - Proposed standard async execution context and executor
    - Interest in each of the suggested potential additions
      for initial insertion into Executors TS

******************************************************************
Proposal
******************************************************************

Add this minimal ExecutionContext specification to the Executors TS.

-----------------------------------------------------
Concept / Definition
-----------------------------------------------------

A concurrency and parallelism **execution context** manages a set of 
execution agents on a set of **execution resources** of a given
**execution architecture**.
These execution agents execute work, implemented by a callable,
that is submitted to the execution context by an **executor**.
One or more types of executors may submit work to the same
execution context.
Work to an execution context is **incomplete** until it 
(1) is invoked and exits execution by return or exception 
(2) its submission for execution is cancelled.


-----------------------------------------------------
Contrast to Networking TS (N4656) Execution Context
-----------------------------------------------------

The Networking TS (N4656 / 2017-03-17) defines a
*networking execution context* as

  "Class ``execution_context`` implements an extensible, type-safe,
  polymorphic set of services, indexed by service type."

The networking TS requirements for ``execution_context``
specify a single ``execution_context::executor_type``.
This executor type has numerous work submission member functions
each with particular semantics.


Differences between the proposed parallelism and concurrency execution context
and the networking execution context include the following.

  #.  Limited to executing work, as opposed to providing unspecified services.

  #.  Is not a concrete base class from which other forms of execution contexts
      are specialized ( ``system_context`` , ``io_context`` ) are derived.

  #.  An extensible one-to-many relationship between an execution context type
      and executor types that may submit work, as opposed to a particular
      executor type with specific work submission functions.

  #.  The proposed ``async_execution_context`` could be viewed as having
      similar intent as the networking ``system_context``.
      The significant difference of is interchangeability with
      ``std::async`` usage and extensibility to other executors
      versus the prescribed networking ``system_executor``.


------------------------------------------------------------------------------
Minimal *Concept* Specification
------------------------------------------------------------------------------

.. code-block:: c++

  class ExecutionContext /* exposition only */ {
  public:
    ~ExecutionContext();

    // Not copyable or moveable
    ExecutionContext( ExecutionContext const & ) = delete ;
    ExecutionContext( ExecutionContext && ) = delete ;
    ExecutionContext & operator = ( ExecutionContext const & ) = delete ;
    ExecutionContext & operator = ( ExecutionContext && ) = delete ;

    // Execution resource
    using execution_resource_t = /* implementatin defined */

    execution_resource_t const & execution_resource() const noexcept ;

    // Executor generator
    template< class ... ExecutorProperties >
      /* exposition only */ detail::executor_t< ExecutionContext , ExecutorProperties... >
    executor( ExecutorProperties... );

    // Waiting functions:
    void wait();
    template< class Clock , class Duration >
    bool wait_until( chrono::time_point<Clock,Duration> const & );
    template< class Rep , class Period >
    bool wait_for( chrono::duration<Rep,Period> const & );
  };

  bool operator == ( ExecutionContext const & , ExecutionContext const & );
  bool operator != ( ExecutionContext const & , ExecutionContext const & );

..

Let ``EC`` be an *ExecutionContext* type.

``EC::execution_resource_t const & EC::execution_resource() const noexcept ;``

  Returns: A descriptor of the execution resource(s) utilized by this
  execution context to execute work.
  An execution architecture is denoted by the ``execution_resource_t`` type.

| ``template< class ... ExecutorProperties >``
|   ``/* exposition only */ detail::executor_t< EC , ExecutorProperties... >``
| ``EC::executor( ExecutorProperties ... p );``

  Returns:
  An executor with **\*this** execution context and
  execution properties ``p`` when the execution context
  supports these properties.
  Otherwise ``void``.
  [Note: The *detail::executor_t* is for exposition only denoting the
  expectation that an implementation will use an implementation-defined
  metafunction to determine the type of the returned executor. --end note]

.. code-block:: c++

  static_assert( ! is_same_v< void , decltype( ec.executor( p... ) )
               , "Execution context cannot generate executor for given execution properties." );

..

  Remark:
  A particular execution property may have semantic and interface implications,
  such as whether application of the exector returns a future or not
  (sometimes referred to as a two-way or one-way property).
  A particular execution property may only be a performance hint.


``void EC::wait();``

  Requires:
  Cannot be called from non-blocking work submitted to this execution context.
  [Note: Work waiting upon itself guarantees deadlock. --end note]

  Effects:
  Waits until the number of incomplete, non-blocking callables submitted
  to the execution context is observed to be zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context. --end note]


| ``template< class Clock , class Duration >``
| ``bool EC::wait_until( chrono::time_point<Clock,Duration> const & dt );``
| ``template< class Rep , class Period >``
| ``bool EC::wait_for( chrono::duration<Rep,Period> const & dt );``

  Requires:
  Cannot be called from non-blocking work submitted to this execution context.
  [Note: Work waiting upon itself can never return true. --end note]

  Returns:
  ``true`` if the number of incomplete callables is observed zero
  at any point during the call to wait.

  Effects:
  Waits at least ``dt`` for the number of incomplete, non-blocking
  callables submitted to the execution context is observed to be zero.
  [Note: The execution agent from which the wait function is called should
  *boost block* execution agents in the execution context, but may
  only poll to honor the time out.  --end note]


``EC::~EC();``

  Effects: Type dependent potential behaviors identified by
  to-be-defined *at destruction* traits.

    - ``wait()`` for all incomplete work.
    - Cancel work that is not executing and ``wait()`` for executing work.
    - Cancel work that is not executing and abort executing work.

------------------------------------------------------------------------------
Thread Execution Resource
------------------------------------------------------------------------------

A *thread* executes on a *thread execution unit* within an
*execution resource*.
Threads can concurrently make forward progress only if they execute on
different thread execution units.
Conversely, a single thread execution unit cannot
cause two or more threads to make concurrent forward progress.
[Note: A *CPU hyperthread* is a common example of 
a thread execution unit. --end note]

Hierarchical locality-topology of thread execution resources.

.. code-block:: c++

  struct thread_execution_resource_t {

    std::vector<bool> const & affinity() const noexcept ;

    int size() const noexcept ;

    thread_execution_resource_t operator[]( int i ) const noexcept ;
  };

  extern thread_execution_resource_t program_thread_execution_resource ;

..

``std::vector<bool> const & affinity() const noexcept ;``

  Returns:
  Bit vector *M* with size equal to the maximum number of
  thread execution units available in the system.
  Thread execution unit *k* is in the thread execution resource
  if-and-only-if *M[k]* is set.


``int size() const noexcept;``

  Returns:
  Number of *locality partitions* of the execution resource.


``thread_execution_resource_t operator[]( int i ) const noexcept ;``

  Requires: ``0 <= i < size()``

  Returns: *Locality partition* of an execution resource.
  Given thread execution resource ``E``
  ``E.affinity()[k]`` set and ``0 < E.size()`` then there exists
  one-and-only-one value of ``i`` such that ``E[i].affinity()[k]``
  is set.

  Remark:
  Thread execution units residing in the same locality partition
  are *more local* with respect to the memory system
  than thread execution units in disjoint partitions.
  For example, non-uniform memory access (NUMA) partitions.


``extern thread_execution_resource_t program_thread_execution_resource ;``

  Thread execution resources in which the program is permitted
  to execute threads. 
  [Note: For a Linux runtime calling
  ``progream_thread_execution_resource.affinity()``
  is equivalent to calling ``sched_getaffinity(getpid(),...)``.
  --end note]



------------------------------------------------------------------------------
Standard Async Execution Context and Executor
------------------------------------------------------------------------------

.. code-block:: c++

  namespace std {

  class async_execution_context_t {
    // conforming to ExecutionContext concept

    // Execution resource
    using execution_resource_t = thread_execution_resource_t ;

    template< class ... ExecutorProperties >
      /* exposition only */ detail::executor_t< async_execution_context_t , ExecutorProperties... >
    executor( ExecutorProperties ... p );``
  };

  class async_executor_t ; // implementation defined

  extern async_execution_context_t async_execution_context ;

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
  [Note: It is likely that
  ``async_execution_context == program_thread_execution_context``.
  --end note]


| ``template< class ... ExecutorProperties >``
|   ``/* exposition only */ detail::executor_t< async_execution_context_t , ExecutorProperties... >``
| ``async_execution_context_t::executor( ExecutorProperties ... p );``

  Returns:
  An *executor* with **\*this** *execution context* and
  execution properties ``p``.
  If ``p`` is empty, is ``std::launch::async``, or is ``std::launch::deferred``
  the *executor* type is ``async_executor_t``.

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
  auto f = std::async( std::async_execution_context.executor() , []{ std::cout << "executor way\n"} );

  // Equivalent without- and with-executor async statements with launch policy

  auto f = std::async( std::launch::deferred , []{ std::cout << "anonymous way\n"} );
  auto f = std::async( std::async_execution_context.executor( std::launch::deferred ) , []{ std::cout << "executor way\n"} );

..


******************************************************************
Potential additions, request straw poll for each
******************************************************************

  #. A mechanism to accumulate and query exceptions thrown by
     callables that were submitted by a one-way executor.

  #. A mechanism to provide a callable that is invoked to consume
     exceptions thrown by callables that were submitted by a one-way executor.

  #. A mechanism for cancelling submitted callables that have not been invoked.
     Similar intent as Networking TS ``system_executor::stop()``.

  #. A mechanism for aborting callables that are executing.

  #. A mechanism for preventing further submissions.

  #. A preferred-locality (affinity) memory space allocator

  #. Proposal to revise Networking TS execution context to align with
     parallelism and concurrency execution context.

