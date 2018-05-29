/*//////////////////////////////////////////////////////////////////
////     The SKIRT project -- advanced radiative transfer       ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#include "GeometricSource.hpp"
#include "PhotonPacket.hpp"

//////////////////////////////////////////////////////////////////////

int GeometricSource::dimension() const
{
    return _geometry->dimension();
}

//////////////////////////////////////////////////////////////////////

double GeometricSource::luminosity() const
{
    return 0;
}

//////////////////////////////////////////////////////////////////////

void GeometricSource::launch(PhotonPacket* pp, size_t historyIndex, double L) const
{
}

//////////////////////////////////////////////////////////////////////