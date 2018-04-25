/*//////////////////////////////////////////////////////////////////
////     The SKIRT project -- advanced radiative transfer       ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#include "ProcessManager.hpp"
#include "FatalError.hpp"

#ifdef BUILD_WITH_MPI
#include <mpi.h>
#endif

////////////////////////////////////////////////////////////////////

int ProcessManager::_size{1};    // the number of processes: initialize to non-MPI default value
int ProcessManager::_rank{0};    // the rank of this process: initialize to non-MPI default value

////////////////////////////////////////////////////////////////////

#ifdef BUILD_WITH_MPI
namespace
{
    // The MPI interfaces specify the number of elements as a signed integer.
    // Therefore, when a very large array is to be communicated,
    // the message will be broken up into pieces of the following size
    const size_t maxMessageSize = INT_MAX - 2;
}
#endif

//////////////////////////////////////////////////////////////////////

void ProcessManager::initialize(int *argc, char ***argv)
{
#ifdef BUILD_WITH_MPI
    int initialized;
    MPI_Initialized(&initialized);
    if (!initialized)
    {
        // initialize MPI and verify that the implementation supports running multiple threads, as long
        // as we're calling MPI only from the main thread; this should avoid busy waits when blocking
        int provided = 0;
        MPI_Init_thread(argc, argv, MPI_THREAD_FUNNELED, &provided);
        if (provided < MPI_THREAD_FUNNELED)
            throw FATALERROR("MPI implementation does not support funneled threads");

        // get the process group size and our rank
        MPI_Comm_size(MPI_COMM_WORLD, &_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
    }
#else
    // the size and rank are statically initialized to the appropriate values
    (void)argc; (void)argv;
#endif
}

//////////////////////////////////////////////////////////////////////

void ProcessManager::finalize()
{
#ifdef BUILD_WITH_MPI
    MPI_Finalize();
#endif
}

//////////////////////////////////////////////////////////////////////

void ProcessManager::wait()
{
#ifdef BUILD_WITH_MPI
    if (isMultiProc()) MPI_Barrier(MPI_COMM_WORLD);
#endif
}

//////////////////////////////////////////////////////////////////////

void ProcessManager::sumToAll(Array& arr)
{
#ifdef BUILD_WITH_MPI
    if (isMultiProc())
    {
        double* data = begin(arr);
        size_t remaining = arr.size();
        while (remaining > maxMessageSize)
        {
            MPI_Allreduce(MPI_IN_PLACE, data, maxMessageSize, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            data += maxMessageSize;
            remaining -= maxMessageSize;
        }
        MPI_Allreduce(MPI_IN_PLACE, data, remaining, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    }
#else
    (void)arr;
#endif
}

//////////////////////////////////////////////////////////////////////

void ProcessManager::sumToRoot(Array& arr)
{
#ifdef BUILD_WITH_MPI
    if (isMultiProc())
    {
        double* data = begin(arr);
        size_t remaining = arr.size();

        while (remaining > maxMessageSize)
        {
            if (isRoot())
                MPI_Reduce(MPI_IN_PLACE, data, maxMessageSize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            else
                MPI_Reduce(data, data, maxMessageSize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

            remaining -= maxMessageSize;
            data += maxMessageSize;
        }
        if (isRoot())
            MPI_Reduce(MPI_IN_PLACE, data, remaining, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        else
            MPI_Reduce(data, data, remaining, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
#else
    (void)arr;
#endif
}

//////////////////////////////////////////////////////////////////////
