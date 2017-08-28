===================================================================
D0737r0 : Execution Context of Execution Agents
===================================================================

:Project: ISO JTC1/SC22/WG21: Programming Language C++
:Number: D0737r0
:Date: 2017-xx-xx
:Reply-to: hcedwar@sandia.gov
:Author: H\. Carter Edwards, Sandia National Laboratories
:Author: Daniel Sunderland, Sandia National Laboratories
:Author: Michael Wong, Codeplay
:Author: Thomas Rodgers
:Author: Gordon Brown, Codeplay
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
    - Interest in each of the suggested `potential additions`_
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
Work submitted to an execution context is **incomplete** until it 
(1) is invoked and exits execution by return or exception 
(2) its submission for execution is cancelled.

    Note: The *execution context* terminology used here
    and in the Networking TS (N4656) deviate from the 
    traditional *context of execution* usage that refers
    to the state of a single executing callable; *e.g.*,
    program counter, registers, stack frame.

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
      are derived; *e.g.*, ``system_context`` , ``io_context`` .

  #.  An extensible one-to-many relationship between an execution context type
      and executor types that may submit work, as opposed to a particular
      executor type with specific work submission functions.

  #.  The proposed ``async_execution_context`` could be viewed as having
      similar intent as the networking ``system_context``.
      The significant difference of is interchangeability with
      ``std::async`` usage and extensibility to other executors
      versus the prescribed networking ``system_executor``.


-----------------------------------------------------
Partitioning and Affinity Background
-----------------------------------------------------

Hardware Locality (hwloc) Software Package
------------------------------------------

The `hardware locality (hwloc) software package
<https://www.open-mpi.org/projects/hwloc/>`_
provides a portable interface for querying and managing
hardware resources, including *processing units*
on which threads execute.
The proposed thread execution resource is motivated
by a small fraction of hwloc capabilities.

SYCL for OpenCL and HSA Standards
---------------------------------

`SYCL <https://www.khronos.org/registry/SYCL/specs/sycl-1.2.pdf>`_ (based on
`OpenCL <https://www.khronos.org/registry/OpenCL/specs/opencl-2.2.pdf>`_)
provides a C++ programming model for targeting a wide range of heterogeneous
systems including multi-core CPUs, NUMA systems, embedded SoCs and discrete
accelerators.
`HSA (Heterogeneous System Architecture) <http://www.hsafoundation.com/standards/>`_
is a similar standard that provides a lower level, and lower overhead API and
set of architecture requirements.

Both of these standards represent the topology of a system with a hierarchy of
ids that remain constant throughout the execution of a program. Both also allow
users to partition the system topology to do fine-grained work execution. The
extent of the partitioning depends on the platform.

------------------------------------------------------------------------------
Minimal *Concept* Specification
------------------------------------------------------------------------------

  The proposed *parallelism and concurrency execution context*
  has minimal scope, with the intent to grow this scope as
  consensus is achieved on `potential additions`_.

.. code-block:: c++

  class ExecutionContext /* exposition only */ {
  public:

    using at_destruction = /* implementation defined */ ;

    ~ExecutionContext();

    // Not copyable or moveable
    ExecutionContext( ExecutionContext const & ) = delete ;
    ExecutionContext( ExecutionContext && ) = delete ;
    ExecutionContext & operator = ( ExecutionContext const & ) = delete ;
    ExecutionContext & operator = ( ExecutionContext && ) = delete ;

    // Execution resource
    using execution_resource_t = /* implementation defined */ ;

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
  Execution architecture is identified by the ``execution_resource_t`` type.

| ``template< class ... ExecutorProperties >``
|   ``/* exposition only */ detail::executor_t< EC , ExecutorProperties... >``
| ``EC::executor( ExecutorProperties ... p );``

  Returns:
  An executor with ``\*this`` execution context and
  execution properties ``p`` when the execution context
  supports these properties.
  Otherwise ``void``.
  [Note: The *detail::executor_t* is for exposition only denoting the
  expectation that an implementation will use an implementation-defined
  metafunction to determine the type of the returned executor. --end note]

.. code-block:: c++

  static_assert( ! is_same_v< void , decltype( ec.executor( p... ) )
               , "Execution context cannot generate executor for given executor properties." );

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
  to-be-defined ``at_destruction`` trait.


``EC::at_destruction = /* implementation defined */ ;``

  Trait specifying behavoir of the destructor with respect to
  incomplete work.  Possibilities:

    - Reject submission of new work.
    - Wait for all incomplete work to complete.
    - Cancel work that is not executing and wait for executing work.
    - Cancel work that is not executing and abort executing work.

--------------------------------------------------------------------------------
Execution Resource (see also P0761, Executors Design Document)
--------------------------------------------------------------------------------

An *execution resource* is an implementation defined
hardware and/or software facility capable of executing a
callable function object.
Different resources may offer a broad array of functionality
and semantics and exhibit different performance characteristics
of interest to the performance-conscious programmer.
For example, an implementation might expose different processor cores,
with potentially non-uniform access to memory, as separate resources
to enable programmers to reason about locality.

An execution resource can range from SIMD vector units accessible
in a single thread to an entire runtime managing a large collection of threads.

--------------------------------------------------------------------------------
Thread Execution Resource
--------------------------------------------------------------------------------
    
A *thread of execution* executes on a *processing unit* (PU) within an
*execution resource*.
*Threads of execution* can make *concurrent forward progress*
only if they execute on different processing unit.
Conversely, a single processing unit cannot
cause two or more *threads of execution* to make concurrent forward progress.
A *thread execution resource* identifies a specific set of processing units
within the system hardware.

  [Note:
  A *CPU hyperthread* is a common example of 
  a processing unit.
  In a Linux runtime a *thread execution resource* is defined by
  a ``cpu_set_t`` object and is queried through the
  ``sched_getaffinity`` function.
  --end note]

A *processing unit* or *thread execution resource* may be what
was intended by the undefined term "thread contexts" in 33.3.2.6,
"thread static members."

A *thread execution resource* may have *locality partitions*
for its set of processing units.

.. code-block:: c++

  struct thread_execution_resource_t {

    static inline constexpr size_t procset_limit = /* implementation defined */ ;

    static size_t procset_size();

    using procset_t = std::bitset< procset_limit > ;

    procset_t const & procset() const noexcept ;

    std::vector<thread_execution_resource_t> partitioning() const noexcept ;
  };

  extern thread_execution_resource_t program_thread_execution_resource ;

..


``static inline constexpr size_t procset_limit = /* implementation defined */ ;``

  *Loose* upper bound for the number of processing units
  available across system hardware supported by the library ABI.

  Note: The preferred type for ``procset_t`` is a ``bitset``
  conformal type with runtime defined length to enable an ABI
  without the ``procset_limit`` *loose* upper bound.

``static size_t procset_size();``

  Returns:
  *Tight* upper bound for the number of processing units available
  for the system hardware in which the program is running.
  ``! procset()[k]`` when ``procset_size() <= k``.

  Remark: Has the same intent as 33.2.2.6
  ``std::thread::hardware_concurrency();`` which returns
  "The number of hardware thread contexts."

``procset_t const & procset() const noexcept ;``

  Returns:
  Processing unit *k* is in the thread execution resource
  if-and-only-if *procset()[k]* is set.


``std::vector<thread_execution_resource_t> partitioning() const noexcept ;``

  Returns:
  Locality partitioning of the execution resource.
  Given thread execution resource ``E`` and
  ``0 < E.partitioning().size()`` then
  ``E.procset()[k]`` is set then there exists
  one-and-only-one locality partition ``i`` such that
  ``E[i].procset()[k]``.

  Remark:
  Processing units residing in the same locality partition
  are *more local* with respect to the memory system
  than processing units in disjoint partitions.
  For example, non-uniform memory access (NUMA) partitions.

  Note:
  Prefer ``unique_ptr<thread_execution_resource_t[]>`` ;
  however, this array type is missing a ``size()`` observer.


``extern thread_execution_resource_t program_thread_execution_resource ;``

  Thread execution resource in which the program is *permitted*
  to execute threads. 
  When a program executes it is common for the system runtime to restrict
  that program to execute on a subset of all possible processing units
  of the system hardware.

    [Note:
    For example, the linux ``taskset`` command can restrict a program to
    a specified set of processing units and the program can use
    ``sched_getaffinity(0,...)`` to query that restriction.
    The proposed ``program_thread_execution_resource.procset()``
    is intended to provide the same query mechanism.
    --end note]


------------------------------------------------------------------------------
Motivation for Standard Async Execution Context and Executor
------------------------------------------------------------------------------

Require that the **33.6.9 Function template async** 
have an equivalent execution context and executor based
mechanism for launching asynchronous work.
This exposes the currently hidden execution context and executor(s)
which the underlying runtime has implemented to enable ``std::async``.

.. code-block:: c++

  // Equivalent without- and with-executor async statements without launch policy

  auto f = std::async( []{ std::cout << "anonymous way\n"} );
  auto f = std::async( std::async_execution_context.executor() , []{ std::cout << "executor way\n"} );

  // Equivalent without- and with-executor async statements with launch policy

  auto f = std::async( std::launch::deferred , []{ std::cout << "anonymous way\n"} );
  auto f = std::async( std::async_execution_context.executor( std::launch::deferred ) , []{ std::cout << "executor way\n"} );

..


------------------------------------------------------------------------------
Wording for Standard Async Execution Context and Executor
------------------------------------------------------------------------------

.. code-block:: c++

  namespace std {

  struct async_execution_context_t {
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

``extern async_execution_context_t async_execution_context;``

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


******************************************************************
Appendices
******************************************************************

------------------------------------------------------------------------------
Potential Additions: Request straw poll for each
------------------------------------------------------------------------------

..  _`potential additions` :

  #. Extension of *33 Thread support library* for querying the
     processing unit on which an executing thread *recently* resided,
     restrict a thread to execute on a specified thread execution resource,
     query the thread execution resource restriction imposed on a thread.
     Note: By definition a program's threads are restricted to
     ``program_thread_execution_resouce()``.

  #. Add to `thread_execution_resource_t` a hardware architecture trait;
     e.g., the **hwloc** trait for *socket*, *numa*, and *core*.

  #. Current `thread_execution_resource_t` assumes static set of
     processing units with static hierarchical partitioning topology.
     A process' set of processing units and associated topology could be
     dynamic such that an executing process could adapt to changes;
     e.g., cores could dynamically go off-line and previously off-line
     cores could come back on-line.

  #. A mechanism to accumulate and query exceptions thrown by
     callables that were submitted by a one-way executor.

  #. A mechanism to provide a callable that is invoked to consume
     exceptions thrown by callables that were submitted by a one-way executor.

  #. A mechanism for preventing further submissions.
     Related to "closed" in Concurrent Queue paper P0260.

  #. A mechanism for cancelling submitted callables that have not been invoked.
     Similar intent as Networking TS ``system_executor::stop()``.

  #. A mechanism for aborting callables that are executing.
     *Included for completeness, but not currently requested or recommended.*

  #. A preferred-locality (affinity) memory space allocator

  #. Proposal to revise Networking TS execution context to align with
     parallelism and concurrency execution context.

.. Note: Boost "ASIO" library

------------------------------------------------------------------------------
Related preferences requiring separate papers
------------------------------------------------------------------------------

  #. ``std::bitset<>`` with runtime length when length is omitted.
     Similar to the proposed ``dynamic_bitset`` of
     `N2050 <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2050>`_ ;
     however, not resizeable after construction.

  #. ``std::unique_ptr<T[],D>::size()`` method to query runtime array length.


