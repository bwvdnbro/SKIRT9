/*//////////////////////////////////////////////////////////////////
////     The SKIRT project -- advanced radiative transfer       ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#ifndef MATERIALMIX_HPP
#define MATERIALMIX_HPP

#include "Array.hpp"
#include "SimulationItem.hpp"
class MediumState;
class PhotonPacket;
class Random;
class StokesVector;
class WavelengthGrid;

////////////////////////////////////////////////////////////////////

/** MaterialMix is the abstract base class for all classes representing the concrete material
    properties of a specific transfer medium. The MaterialMix class hierarchy allows fundamentally
    different material types (e.g. dust, electrons, and hydrogen-dominated gas) to be implemented
    as part of a single framework.

    Instances of MaterialMix subclasses are immutable after setup has been completed, so the same
    instance can be reused in multiple contexts.

    <b>Material properties</b>

    The medium state maintained by a simulation for each cell and medium component includes a
    pointer to a MaterialMix instance defining the properties of the material, and a number density
    value defining the amount of material present in the cell per unit of volume. The kind of
    physical entity being counted by the number density and the conversion from number density to
    mass density depend on the type of material, as indicated in the table below.

    Material type | Entity counted | Mass conversion
    --------------|----------------|-----------------------------
    Dust          | hydrogen atom  | dust mass per hydrogen atom
    Electrons     | electron       | electron mass
    Gas           | hydrogen atom  | gas mass per hydrogen atom

    The following table lists some relevant physical quantities including cell properties that may
    be traced by a simulation, material properties defined by material mixes, and properties that
    can be derived from these.

    <TABLE>
    <TR><TD><B>Symbol</B></TD>  <TD><B>Units</B></TD>  <TD><B>Description</B></TD></TR>
    <TR><TD>\f$\Delta s\f$</TD>  <TD>\f$m\f$</TD>  <TD>Distance along a path</TD></TR>
    <TR><TD>\f$V\f$</TD>  <TD>\f$\text{m}^3\f$</TD>  <TD>Volume</TD></TR>
    <TR><TD>\f$v\f$</TD>  <TD>\f$\text{m}\,\text{s}^{-1}\f$</TD>  <TD>Bulk velocity</TD></TR>
    <TR><TD>\f$\bf{B}\f$</TD>  <TD>\f$\text{T}\f$</TD>  <TD>Magnetic field vector</TD></TR>
    <TR><TD>\f$T\f$</TD>  <TD>\f$\text{K}\f$</TD>  <TD>Temperature</TD></TR>
    <TR><TD>\f$n\f$</TD>  <TD>\f$\#\,\text{m}^{-3}\f$</TD>  <TD>Number density (of entities)</TD></TR>
    <TR><TD>\f$\mu\f$</TD>  <TD>\f$\text{kg}\,\#^{-1}\f$</TD>  <TD>Mass per entity</TD></TR>
    <TR><TD>\f$\varsigma\f$</TD>  <TD>\f$\text{m}^2\,\#^{-1}\f$</TD>  <TD>Cross section per entity</TD></TR>
    <TR><TD>\f$\mathcal{N}=n\Delta s\f$</TD> <TD>\f$\#\,\text{m}^{-2}\f$</TD>  <TD>Number column density</TD></TR>
    <TR><TD>\f$N=nV\f$</TD>  <TD>\f$\#\f$</TD>  <TD>Number (of entities)</TD></TR>
    <TR><TD>\f$\rho=n\mu\f$</TD>  <TD>\f$\text{kg}\,\text{m}^{-3}\f$</TD>  <TD>Mass density</TD></TR>
    <TR><TD>\f$\Sigma=n\mu\Delta s\f$</TD> <TD>\f$\text{kg}\,\text{m}^{-2}\f$</TD>  <TD>Mass column density</TD></TR>
    <TR><TD>\f$M=n\mu V\f$</TD>  <TD>\f$\text{kg}\f$</TD>  <TD>Mass</TD></TR>
    <TR><TD>\f$\kappa=\varsigma/\mu\f$</TD>  <TD>\f$\text{m}^2\,\text{kg}^{-1}\f$</TD>  <TD>Mass coefficient</TD></TR>
    <TR><TD>\f$k=n\varsigma\f$</TD>  <TD>\f$\text{m}^{-1}\f$</TD>  <TD>Opacity</TD></TR>
    <TR><TD>\f$\tau=n\varsigma\Delta s\f$</TD>  <TD>\f$1\f$</TD>  <TD>Optical depth</TD></TR>
    </TABLE>

    <b>Public interface</b>

    All MaterialMix subclasses, regardless of material type, must implement the public interface
    offered by this base class. This interface includes the capabilities required for tracing
    photon packets through a material of this type, in other words, for processing absorption and
    scattering.

    When the implementation of a particular feature specific to a subset of material mixes requires
    external access to information offered by those material mixes, the corresponding set of public
    functions is bundled in a separate abstract interface that can be implemented in the
    appropriate subclasses. For example, the set of material properties needed to calculate
    secondary emission spectra differs between fundamental material types and is thus offered by a
    specific public interface for each material type (e.g. thermal emission from dust grains).

    <b>Capabilities functions</b>

    The MaterialMix class hierarchy offers the materialType() function to obtain the overall
    material category (dust, gas, or electrons). In addition, it offers a number of Boolean
    functions that indicate whether a certain physical process is supported.

    This approach allows fine-grained run-time discovery of capabilities. The functions can be
    used, for example, during setup to ensure that the configuration is valid (e.g., all material
    mixes have the same level of support for polarization, all material mixes support stochastic
    heating when enabled in the configuration), to disable optimizations as needed (e.g., when
    calculating optical depth for dichroic materials), and to enable probing of the appropriate
    information (e.g., grain size distributions only for dust mixes offering that information).

    <b>Medium state setup functions</b>

    The MaterialMix class hierarchy offers a number of functions that advertise the required medium
    state variables and assist with initializing their values during setup. For example, the
    stateVariableInfo() function returns a list of medium state variable descriptors specifying the
    common and specific state variables used by the material mix. This allows the medium system to
    allocate storage for the appropriate set of state variables.

    The common state variables are initialized by the medium system without further help from the
    material mixes. Initialization of the specific state variables proceeds as follows. If the
    material mix is configured as part of a geometric medium component, the total density for the
    component in each spatial cell is determined from the configured geometry and normalization and
    it is passed to the material mix via the initializeGeometricState() function. If the material
    mix is configured as part of an imported medium component, extra data fields are imported from
    the snapshot based on the information returned by the parameterInfo() function and passed to
    the material mix via the initializeImportedState() function. In each case, the initialize()
    function is responsible for initializing all specific state variables.

    <b>Low-level material properties functions</b>

    The MaterialMix class hierarchy offers functions for retrieving some basic material properties
    as a function of wavelength, including the absorption cross section, the scattering cross
    section, and the scattering asymmetry parameter. These functions return \em default property
    values, assuming fixed, predefined values for any quantities other than wavelength (e.g., a
    default temperature, no polarization, no kinematics).

    The indicativeTemperature(Jv) function similarly returns an indicative temperature, depending
    on the material type. For dust mixes it returns the averaged equilibrium temperature of the
    grain population given the specified radiation field and assuming local thermal equilibrium
    conditions. Other materials may return a temperature determined based on the radiation field, a
    default value, or zero if none of the above apply.

    In principle, the values returned by these low-level functions may be used only during setup
    and for probing. However, some portions of the photon life cycle code might be optimized to use
    these functions directly in cases where the optical properties are known to depend solely on
    the photon packet’s wavelength.

    <b>High-level functions for photon life cycle</b>

    Most importantly, the MaterialMix class hierarchy offers a set of functions that help implement
    the photon life cycle on a high, generic level. These functions  receive at least two
    arguments: an object representing the medium state for a spatial cell and for a medium
    component configured with the receiving material mix, and an incoming photon packet. Extra
    arguments may override information that is also available as part of the state or photon
    packet, or they may simply provide additional information.

    For example, the opacityAbs() and opacitySca() functions return the absorption and scattering
    opacity \f$k=n\varsigma\f$. They are given a wavelength that overrides the photon packet
    wavelength. Providing a photon packet is in fact optional so that these functions can be used
    in situations where there is no photon packet involved, such as when calculating the luminosity
    absorbed by the dust in a cell.

    The propagate() function adjusts the photon packet for any effects caused by propagation over a
    given distance through the cell. This may include, for example, changes to the polarization
    state caused by dichroism. The function also returns the total (possibly dichroic) optical
    depth for the photon packet intensity over the given distance.

    The performScattering() function handles a complete random-walk scattering interaction with the
    medium component of the receiving material mix, including the effects of bulk velocity,
    polarization, and so forth. The peelOffScattering() function similarly calculates the
    contribution to a scattering peel-off event for this material, given the instrument reference
    frame and the relative weight of this medium component. */
class MaterialMix : public SimulationItem
{
    ITEM_ABSTRACT(MaterialMix, SimulationItem, "a material mix")
    ITEM_END()

    //============= Construction - Setup - Destruction =============

protected:
    /** This function caches the simulation's random generator for use by subclasses. */
    void setupSelfBefore() override;

    //======== Material type =======

public:
    /** This enumeration lists the fundamental material types supported by the MaterialMix class
        hierarchy. */
    enum class MaterialType { Dust, Electrons, Gas };

    /** This function returns the fundamental material type represented by this material mix. See
        the documentation of the MaterialMix class for more information. */
    virtual MaterialType materialType() const = 0;

    /** This convenience function returns true if the fundamental material type represented by this
        material mix is Dust, and false otherwise. */
    bool isDust() const { return materialType() == MaterialType::Dust; }

    /** This convenience function returns true if the fundamental material type represented by this
        material mix is Electrons, and false otherwise. */
    bool isElectrons() const { return materialType() == MaterialType::Electrons; }

    /** This convenience function returns true if the fundamental material type represented by this
        material mix is Gas, and false otherwise. */
    bool isGas() const { return materialType() == MaterialType::Gas; }

    //======== Capabilities =======

public:
    /** This enumeration lists the possible scattering modes offered by the public material mix
        interface. */
    enum class ScatteringMode {
        HenyeyGreenstein,
        MaterialPhaseFunction,
        SphericalPolarization,
        SpheroidalPolarization,
        Lya,
        LyaPolarization
    };

    /** This function returns the scattering mode supported by this material mix. In the current
        implementation, this can be one of the following modes:

        - HenyeyGreenstein: the value returned by the asymmpar() function serves as the assymmetry
        parameter \f$g\f$ for the Henyey-Greenstein phase function. For a value of \f$g=0\f$,
        isotropic scattering is implemented directly (rather than subsituting zero into the
        Henyey-Greenstein phase function).

        - MaterialPhaseFunction: this material type implements a custom phase function that depends
        only on the cosine of the scattering angle, for unpolarized radiation. Specifically, the
        phaseFunctionValueForCosine() and generateCosineFromPhaseFunction() functions are used to
        obtain the value of the phase function and to sample a scattering angle from it.

        - SphericalPolarization: this material type supports polarization through scattering by
        spherical particles. In this mode, the phase function depends on the polarization state of
        the incoming radiation, and the polarization state of the outgoing radiation must be
        updated appropriately. The phaseFunctionValue() and generateAnglesFromPhaseFunction()
        functions are used to obtain the value of the phase function and to sample a scattering
        angle from it, and the applyMueller() function is used to updated the polarization state.

        - SpheroidalPolarization: this material type supports polarization through scattering,
        absorption and emission by nonspherical, spheroidal particles. Currently, only \em emission
        is implemented and all other areas of the code treat spheroidal particles as if they were
        spherical.

        - Lya: this material type requires and offers treatment of Lyman-alpha line scattering,
        without support for polarization.

        - LyaPolarization: this material type requires and offers treatment of Lyman-alpha line
          scattering with support for polarization.

        The implementation of this function in this base class returns the HenyeyGreenstein
        scattering mode as a default value. Subclasses that support another scattering mode must
        override this function and return the appropriate value. */
    virtual ScatteringMode scatteringMode() const;

    /** This function returns true if this material mix supports polarization during scattering
        events, and false otherwise. The default implementation in this base class returns false.
        */
    virtual bool hasPolarizedScattering() const;

    /** This function returns true if the absorption of radiation for this material mix is dichroic
        (i.e. the absorption cross section depends on the polarization state of incoming photon and
        the polarization state is adjusted during absorption), and false otherwise. If
        hasPolarizedAbsorption() returns true, hasPolarizedScattering() must return true as well.
        The default implementation in this base class returns false. */
    virtual bool hasPolarizedAbsorption() const;

    /** This function returns true if the secondary emission for this material mix is or may be
        polarized and anisotropic, and false otherwise. If hasPolarizedEmission() returns true,
        hasPolarizedScattering() must return true as well. The default implementation in this base
        class returns false. */
    virtual bool hasPolarizedEmission() const;

    /** This function returns true if scattering for this material mix is resonant (such as for
        Lyman-alpha), and false otherwise. The default implementation in this base class returns
        false. */
    virtual bool hasResonantScattering() const;

    /** This function returns true if this material mix represents dust and supports stochastic
        heating of dust grains for the calculation of secondary emission, and false otherwise. The
        default implementation in this base class returns false. */
    virtual bool hasStochasticDustEmission() const;

    //======== Low-level material properties =======

public:
    /** This function returns the mass per entity \f$\mu\f$ for this material. The table below
        indicates the precise meaning of this number depending on the type of material being
        represented.

        Material type | Interpretation of mass() return value
        --------------|---------------------------------------
        Dust          | dust mass per hydrogen atom
        Electrons     | electron mass
        Gas           | gas mass per hydrogen atom
        */
    virtual double mass() const = 0;

    /** This function returns the default absorption cross section per entity
        \f$\varsigma^{\text{abs}}_{\lambda}\f$ at wavelength \f$\lambda\f$. */
    virtual double sectionAbs(double lambda) const = 0;

    /** This function returns the default scattering cross section per entity
        \f$\varsigma^{\text{sca}}_{\lambda}\f$ at wavelength \f$\lambda\f$. */
    virtual double sectionSca(double lambda) const = 0;

    /** This function returns the default extinction cross section per entity
        \f$\varsigma^{\text{ext}}_{\lambda} = \varsigma^{\text{abs}}_{\lambda} +
        \varsigma^{\text{sca}}_{\lambda}\f$ at wavelength \f$\lambda\f$. */
    virtual double sectionExt(double lambda) const = 0;

    /** This function returns the default scattering asymmetry parameter \f$g_\lambda =
        \left<\cos\theta\right>\f$ at wavelength \f$\lambda\f$. This value serves as a parameter
        for the Henyey-Greenstein phase function. The default implementation in this base class
        returns zero, indicating isotropic scattering. */
    virtual double asymmpar(double lambda) const;

    //======== High-level photon life cycle =======

    /** This function returns the absorption opacity \f$k^\text{abs}=n\varsigma^\text{abs}\f$ for
        the given wavelength, medium state, and photon properties (optional; may be nullptr). */
    virtual double opacityAbs(double lambda, const MediumState* state, const PhotonPacket* pp) const = 0;

    /** This function returns the scattering opacity \f$k^\text{sca}=n\varsigma^\text{sca}\f$ for
        the given wavelength, medium state, and photon properties (optional; may be nullptr). */
    virtual double opacitySca(double lambda, const MediumState* state, const PhotonPacket* pp) const = 0;

    /** This function returns the extinction opacity \f$k^\text{ext}=k^\text{abs}+k^\text{sca}\f$
        for the given wavelength, medium state, and photon properties (optional; may be nullptr). */
    virtual double opacityExt(double lambda, const MediumState* state, const PhotonPacket* pp) const = 0;

    //======== Scattering with material phase function =======

public:
    /** This function is used with the MaterialPhaseFunction scattering mode, which assumes that
        the scattering phase function depends only on the cosine of the scattering angle. The
        function returns the value of the scattering phase function \f$\Phi_\lambda(\cos\theta)\f$
        at wavelength \f$\lambda\f$ for the specified scattering angle cosine \f$\cos\theta\f$,
        where the phase function is normalized as \f[\int_{-1}^1 \Phi_\lambda(\cos\theta)
        \,\mathrm{d}\cos\theta =2.\f] The default implementation in this base class returns one,
        corresponding to isotropic scattering. */
    virtual double phaseFunctionValueForCosine(double lambda, double costheta) const;

    /** This function is used with the MaterialPhaseFunction scattering mode, which assumes that
        the scattering phase function depends only on the cosine of the scattering angle. The
        function generates a random scattering angle cosine sampled from the phase function
        \f$\Phi_\lambda(\cos\theta)\f$ at wavelength \f$\lambda\f$. The default implementation in
        this base class returns a value sampled uniformly over the interval [-1,1], corresponding
        to isotropic scattering. */
    virtual double generateCosineFromPhaseFunction(double lambda) const;

    //======== Polarization through scattering by spherical particles =======

public:
    /** This function is used with the SphericalPolarization scattering mode. It returns the value
        of the scattering phase function \f$\Phi_\lambda(\theta,\phi)\f$ at wavelength
        \f$\lambda\f$ for the specified scattering angles \f$\theta\f$ and \f$\phi\f$, and for the
        specified incoming polarization state. The phase function is normalized as
        \f[\int\Phi_\lambda(\theta,\phi) \,\mathrm{d}\Omega =4\pi.\f] The default implementation in
        this base class throws a fatal error. */
    virtual double phaseFunctionValue(double lambda, double theta, double phi, const StokesVector* sv) const;

    /** This function is used with the SphericalPolarization scattering mode. It generates random
        scattering angles \f$\theta\f$ and \f$\phi\f$ sampled from the phase function
        \f$\Phi_\lambda(\theta,\phi)\f$ at wavelength \f$\lambda\f$, and for the specified incoming
        polarization state. The results are returned as a pair of numbers in the order \f$\theta\f$
        and \f$\phi\f$. The default implementation in this base class throws a fatal error. */
    virtual std::pair<double, double> generateAnglesFromPhaseFunction(double lambda, const StokesVector* sv) const;

    /** This function is used with the SphericalPolarization scattering mode. It applies the
        Mueller matrix transformation for the specified wavelength \f$\lambda\f$ and scattering
        angle \f$\theta\f$ to the given polarization state (which serves as both input and output
        for the function). The default implementation in this base class throws a fatal error. */
    virtual void applyMueller(double lambda, double theta, StokesVector* sv) const;

    //======== Polarization through scattering, absorption and emission by spheroidal particles =======

public:
    /** This function is intended for use with the SpheroidalPolarization mode. It returns the grid
        used for discretizing quantities that are a function of the scattering/emission angle
        \f$\theta\f$. The same grid is returned by all material mixes that have
        SpheroidalPolarization mode. The default implementation in this base class throws a fatal
        error. */
    virtual const Array& thetaGrid() const;

    /** This function is intended for use with the SpheroidalPolarization mode. It returns the
        absorption cross sections per entity \f$\varsigma ^{\text{abs}} _{\lambda} (\theta)\f$ at
        wavelength \f$\lambda\f$ as a function of the emission angle \f$\theta\f$, discretized on
        the grid returned by the thetaGrid() function. The default implementation in this base
        class throws a fatal error. */
    virtual const Array& sectionsAbs(double lambda) const;

    /** This function is intended for use with the SpheroidalPolarization mode. It returns the
        linear polarization absorption cross sections per entity \f$\varsigma ^{\text{abspol}}
        _{\lambda} (\theta)\f$ at wavelength \f$\lambda\f$ as a function of the emission angle
        \f$\theta\f$, discretized on the grid returned by the thetaGrid() function. The default
        implementation in this base class throws a fatal error. */
    virtual const Array& sectionsAbspol(double lambda) const;

    //======== Temperature and emission =======

    /** This function returns the equilibrium temperature \f$T_{\text{eq}}\f$ (assuming LTE
        conditions) of the material mix when it would be embedded in the radiation field specified
        by the mean intensities \f$(J_\lambda)_\ell\f$, which must be discretized on the
        simulation's radiation field wavelength grid as returned by the
        Configuration::radiationFieldWLG() function. */
    virtual double equilibriumTemperature(const Array& Jv) const = 0;

    /** This function returns the emissivity spectrum \f$\varepsilon_{\ell'}\f$ of the material mix
        when it would be embedded in the radiation field specified by the mean intensities
        \f$(J_\lambda)_\ell\f$. The input radiation field must be discretized on the simulation's
        radiation field wavelength grid as returned by the Configuration::radiationFieldWLG()
        function. The output emissivity spectrum is discretized on a wavelength grid that depends
        on the material type. For more information, refer to the documentation of this function for
        each material type. */
    virtual Array emissivity(const Array& Jv) const = 0;

    //======================== Other Functions =======================

protected:
    /** This function returns the simulation's random generator as a service to subclasses. */
    Random* random() const { return _random; }

    //======================== Data Members ========================

private:
    // data member initialized in setupSelfBefore
    Random* _random{nullptr};
};

////////////////////////////////////////////////////////////////////

#endif
