/*//////////////////////////////////////////////////////////////////
////     The SKIRT project -- advanced radiative transfer       ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#ifndef MULTITHREADPARALLEL_HPP
#define MULTITHREADPARALLEL_HPP

#include "MultiParallel.hpp"

////////////////////////////////////////////////////////////////////

/** This class implements the Parallel base class interface using multiple execution threads in a
    single process. It uses the facilities offered by the MultiParallel base class. */
class MultiThreadParallel : public MultiParallel
{
    friend class ParallelFactory;       // so ParallelFactory can access our private constructor

    //============= Construction - Destruction =============

private:
    /** Constructs a MultiThreadParallel instance with the specified number of execution threads.
        The constructor is private; use the ParallelFactory::parallel() function instead. */
    explicit MultiThreadParallel(int threadCount);

public:
    /** Destructs the instance and its parallel threads. */
    ~MultiThreadParallel();

    //======================== Other Functions =======================

public:
    /** This function implements the call() interface described in the Parallel base class for the
        parallelization scheme offered by this subclass. */
    void call(std::function<void(size_t firstIndex, size_t numIndices)> target, size_t maxIndex) override;

protected:
    /** The function to do the actual work, one chunk at a time. */
    bool doSomeWork() override;

    //======================== Data Members ========================

private:
    std::function<void(size_t,size_t)> _target; // the target function to be called
    size_t _chunkSize{0};                       // the number of indices in all but the last chunk
    size_t _maxIndex{0};                        // the maximum index (i.e. limiting the last chunk)
    std::atomic<size_t> _nextIndex{0};          // the first index of the next available chunk
};

////////////////////////////////////////////////////////////////////

#endif
