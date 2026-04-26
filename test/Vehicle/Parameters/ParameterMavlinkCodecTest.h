#pragma once

#include "UnitTest.h"

class ParameterMavlinkCodecTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _factTypeRoundTrip_data();
    void _factTypeRoundTrip();
    void _factTypeUnsupportedFallsBackToInt32();
    void _mavTypeUnsupportedFallsBackToInt32();
    void _fillUnion_data();
    void _fillUnion();
    void _fillUnionRejectsNonNumeric();
    void _unionToVariant_data();
    void _unionToVariant();
    void _unionToVariantRejectsUnsupportedType();
    void _variantUnionRoundTrip_data();
    void _variantUnionRoundTrip();
};
