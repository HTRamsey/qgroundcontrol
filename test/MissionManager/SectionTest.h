#pragma once

#include "VisualMissionItemTest.h"

class Section;
class SimpleMissionItem;

/// Unit test for Sections
class SectionTest : public VisualMissionItemTest
{
    Q_OBJECT

public:
    SectionTest(void);

    void init(void) override;
    void cleanup(void) override;

protected:
    void _createSpy(Section* section, MultiSignalSpy** sectionSpy);
    void _commonScanTest(Section* section);

    SimpleMissionItem*  _simpleItem;
};
