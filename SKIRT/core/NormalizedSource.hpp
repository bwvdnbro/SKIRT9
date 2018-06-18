/*//////////////////////////////////////////////////////////////////
////     The SKIRT project -- advanced radiative transfer       ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#ifndef NORMALIZEDSOURCE_HPP
#define NORMALIZEDSOURCE_HPP

#include "Source.hpp"
#include "LuminosityNormalization.hpp"
#include "SED.hpp"
class RedshiftInterface;

//////////////////////////////////////////////////////////////////////

/** NormalizedSource is an abstract class representing a primary radiation source characterized by
    a single SED object, i.e. the spectral distribution is identical in all spatial locations. The
    source can have a single bulk velocity, i.e. the bulk velocity is also identical in all
    locations. The bolometric power of the source is characterized by a LuminosityNormalization
    object.

    Subclasses must handle the spatial distribution of the source, and can optionally add
    anisotropy and/or polarization. */
class NormalizedSource : public Source
{
    ITEM_ABSTRACT(NormalizedSource, Source, "a primary source with a single SED")

    PROPERTY_ITEM(sed, SED, "the spectral energy distribution for the source")
        ATTRIBUTE_DEFAULT_VALUE(sed, "SunSED")

    PROPERTY_ITEM(normalization, LuminosityNormalization, "the type of luminosity normalization for the source")
        ATTRIBUTE_DEFAULT_VALUE(normalization, "IntegratedLuminosityNormalization")

    ATTRIBUTE_SUB_PROPERTIES_HERE()

    PROPERTY_DOUBLE(velocityX, "the bulk velocity of the source, x component")
        ATTRIBUTE_QUANTITY(velocityX, "velocity")
        ATTRIBUTE_MIN_VALUE(velocityX, "[0")
        ATTRIBUTE_MAX_VALUE(velocityX, "100000 km/s]")
        ATTRIBUTE_DEFAULT_VALUE(velocityX, "0")

    PROPERTY_DOUBLE(velocityY, "the bulk velocity of the source, y component")
        ATTRIBUTE_QUANTITY(velocityY, "velocity")
        ATTRIBUTE_MIN_VALUE(velocityY, "[0")
        ATTRIBUTE_MAX_VALUE(velocityY, "100000 km/s]")
        ATTRIBUTE_DEFAULT_VALUE(velocityY, "0")

    PROPERTY_DOUBLE(velocityZ, "the bulk velocity of the source, z component")
        ATTRIBUTE_QUANTITY(velocityZ, "velocity")
        ATTRIBUTE_MIN_VALUE(velocityZ, "[0")
        ATTRIBUTE_MAX_VALUE(velocityZ, "100000 km/s]")
        ATTRIBUTE_DEFAULT_VALUE(velocityZ, "0")

    ITEM_END()

    //============= Construction - Setup - Destruction =============

protected:
    /** This function creates a private object offering the redshift interface if the bulk velocity
        is nonzero. */
    void setupSelfBefore() override;

    /** The destructor deletes the private object offering the redshift interface. */
    ~NormalizedSource();

    //======================== Other Functions =======================

public:
    /** This function returns the dimension of the source, which is the same as the dimension of
        its spatial distribition (to be provided by the subclass), except if there is a nonzero
        bulk velocity. */
    int dimension() const override;

    /** This function returns the luminosity \f$L\f$ (i.e. radiative power) of the source
        integrated over the wavelength range of primary sources (configured for the source system
        as a whole) and across its complete spatial domain. */
     double luminosity() const override;

     /** This function returns the specific luminosity \f$L_\lambda\f$ (i.e. radiative power per
         unit of wavelength) of the source at the specified wavelength, or zero if the wavelength is
         outside the wavelength range of primary sources (configured for the source system as a
         whole) or if the source simply does not emit at the wavelength. */
     double specificLuminosity(double wavelength) const override;

     /** This function causes the photon packet \em pp to be launched from the source using the
         given history index and luminosity contribution. In this abstract class, the function
         handles the wavelength sampling and normalization, relying on the subclass to determine
         the position and propagation direction of the emission from the geometry of the source. */
    void launch(PhotonPacket* pp, size_t historyIndex, double L) const override;

    //============== Functions to be implemented in each subclass =============

    /** This function returns the dimension of the spatial distribition implemented by the
        subclass, taking into account anisotropic emission or polarization, if any. It must be
implemented in a subclass. */
    virtual int geometryDimension() const = 0;

    /** This function causes the photon packet \em pp to be launched from the source using the
        given history index, wavelength, weighted luminosity contribution, and redshift interface
        (corresponding to the bulk velocity of the source). It must be implemented in a subclass to
        handle the spatial distribution of the source, optionally adding anisotropy and/or
        polarization. */
    virtual void launchNormalized(PhotonPacket* pp, size_t historyIndex, double lambda, double Lw,
                                  RedshiftInterface* rsi) const = 0;

    //======================== Data Members ========================

private:
    // pointer to object offering the redshift interface, or null pointer if the bulk velocity is zero
    RedshiftInterface* _bulkvelocity{nullptr};
};

//////////////////////////////////////////////////////////////////////

#endif