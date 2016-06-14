#include "gx2.h"
#include "gx2_sampler.h"
#include "gpu/pm4_writer.h"

namespace gx2
{

inline uint32_t
floatToFixedPoint(float value, uint32_t bits, float min, float max)
{
   return static_cast<uint32_t>((value - min) * (static_cast<float>(1 << bits) / (max - min)));
}

void
GX2InitSampler(GX2Sampler *sampler,
               GX2TexClampMode clampMode,
               GX2TexXYFilterMode minMagFilterMode)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .CLAMP_X().set(static_cast<latte::SQ_TEX_CLAMP>(clampMode))
      .CLAMP_Y().set(static_cast<latte::SQ_TEX_CLAMP>(clampMode))
      .CLAMP_Z().set(static_cast<latte::SQ_TEX_CLAMP>(clampMode))
      .XY_MAG_FILTER().set(static_cast<latte::SQ_TEX_XY_FILTER>(minMagFilterMode))
      .XY_MIN_FILTER().set(static_cast<latte::SQ_TEX_XY_FILTER>(minMagFilterMode));

   sampler->regs.word0 = word0;
}

void
GX2InitSamplerBorderType(GX2Sampler *sampler,
                         GX2TexBorderType borderType)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .BORDER_COLOR_TYPE().set(static_cast<latte::SQ_TEX_BORDER_COLOR>(borderType));

   sampler->regs.word0 = word0;
}

void
GX2InitSamplerClamping(GX2Sampler *sampler,
                       GX2TexClampMode clampX,
                       GX2TexClampMode clampY,
                       GX2TexClampMode clampZ)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .CLAMP_X().set(static_cast<latte::SQ_TEX_CLAMP>(clampX))
      .CLAMP_Y().set(static_cast<latte::SQ_TEX_CLAMP>(clampY))
      .CLAMP_Z().set(static_cast<latte::SQ_TEX_CLAMP>(clampZ));

   sampler->regs.word0 = word0;
}

void
GX2InitSamplerDepthCompare(GX2Sampler *sampler,
                           GX2CompareFunction depthCompare)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .DEPTH_COMPARE_FUNCTION().set(static_cast<latte::SQ_TEX_DEPTH_COMPARE>(depthCompare));

   sampler->regs.word0 = word0;
}

void
GX2InitSamplerFilterAdjust(GX2Sampler *sampler,
                           BOOL highPrecision,
                           GX2TexMipPerfMode perfMip,
                           GX2TexZPerfMode perfZ)
{
   auto word2 = sampler->regs.word2.value();

   word2 = word2
      .HIGH_PRECISION_FILTER().set(!!highPrecision)
      .PERF_MIP().set(perfMip)
      .PERF_Z().set(perfZ);

   sampler->regs.word2 = word2;
}

void
GX2InitSamplerLOD(GX2Sampler *sampler,
                  float lodMin,
                  float lodMax,
                  float lodBias)
{
   auto word1 = sampler->regs.word1.value();

   word1 = word1
      .MIN_LOD().set(floatToFixedPoint(lodMin, 10, 0.0f, 16.0f))
      .MAX_LOD().set(floatToFixedPoint(lodMax, 10, 0.0f, 16.0f))
      .LOD_BIAS().set(floatToFixedPoint(lodBias, 12, -32.0f, 32.0f));

   sampler->regs.word1 = word1;
}

void
GX2InitSamplerLODAdjust(GX2Sampler *sampler,
                        float anisoBias,
                        BOOL lodUsesMinorAxis)
{
   auto word0 = sampler->regs.word0.value();
   auto word2 = sampler->regs.word2.value();

   word2 = word2
      .ANISO_BIAS().set(floatToFixedPoint(anisoBias, 6, 0.0f, 2.0f));

   word0 = word0
      .LOD_USES_MINOR_AXIS().set(!!lodUsesMinorAxis);

   sampler->regs.word0 = word0;
   sampler->regs.word2 = word2;
}

void
GX2InitSamplerRoundingMode(GX2Sampler *sampler,
                           GX2RoundingMode roundingMode)
{
   auto word2 = sampler->regs.word2.value();

   word2 = word2
      .TRUNCATE_COORD().set(static_cast<latte::SQ_TEX_ROUNDING_MODE>(roundingMode));

   sampler->regs.word2 = word2;
}

void
GX2InitSamplerXYFilter(GX2Sampler *sampler,
                       GX2TexXYFilterMode filterMag,
                       GX2TexXYFilterMode filterMin,
                       GX2TexAnisoRatio maxAniso)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .XY_MAG_FILTER().set(static_cast<latte::SQ_TEX_XY_FILTER>(filterMag))
      .XY_MIN_FILTER().set(static_cast<latte::SQ_TEX_XY_FILTER>(filterMin))
      .MAX_ANISO_RATIO().set(static_cast<latte::SQ_TEX_ANISO>(maxAniso));

   sampler->regs.word0 = word0;
}

void
GX2InitSamplerZMFilter(GX2Sampler *sampler,
                       GX2TexZFilterMode filterZ,
                       GX2TexMipFilterMode filterMip)
{
   auto word0 = sampler->regs.word0.value();

   word0 = word0
      .Z_FILTER().set(static_cast<latte::SQ_TEX_Z_FILTER>(filterZ))
      .MIP_FILTER().set(static_cast<latte::SQ_TEX_Z_FILTER>(filterMip));

   sampler->regs.word0 = word0;
}

void
GX2SetPixelSamplerBorderColor(uint32_t unit,
                              float red,
                              float green,
                              float blue,
                              float alpha)
{
   uint32_t values[] = {
      bit_cast<uint32_t>(red),
      bit_cast<uint32_t>(green),
      bit_cast<uint32_t>(blue),
      bit_cast<uint32_t>(alpha),
   };

   auto id = latte::Register::TD_PS_SAMPLER_BORDER0_RED + 4 * (unit * 4);
   pm4::write(pm4::SetConfigRegs { static_cast<latte::Register>(id), gsl::as_span(values) });
}


void
GX2SetVertexSamplerBorderColor(uint32_t unit,
                                 float red,
                                 float green,
                                 float blue,
                                 float alpha)
{
   uint32_t values[] = {
      bit_cast<uint32_t>(red),
      bit_cast<uint32_t>(green),
      bit_cast<uint32_t>(blue),
      bit_cast<uint32_t>(alpha),
   };

   auto id = latte::Register::TD_VS_SAMPLER_BORDER0_RED + 4 * (unit * 4);
   pm4::write(pm4::SetConfigRegs { static_cast<latte::Register>(id), gsl::as_span(values) });
}

void
GX2SetGeometrySamplerBorderColor(uint32_t unit,
                                 float red,
                                 float green,
                                 float blue,
                                 float alpha)
{
   uint32_t values[] = {
      bit_cast<uint32_t>(red),
      bit_cast<uint32_t>(green),
      bit_cast<uint32_t>(blue),
      bit_cast<uint32_t>(alpha),
   };

   auto id = latte::Register::TD_GS_SAMPLER_BORDER0_RED + 4 * (unit * 4);
   pm4::write(pm4::SetConfigRegs { static_cast<latte::Register>(id), gsl::as_span(values) });
}

} // namespace gx2