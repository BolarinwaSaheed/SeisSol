#ifndef SEISSOL_THERMALPRESSURIZATION_H
#define SEISSOL_THERMALPRESSURIZATION_H

#include <array>

#include "DynamicRupture/Misc.h"
#include "DynamicRupture/Parameters.h"
#include "Initializer/DynamicRupture.h"
#include "Kernels/precision.hpp"

namespace seissol::dr::friction_law {

template <size_t N>
class GridPoints {
  public:
  GridPoints() {
    for (size_t i = 0; i < N; ++i) {
      values[i] =
          misc::tpMaxWavenumber * std::exp(-misc::tpLogDz * (misc::numberOfTPGridPoints - i - 1));
    }
  }
  real const& operator[](size_t i) const { return values[i]; };

  private:
  real values[N]{0};
};

template <size_t N>
class InverseFourierCoefficients {
  public:
  constexpr InverseFourierCoefficients() {
    GridPoints<N> localGridPoints;

    for (size_t i = 1; i < N - 1; ++i) {
      values[i] = std::sqrt(2 / M_PI) * localGridPoints[i] * misc::tpLogDz;
    }
    values[0] = std::sqrt(2 / M_PI) * localGridPoints[0] * (1 + misc::tpLogDz);
    values[N - 1] = std::sqrt(2 / M_PI) * localGridPoints[N - 1] * 0.5 * misc::tpLogDz;
  }
  real const& operator[](size_t i) const { return values[i]; };

  private:
  real values[N]{0};
};

class ThermalPressurization {
  public:
  ThermalPressurization(DRParameters& drParameters) : drParameters(drParameters){};

  private:
  DRParameters& drParameters;

  protected:
  real (*temperature)[misc::numPaddedPoints];
  real (*pressure)[misc::numPaddedPoints];
  real (*theta)[misc::numPaddedPoints][misc::numberOfTPGridPoints];
  real (*sigma)[misc::numPaddedPoints][misc::numberOfTPGridPoints];
  real (*thetaTmpBuffer)[misc::numPaddedPoints][misc::numberOfTPGridPoints];
  real (*sigmaTmpBuffer)[misc::numPaddedPoints][misc::numberOfTPGridPoints];
  real (*halfWidthShearZone)[misc::numPaddedPoints];
  real (*hydraulicDiffusivity)[misc::numPaddedPoints];
  real (*faultStrength)[misc::numPaddedPoints];

  public:
  /**
   * copies all parameters from the DynamicRupture LTS to the local attributes
   */
  void copyLtsTreeToLocal(seissol::initializers::Layer& layerData,
                          seissol::initializers::DynamicRupture* dynRup,
                          real fullUpdateTime);

  /**
   * set initial value of thermal pressure
   */
  void setInitialFluidPressure(unsigned int ltsFace);

  /**
   * compute thermal pressure according to Noda and Lapusta 2010
   * bool saveTmpInTP is used to save final thermal pressure values for theta and sigma
   */
  void calcFluidPressure(std::array<real, misc::numPaddedPoints> const& normalStress,
                         real (*mu)[misc::numPaddedPoints],
                         std::array<real, misc::numPaddedPoints>& slipRateMagnitude,
                         real deltaT,
                         bool saveTmpInTP,
                         unsigned int timeIndex,
                         unsigned int ltsFace);

  /**
   * compute thermal pressure according to Noda and Lapusta 2010
   */
  void updateTemperatureAndPressure(real slipRateMagnitude,
                                    real deltaT,
                                    unsigned int pointIndex,
                                    unsigned int timeIndex,
                                    unsigned int ltsFace);

  real heatSource(double tauV, unsigned int tpGridPointIndex);

  real fluidPressure(unsigned int ltsFace, unsigned int pointIndex) const {
    return pressure[ltsFace][pointIndex];
  }
};
} // namespace seissol::dr::friction_law

#endif // SEISSOL_THERMALPRESSURIZATION_H
